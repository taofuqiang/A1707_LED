/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_encrypt.h
*\Description   
*\Note          
*\Log           2017.06.16    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_ENCRYPT_H
#define _DRV_ENCRYPT_H
#include "common_lib.h"
#include "stm32f10x.h"

#define ENCRYPT_STM32F1

//UUID 地址定义
#ifdef ENCRYPT_STM32F1  

    //倒数第四个扇区
    #define ENCRYPT_FLASH_ADDR    (0x08000000 + 1024 * (128 - 4))

    // chipID 地址 分成4份
    #define UUID_ADDR0 0X1F
    #define UUID_ADDR1 0XFF
    #define UUID_ADDR2 0XF7
    #define UUID_ADDR3 0XE8
#else 
    // chipID 地址 分成4份
    #define UUID_ADDR0 0X1F
    #define UUID_ADDR1 0XFF
    #define UUID_ADDR2 0X7A
    #define UUID_ADDR3 0X10
#endif


//UUID地址在 encrypt_offset_data 中的存储坐标
#define UUID_OFFSET0    0x23
#define UUID_OFFSET1    0x33
#define UUID_OFFSET2    0x43
#define UUID_OFFSET3_0  0x53
#define UUID_OFFSET3_1  0x63
#define UUID_OFFSET3_3  0x73

// 通用密码
#define GeneralID0  0xFF
#define GeneralID1  0xFF
#define GeneralID2  0xFF
#define GeneralID3  0xFF
#define GeneralID4  0xFF
#define GeneralID5  0xFF
#define GeneralID6  0xFF
#define GeneralID7  0xFF
#define GeneralID8  0xFF
#define GeneralID9  0xFF
#define GeneralID10 0xFF
#define GeneralID11 0xFF

//通用密码在 encrypt_offset_data 中的存储坐标
#define General_ID_OFFSET0  0x09
#define General_ID_OFFSET1  0x19
#define General_ID_OFFSET2  0x29
#define General_ID_OFFSET3  0x39
#define General_ID_OFFSET4  0x49
#define General_ID_OFFSET5  0x59
#define General_ID_OFFSET6  0x0A
#define General_ID_OFFSET7  0x1A
#define General_ID_OFFSET8  0x2A
#define General_ID_OFFSET9  0x3A
#define General_ID_OFFSET10 0x4A
#define General_ID_OFFSET11 0x5A


enum
{
    Encrypt_INIT,
    Encrypt_OK,
    Encrypt_Error,
};

int encrypt_state_check(void);
#endif /*_DRV_ENCRYPT_H*/

