/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          iap_if.h
*\Description   
*\Note          
*\Log           2016.06.28    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _IAP_IF_H
#define _IAP_IF_H
#include "stm32f1xx.h"
#include "public_type.h"
extern char* IAPoE_Name;//IAP固件名称
extern char* IAPoE_Ver;//IAP固件版本
extern char* IAPoE_Date;//IAP固件发布日期

extern uint64_t IAP_FLAG;//iap模式标志


#define FLASH_SIZE_ADDR     (0x1FFFF7E0)//flash容量存放地址
#define FLASH_START_ADDR    (0x08000000)//flash起始地址

//固件最大大小
#define OTA_BIN_MAX_SIZE     52* FLASH_PAGE_SIZE//(FLASH_SIZE - 64 * FLASH_PAGE_SIZE)
#define OTA_RUN_APP          ((uint64_t)0xFFFFFFFFFFFFFFFF)
#define OTA_Update_APP       ((uint64_t)0xFFFF5678FFFF5678)
#define OTA_Force_Update_APP ((uint64_t)0x1234567812345678)

//运行区地址
#define FLASH_APP_RUN_ADDR  (0x0800A000)

//IAP标志地址
#define FLASH_IAP_FLAG_ADDR  1//(FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE)
//IAP 设备参数存储地址
#define FLASH_IAP_PARAM_ADDR  2//(FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE*2)
//APP 设备参数存储地址
#define FLASH_APP_PARAM_ADDR  3//(FLASH_BASE + FLASH_SIZE - FLASH_PAGE_SIZE*3)
#define FLASH_APP_INFO_ADDR  4//(FLASH_BASE + FLASH_SIZE)
#define FLASH_APP_FACTORY_ADDR  5//(FLASH_BASE + FLASH_SIZE)
//app固件存储起始地址
#define FLASH_APP_COPY_ADDR  57//(FLASH_BASE + FLASH_SIZE + FLASH_PAGE_SIZE)未处理 升级的时候处理

uint16_t Get_FlashSize(void);
void RunAppMode(void);
void UpdateAppMode(void);
void ForceUpdateAppMode(void);
uint64_t ReadIAPMode(void);
void Get_ChipID(uint16_t* chipID);
	
#define DEV_FIRST_POWER_FLAG 0x1234
typedef struct iap_param_type_st
{
	uint16_t Power_First_Flag;//第一次上电标志
	uint16_t ChipUniqueID[6];
	uint16_t swversion;//软件版本信息
	uint16_t hwversion;//硬件版本信息
	uint16_t IAP_flag;//IAP标志 iap 1 app 2
	uint16_t remoteID;//外部通信设备ID 
	uint16_t ID;//
}iap_param_type;

typedef struct
{
	uint16_t CompressFlag;//压缩标志，0未压缩，1压缩
	uint32_t UpdatesTotalSize;//升级包总大小
	uint16_t UpdatesSubSize;//升级包分包大小
	uint32_t UpdatesCRC32;//升级包文件校验
	uint32_t ComPackOriginalSize;//压缩之前原始文件总大小
	uint16_t ComPackNo;//压缩包个数
	uint16_t PerComPackOriginalSize;//压缩之前每个压缩包原始数据大小
	uint32_t ComPackOriginalCRC32;//lora配置第四位
}__attribute__((packed)) iap_updata_param_type;
extern iap_param_type iap_param;
void iap_param_save(void);
void iap_param_load(void);
extern void iap_param_default(void);

#endif /*_IAP_IF_H*/

