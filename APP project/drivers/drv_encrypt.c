/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_encrypt.c
*\Description   
*\Note          
*\Log           2017.06.16    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "drv_encrypt.h"
#include "stdlib.h"

// 密码存储区
static unsigned char encrypt_code[12] ={0}; 
// 通用密码和 计算数据
const unsigned char encrypt_offset_data[128]=
{
    //      0    1      2    3          4     5      6     7     8     9            A           B     C     D    E      F
    /*0*/ 0x45, 0x08, 0xcb, 0x5a,       0xdd, 0x21, 0xff, 0xee, 0x62, GeneralID0, GeneralID6, 0x14, 0x66, 0x77, 0x44, 0x55,
    /*1*/ 0x2b, 0x90, 0x21, 0x32,       0x42, 0x54, 0x67, 0x76, 0xea, GeneralID1, GeneralID7, 0x89, 0xfe, 0xef, 0x2c, 0xcd,
    /*2*/ 0x00, 0x21, 0x22, UUID_ADDR0, 0x44, 0x65, 0xc6, 0x87, 0x88, GeneralID2, GeneralID8, 0xbb, 0xcc, 0x3a, 0xce, 0x2c,
    /*3*/ 0x31, 0x10, 0x32, UUID_ADDR1, 0xe5, 0xb4, 0xf7, 0x86, 0x34, GeneralID3, GeneralID9, 0x01, 0xc3, 0x67, 0x54, 0x05,
    /*4*/ 0xab, 0xbb, 0x88, UUID_ADDR2, 0xf7, 0xef, 0xcc, 0x8f, 0x33, GeneralID4, GeneralID10,0x00, 0xd7, 0x66, 0x55, 0xb4,
    /*5*/ 0x5b, 0xba, 0x99, UUID_ADDR3, 0xdf, 0xf7, 0xdd, 0xfe, 0xef, GeneralID5, GeneralID11,0xcf, 0xbb, 0x3a, 0x99, 0x78,
    /*6*/ 0x8a, 0xa9, 0xca,UUID_ADDR3+4,0xdc, 0x2d, 0xfe, 0x6f, 0x2c, 0xa1,       0x30,       0x20, 0x50, 0x24, 0x76, 0x3a,
    /*7*/ 0x48, 0x45, 0x87,UUID_ADDR3+8,0x08, 0x62, 0x32, 0x23, 0x66, 0x2c,       0x4c,       0x45, 0x2c, 0x33, 0x00, 0x41,
};
// 本机 ID 存储变量
volatile union 
{
    uint32_t data32[3];
    uint8_t  data08[12];
}MCU_UUID;

// 本机密码存储变量
volatile uint8_t MyIDCODE[12];

/***************************************************************************************************
*\Function      encrypt_MyIDCODE_calc
*\Description   计算本机密码，对ID进行加密后的代码
*\Parameter     pUID
*\Parameter     peCODE
*\Return        void
*\Note          随便自己搞点运算
*\Log           2017.06.19    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
static void encrypt_MyIDCODE_calc(uint8_t *pUID, uint8_t *peCODE)
{
    peCODE[0] = ~pUID[0] >> 2 + 0x01;
    peCODE[1] = ~pUID[1] * 3 - 0x02;
    peCODE[2] = ~pUID[2] + 3;
    peCODE[3] = ~pUID[3] - 0x04;
    peCODE[4] = ~pUID[4] * 4;
    peCODE[5] = ~pUID[5] ^ peCODE[4];
    peCODE[6] = ~pUID[6] << 2;
    peCODE[7] = ~pUID[7] >> 2;
    peCODE[8] = ~pUID[8] + peCODE[7];
    peCODE[9] = ~pUID[9] << 5;
    peCODE[10]= ~pUID[10] >> 3;
    peCODE[11]= ~pUID[11];
}
/***************************************************************************************************
*\Function      encrypt_code_save
*\Description   保存密码
*\Parameter     
*\Return        void
*\Note          
*\Log           2017.06.19    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
static void encrypt_code_save(void)
{
    u32 addr = ENCRYPT_FLASH_ADDR;
    u32 i = 0;
    uint16_t *pBuffer = (uint16_t*)MyIDCODE;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_EOP); 
    /* 擦除这个扇区 */
    if (FLASH_COMPLETE == FLASH_ErasePage(addr))
    {
        //保存动作 
        for (i = 0; i < sizeof(MyIDCODE) / 2; i++)
        {
            FLASH_ProgramHalfWord(addr, pBuffer[i]);
            addr += 2;
        }
        //随便多写一些数据
        for (i = 0; i < 64; i++)
        {
            FLASH_ProgramHalfWord(addr, rand()%65535);
            addr += 2;
        }
    }

    FLASH_Lock();
}

