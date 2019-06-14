/***************************************************************************************************
*                    (c) Copyright 2006-2016  Syncretize technologies co.,ltd.
*										All Rights Reserved
*	
*\File 			stm32_simu_i2c.c
*\Description   
*\Note          实现，待测试
*\Log           2016.07.11    Ver 1.0   Job
*               创建文件。
***************************************************************************************************/
#include "drv_simu_i2c.h"


/***************************************************************************************************
*\Function      SimuI2CDelay
*\Description   内部延时函数
*\Parameter     
*\Return        void
*\Note          f4上1约为1us
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static  void SimuI2CDelay(void)
{
    u32 i = 100;
    while(i--);
}

/***************************************************************************************************
*\Function      I2cBUSBusyRecovery
*\Description   i2c总线死锁恢复，连续发送9个脉冲后再发送停止位
*\Parameter     simu_i2c
*\Return        void
*\Note          
*\Log           2011.10.11    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static void I2cBUSBusyRecovery(stm32_simu_i2c_type* simu_i2c)
{
    u8 i = 0;

    /*SDA开漏输出高*/
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    /*连续输出9个时钟脉冲*/
    for (i = 0; i < 9; i++)
    {
        GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();
        GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();
    }
    /*产生一次停止位，即数据线在scl为高电平时由低变高*/
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    GPIO_ResetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
}
/***************************************************************************************************
*\Function      SimuI2CStart
*\Description   模拟i2c起始条件
*\Parameter     simu_i2c
*\Return        bool
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CStart(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    /*检测总线是否为忙，忙则退出*/
    if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
    {
        return SIMU_I2C_BUSY;
    }
    GPIO_ResetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CStop
