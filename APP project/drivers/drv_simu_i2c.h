/***************************************************************************************************
*                    (c) Copyright 2006-2016  Syncretize technologies co.,ltd.
*										All Rights Reserved
*	
*\File 			stm32_simu_i2c.h
*\Description   
*\Note          
*\Log           2016.07.11    Ver 1.0   Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_SIMU_I2C_H
#define _DRV_SIMU_I2C_H
#include "common_lib.h"

/*i2c总线错误标志*/
typedef enum 
{
    SIMU_I2C_OK,
    SIMU_I2C_BUSY, //总线忙
    SIMU_I2C_NOACK,//无响应
}SimuI2CErrState;


/*模拟i2c使用到的结构*/
typedef struct stm32_simu_i2c_type
{
    GpioType* scl;/*i2c时钟输出脚*/
    GpioType* sda;/*i2c数据输出脚*/
    /*上面需初始化赋值*/
    rt_sem_t simu_i2c_sem;/*设备互斥锁*/
}stm32_simu_i2c_type;

void stm32_simu_i2c_init(stm32_simu_i2c_type* simu_i2c);
u32 stm32_simu_i2c_send(stm32_simu_i2c_type* simu_i2c, u16 slave_addr, u8* buf, u32 len);
u32 stm32_simu_i2c_recv(stm32_simu_i2c_type* simu_i2c, u16 slave_addr, u8 word_addr, u8* buf, u32 len);

u32 stm32_simu_i2c_send_ch45x(stm32_simu_i2c_type* simu_i2c, u8* buf, u32 len);
u32 stm32_simu_i2c_recv_ch45x(stm32_simu_i2c_type* simu_i2c, u8 cmd, u8* buf, u32 len);

#endif /*_DRV_SIMU_I2C_H*/