/***************************************************************************************************
*\Function      encrypt_code_load
*\Description   下载保存的加密值
*\Parameter     
*\Return        void
*\Note          
*\Log           2017.06.19    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
static void encrypt_code_load(void)
{
    u32 addr = ENCRYPT_FLASH_ADDR;
    u32 i = 0;
    uint16_t *pBuffer = (uint16_t*)encrypt_code;

    //保存动作 多余的会随便写一些数据
    for (i = 0; i < sizeof(encrypt_code) / 2; i++)
    {
        pBuffer[i] = *(vu16*)(addr);
        addr += 2;
    }
}
/***************************************************************************************************
*\Function      encrypt_read_ID
*\Description   读到的 UID 存入 pUID_dat32 指向的存储空间
*\Parameter     pUID_dat32
*\Return        void
*\Note          
*\Log           2017.06.19    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
static void encrypt_read_ID(uint32_t *pUID_dat32)
{
    uint32_t mcuID_add;
    uint32_t tmp[4];
    // 31:0
    // 为了更安全,不使用用立即数访问 UID,
    // 所以将 UID 的地址分散在 crc_tmp_data 中,然后通过读取 crc_tmp_data 合成 UID 地址
    tmp[0] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET0];
    tmp[1] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET1];
    tmp[2] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET2];
    tmp[3] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET3_0];
    mcuID_add = (tmp[0]<<24) + (tmp[1]<<16) + (tmp[2]<<8) + tmp[3]; // 合成 UID 地址
    pUID_dat32[0] = *(__IO uint32_t*)(mcuID_add); // 读取 UID[31:0]
    // 63:32
    tmp[0] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET0];
    tmp[1] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET1];
    tmp[2] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET2];
    tmp[3] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET3_1];
    mcuID_add = (tmp[0]<<24) + (tmp[1]<<16) + (tmp[2]<<8) + tmp[3];
    pUID_dat32[1] = *(__IO uint32_t*)(mcuID_add);
    // 95:64
    tmp[0] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET0];
    tmp[1] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET1];
    tmp[2] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET2];
    tmp[3] = *(__IO uint8_t*)&encrypt_offset_data[UUID_OFFSET3_3];
    mcuID_add = (tmp[0]<<24) + (tmp[1]<<16) + (tmp[2]<<8) + tmp[3];
    pUID_dat32[2] = *(__IO uint32_t*)(mcuID_add);
}
/***************************************************************************************************
*\Function      encrypt_state_check
*\Description   
*\Parameter     
*\Return        int
*\Note          0,第一次，1,加密ok，2为盗版
*\Log           2017.06.19    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
int encrypt_state_check(void)
{
    //Step1.下载保存值
    encrypt_code_load();
    //Step2.读取本机 ID,存入 mcuID 联合体
    encrypt_read_ID((uint32_t*)MCU_UUID.data32);
    //Step3.使用本机 ID 计算本机密码,存入 eCODE 数组
    encrypt_MyIDCODE_calc((uint8_t*)MCU_UUID.data08, (uint8_t*)MyIDCODE);

    // Step4.对比 FLASH 中存储的密码是否等于通用密码
    if((encrypt_offset_data[General_ID_OFFSET0] == *(__IO uint8_t*)&encrypt_code[0])
        && (encrypt_offset_data[General_ID_OFFSET1] == *(__IO uint8_t*)&encrypt_code[1])
        && (encrypt_offset_data[General_ID_OFFSET2] == *(__IO uint8_t*)&encrypt_code[2])
        && (encrypt_offset_data[General_ID_OFFSET3] == *(__IO uint8_t*)&encrypt_code[3])
        && (encrypt_offset_data[General_ID_OFFSET4] == *(__IO uint8_t*)&encrypt_code[4])
        && (encrypt_offset_data[General_ID_OFFSET5] == *(__IO uint8_t*)&encrypt_code[5])
        && (encrypt_offset_data[General_ID_OFFSET6] == *(__IO uint8_t*)&encrypt_code[6])
        && (encrypt_offset_data[General_ID_OFFSET7] == *(__IO uint8_t*)&encrypt_code[7])
        && (encrypt_offset_data[General_ID_OFFSET8] == *(__IO uint8_t*)&encrypt_code[8])
        && (encrypt_offset_data[General_ID_OFFSET9] == *(__IO uint8_t*)&encrypt_code[9])
        && (encrypt_offset_data[General_ID_OFFSET10]== *(__IO uint8_t*)&encrypt_code[10])
        && (encrypt_offset_data[General_ID_OFFSET11]== *(__IO uint8_t*)&encrypt_code[11])
        ) 
    {       
        // 如果密码等于通用密码,则将计算的结果写入 FLASH
        encrypt_code_save();
        return Encrypt_INIT;
    }
    //Step5.对比计算的本机密码是否等于 FLASH 中存储的密码
    else if((MyIDCODE[0] == *(__IO uint8_t*)&encrypt_code[0])
        && (MyIDCODE[1] == *(__IO uint8_t*)&encrypt_code[1])
        && (MyIDCODE[2] == *(__IO uint8_t*)&encrypt_code[2])
        && (MyIDCODE[3] == *(__IO uint8_t*)&encrypt_code[3])
        && (MyIDCODE[4] == *(__IO uint8_t*)&encrypt_code[4])
        && (MyIDCODE[5] == *(__IO uint8_t*)&encrypt_code[5])
        && (MyIDCODE[6] == *(__IO uint8_t*)&encrypt_code[6])
        && (MyIDCODE[7] == *(__IO uint8_t*)&encrypt_code[7])
        && (MyIDCODE[8] == *(__IO uint8_t*)&encrypt_code[8])
        && (MyIDCODE[9] == *(__IO uint8_t*)&encrypt_code[9])
        && (MyIDCODE[10]== *(__IO uint8_t*)&encrypt_code[10])
        && (MyIDCODE[11]== *(__IO uint8_t*)&encrypt_code[11])
        ) 
    {
        // 如果密码校验正确,则不进行任何操作
        return Encrypt_OK;
    } 
    else 
    {
        // 如果密码校验错位,则关机或者销毁程序
        return Encrypt_Error;
    }
}