*\Description   模拟i2c停止条件
*\Parameter     simu_i2c
*\Return        SimuI2CERR
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CStop(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_ResetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    return SIMU_I2C_OK; 
}
/***************************************************************************************************
*\Function      SimuI2CACK
*\Description   模拟i2c响应ack
*\Parameter     simu_i2c
*\Return        SimuI2CERR
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CACK(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_ResetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CNOACK
*\Description   模拟i2c无响应
*\Parameter     simu_i2c
*\Return        SimuI2CERR
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CNOACK(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CWaitACK
*\Description   模拟i2c等待接收ack
*\Parameter     simu_i2c
*\Return        
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CWaitACK(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);		
    SimuI2CDelay();
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    SimuI2CDelay();
    if(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin))
    {
        /*读ack为1，发生错误*/
        GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        return SIMU_I2C_NOACK;
    }
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CByteSend
*\Description   模拟i2c一个字节发送过程，高位在前，低位在后
*\Parameter     simu_i2c
*\Parameter     byte
*\Return        SimuI2CERR
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CByteSend(stm32_simu_i2c_type* simu_i2c, u8 byte)
{
    u8  i = 8;

    while (i--)
    {
        GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();
        if (byte & 0x80)
        {
            GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
        }
        else
        {
            GPIO_ResetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
        }
        byte <<= 1;
        SimuI2CDelay();
        GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();
    }
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CByteRecv
*\Description   模拟i2c一个字节接收过程，高位在前，低位在后
*\Parameter     simu_i2c
*\Parameter     byte
*\Return        SimuI2CERR
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CByteRecv(stm32_simu_i2c_type* simu_i2c, u8* byte)
{
    u8  i = 8;
    u8  tmp = 0;

    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);				
    while (i--)
    {
        tmp <<= 1;      
        GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();
        GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
        SimuI2CDelay();	
        if(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin))
        {
            tmp |= 0x01;
        }
    }
    GPIO_ResetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    *byte = tmp;
    return SIMU_I2C_OK;
}
/***************************************************************************************************
*\Function      SimuI2CSend
*\Description   模拟i2c发送函数接口
*\Parameter     simu_i2c
*\Parameter     slave_addr 从器件地址
*\Parameter     buf
*\Parameter     len
*\Return        bool
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CSend(stm32_simu_i2c_type* simu_i2c, u8 slave_addr, u8* buf, u32 len)
{
    SimuI2CErrState result;
    u32 i  = 0;
    /*起始条件*/
    result = SimuI2CStart(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*发送从器件地址 + 写*/
    result = SimuI2CByteSend(simu_i2c, slave_addr & 0xFE);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*等待从器件响应*/
    result = SimuI2CWaitACK(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        SimuI2CStop(simu_i2c);
        return result;
    }
    /*循环发送数据*/
    for (i = 0; i < len; i++)
    {
        SimuI2CByteSend(simu_i2c, buf[i]);
        result = SimuI2CWaitACK(simu_i2c);
        /*如果无响应则错误，发送停止位*/
        if (result != SIMU_I2C_OK)
        {
            SimuI2CStop(simu_i2c);
            return result;
        }
    }
    /*发送停止条件*/
    return SimuI2CStop(simu_i2c);
}
/***************************************************************************************************
*\Function      SimuI2CRecv
*\Description   模拟i2c接收接口函数
*\Parameter     simu_i2c
*\Parameter     slave_addr
*\Parameter     word_addr
*\Parameter     buf
*\Parameter     len
*\Return        SimuI2CErrState
*\Note          
*\Log           2011.10.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CRecv(stm32_simu_i2c_type* simu_i2c, u8 slave_addr, u8 word_addr, u8* buf, u32 len)
{
    SimuI2CErrState result;
    u32 i  = 0;
    /*起始条件*/
    result = SimuI2CStart(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*发送从器件地址 + 写*/
    result = SimuI2CByteSend(simu_i2c, slave_addr & 0xFE);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*等待从器件响应*/
    result = SimuI2CWaitACK(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        SimuI2CStop(simu_i2c);
        return result;
    }
    /*发送从器件内部寄存器地址*/
    result = SimuI2CByteSend(simu_i2c, word_addr);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*等待从器件响应*/
    result = SimuI2CWaitACK(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        SimuI2CStop(simu_i2c);
        return result;
    }
    /*重新发送起始条件*/
    result = SimuI2CStart(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*发送从器件地址 + 读*/
    result = SimuI2CByteSend(simu_i2c, slave_addr | 0x01);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*等待从器件响应*/
    result = SimuI2CWaitACK(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        SimuI2CStop(simu_i2c);
        return result;
    }
    /*循环接收len长数据*/
    for (i = 0; i < len; i++)
    {
        SimuI2CByteRecv(simu_i2c, buf++);
        if (i == len - 1)
        {
            result = SimuI2CNOACK(simu_i2c);
        }
        else
        {
            result = SimuI2CACK(simu_i2c);
        }
    }
    return SimuI2CStop(simu_i2c);
}
/***************************************************************************************************
*\Function      gpio_rcc_enable
*\Description   
*\Parameter     gpio
*\Return        void
*\Note          
*\Log           2016.07.11    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
static void gpio_rcc_enable(GPIO_TypeDef* gpio)
{
    //使能端口时钟
    switch((u32)gpio)
    {
    case GPIOA_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    case GPIOB_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        break;
    case GPIOC_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        break;
    case GPIOD_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        break;
    case GPIOE_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
        break;
    case GPIOF_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
        break;
    case GPIOG_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
        break;
    default:
        rt_kprintf("gpio clk error!\n");
        break;
    }
}
/***************************************************************************************************
*\Function      stm32_simu_i2c_init
*\Description   模拟i2c初始化
*\Parameter     scl，时钟脚
*\Parameter     sda，数据脚
*\Return        stm32_simu_i2c_type*
*\Note          
*\Log           2016.07.11    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
void stm32_simu_i2c_init(stm32_simu_i2c_type* simu_i2c)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    if (simu_i2c == RT_NULL || simu_i2c->scl == RT_NULL || simu_i2c->sda == RT_NULL)
    {
        rt_kprintf("simu i2c null!\n");
        return;
    }
    //gpio时钟使能
    gpio_rcc_enable(simu_i2c->scl->gpio);
    gpio_rcc_enable(simu_i2c->sda->gpio);

    /*根据i2c协议，空闲状态总线为高电平*/  
    /*时钟脚初始化*/
    /*初始化之前先向输出寄存器写入1，防止初始化完成后默认输出0，产生i2c起始时序*/
    GPIO_SetBits(simu_i2c->scl->gpio, simu_i2c->scl->gpio_pin);
    GPIO_InitStructure.GPIO_Pin   = simu_i2c->scl->gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(simu_i2c->scl->gpio, &GPIO_InitStructure);

    /*数据脚初始化*/
    /*初始化之前先向输出寄存器写入1，防止初始化完成后默认输出0，产生i2c起始时序*/
    GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);
    GPIO_InitStructure.GPIO_Pin       = simu_i2c->sda->gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(simu_i2c->sda->gpio, &GPIO_InitStructure);

    /*在初始化完成后如果检测到总线忙，则发送9个脉冲+stop，如果
    *总线不忙，则发送start+stop复位m41t83 i2c状态机，这种操作对其他i2c器件的影响待验证*/
    /*如果检测到数据线被拉低，则总线忙，执行总线解锁操作*/
    if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
    {
        I2cBUSBusyRecovery(simu_i2c);
    }
    else
    {   
        SimuI2CStart(simu_i2c);
        SimuI2CStop(simu_i2c);
    }   
    /*申请互斥信号量*/
    simu_i2c->simu_i2c_sem = rt_sem_create("simu_i2c", 1, RT_IPC_FLAG_FIFO);
    if (simu_i2c->simu_i2c_sem == RT_NULL)
    {
        rt_kprintf("simu_i2c sem create fail!\n");
        return;
    }
    return;
}
/***************************************************************************************************
*\Function      stm32_simu_i2c_send
*\Description   
*\Parameter     simu_i2c
*\Parameter     slave_addr
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note          
*\Log           2016.07.11    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 stm32_simu_i2c_send(stm32_simu_i2c_type* simu_i2c, u16 slave_addr, u8* buf, u32 len)
{
    SimuI2CErrState result;

    rt_sem_take(simu_i2c->simu_i2c_sem, RT_WAITING_FOREVER);

    result = SimuI2CSend(simu_i2c,(u8)slave_addr, buf, len);
    if (result != SIMU_I2C_OK)
    {
        /*发送失败进行复位处理*/
        GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);//读之前先输出1
        if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
        {
            I2cBUSBusyRecovery(simu_i2c);
        }
        else
        {   
            SimuI2CStart(simu_i2c);
            SimuI2CStop(simu_i2c);
        }  
        rt_sem_release(simu_i2c->simu_i2c_sem);
        return 0x00;
    }
    rt_sem_release(simu_i2c->simu_i2c_sem);
    return len;
}

