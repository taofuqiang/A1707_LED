/***************************************************************************************************
*                    (c) Copyright 1992-2014 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          stm32_18b20.h
*\Description   
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_18B20_H
#define _DRV_18B20_H
#include "public_type.h"

enum
{
    Accuracy_Ds18B20_9Bit, //转换时间93.75ms
    Accuracy_Ds18B20_10Bit,//转换时间187.5ms
    Accuracy_Ds18B20_11Bit,//转换时间375ms
    Accuracy_Ds18B20_12Bit,//转换时间750ms
};

enum
{
    DS18B20_ROM_CMD_SKIP = 0xCC,
};

enum
{
    DS18B20_REG_CMD_CONV = 0x44,
    DS18B20_REG_CMD_READ_RAM = 0xBE,
    DS18B20_REG_CMD_WRITE_RAM = 0x4E,
    DS18B20_REG_CMD_COPY_RAM = 0x48,
    DS18B20_REG_CMD_RESET_RAM = 0xB8,
    DS18B20_REG_CMD_READ_VDD = 0xB4,
};

typedef struct TempDs18B20Type_st
{
    GPIO_TypeDef* Gpio;//对应的gpio port
    u16           Pin; //对应的gpio_pin
    TIM_TypeDef*  Tim; //精确延时
}TempDs18B20Type;

//温度格式
typedef struct Ds18B20TempType_st
{
    u8 TmpH;//温度整数部分
    u8 TmpL;//温度小数部分,支取小数后一位，因为18b20精度为0.5，多了也没有意义
    u8 Signal;//温度符号,0为正，1为负
}Ds18B20TempType;

extern Ds18B20TempType Env_TMP;
extern Ds18B20TempType DC_TMP;

void stm32_ds18b20_thread_entry(void* para);

#endif /*_STM32_18B20_H*/