/***************************************************************************************************
*\Function      stm32_simu_i2c_recv
*\Description   
*\Parameter     simu_i2c
*\Parameter     slave_addr
*\Parameter     word_addr
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note          
*\Log           2016.07.11    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 stm32_simu_i2c_recv(stm32_simu_i2c_type* simu_i2c, u16 slave_addr, u8 word_addr, u8* buf, u32 len)
{
    SimuI2CErrState result;

    rt_sem_take(simu_i2c->simu_i2c_sem, RT_WAITING_FOREVER);

    result = SimuI2CRecv(simu_i2c,(u8)slave_addr, word_addr, buf, len);
    if (result != SIMU_I2C_OK)
    {
        /*接收失败进行复位处理*/
        GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);//读之前先输出1
        if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
        {
            I2cBUSBusyRecovery(simu_i2c);
        }
        else
        {   
            SimuI2CStart(simu_i2c);
            SimuI2CStop(simu_i2c);
        } 
        rt_sem_release(simu_i2c->simu_i2c_sem);
        return 0x00;
    }

    rt_sem_release(simu_i2c->simu_i2c_sem);

    return len;
}
/**************************************************************************************************/
/*ch45x专用时序                                                                                   */
/**************************************************************************************************/
/***************************************************************************************************
*\Function      SimuI2CSendCh45x
*\Description   ch45x芯片专用时序
*\Parameter     simu_i2c
*\Parameter     buf
*\Parameter     len
*\Return        SimuI2CErrState
*\Note          
*\Log           2016.07.28    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CSendCh45x(stm32_simu_i2c_type* simu_i2c, u8* buf, u32 len)
{
    SimuI2CErrState result;
    u32 i  = 0;

    /*起始条件*/
    result = SimuI2CStart(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*循环发送数据*/
    for (i = 0; i < len; i++)
    {
        SimuI2CByteSend(simu_i2c, buf[i]);
        SimuI2CNOACK(simu_i2c);
        //由于ch45x无ack，所以不用管ack
    }
    /*发送停止条件*/
    return SimuI2CStop(simu_i2c);
}
/***************************************************************************************************
*\Function      stm32_simu_i2c_send_ch45x
*\Description   
*\Parameter     simu_i2c
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note          
*\Log           2016.07.28    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 stm32_simu_i2c_send_ch45x(stm32_simu_i2c_type* simu_i2c, u8* buf, u32 len)
{
    SimuI2CErrState result;

    rt_sem_take(simu_i2c->simu_i2c_sem, RT_WAITING_FOREVER);

    result = SimuI2CSendCh45x(simu_i2c, buf, len);
    if (result != SIMU_I2C_OK)
    {
        /*发送失败进行复位处理*/
        GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);//读之前先输出1
        if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
        {
            I2cBUSBusyRecovery(simu_i2c);
        }
        else
        {   
            SimuI2CStart(simu_i2c);
            SimuI2CStop(simu_i2c);
        }  
        rt_sem_release(simu_i2c->simu_i2c_sem);
        return 0x00;
    }
    rt_sem_release(simu_i2c->simu_i2c_sem);
    return len;
}
/***************************************************************************************************
*\Function      SimuI2CRecvCh45x
*\Description   ch45x芯片专用
*\Parameter     simu_i2c
*\Parameter     cmd
*\Parameter     buf
*\Parameter     len
*\Return        SimuI2CErrState
*\Note          
*\Log           2016.07.28    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
static SimuI2CErrState SimuI2CRecvCh45x(stm32_simu_i2c_type* simu_i2c, u8 cmd, u8* buf, u32 len)
{
    SimuI2CErrState result;
    u32 i  = 0;
    /*起始条件*/
    result = SimuI2CStart(simu_i2c);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*发送命令码*/
    result = SimuI2CByteSend(simu_i2c, cmd);
    if (result != SIMU_I2C_OK)
    {
        return result;
    }
    /*等待从器件响应 无用管ack结果*/
    SimuI2CNOACK(simu_i2c);

    /*循环接收len长数据*/
    for (i = 0; i < len; i++)
    {
        SimuI2CByteRecv(simu_i2c, buf++);
        SimuI2CNOACK(simu_i2c);
    }
    return SimuI2CStop(simu_i2c);
}
/***************************************************************************************************
*\Function      stm32_simu_i2c_recv
*\Description   
*\Parameter     simu_i2c
*\Parameter     slave_addr
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note          
*\Log           2016.07.28    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 stm32_simu_i2c_recv_ch45x(stm32_simu_i2c_type* simu_i2c, u8 cmd, u8* buf, u32 len)
{
    SimuI2CErrState result;

    rt_sem_take(simu_i2c->simu_i2c_sem, RT_WAITING_FOREVER);

    result = SimuI2CRecvCh45x(simu_i2c, cmd, buf, len);
    if (result != SIMU_I2C_OK)
    {
        /*接收失败进行复位处理*/
        GPIO_SetBits(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin);//读之前先输出1
        if (!(GPIO_ReadInputDataBit(simu_i2c->sda->gpio, simu_i2c->sda->gpio_pin)))
        {
            I2cBUSBusyRecovery(simu_i2c);
        }
        else
        {   
            SimuI2CStart(simu_i2c);
            SimuI2CStop(simu_i2c);
        } 
        rt_sem_release(simu_i2c->simu_i2c_sem);
        return 0x00;
    }

    rt_sem_release(simu_i2c->simu_i2c_sem);

    return len;
}
