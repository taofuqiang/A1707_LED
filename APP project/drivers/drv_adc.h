/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_adc.h
*\Description   
*\Note          
*\Log           2017.05.11    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_ADC_H
#define _DRV_ADC_H
#include "public_type.h"

typedef struct ADC_DMA_Type_st
{
    DMA_Channel_TypeDef* dma;
    ADC_TypeDef*        adc;
    u32                 max_num;
    u16*                adc_data;  
    u32                 data_len;
    /*以上需配置*/
    //DMA_InitTypeDef     DMA_HType;
    //ADC_InitTypeDef     ADC_HType;
    u32                 adc_count;
}ADC_DMA_Type;

bool stm32_adc_with_dma_init(ADC_DMA_Type* adc_dev);
bool stm32_adc_channel_add(ADC_DMA_Type* adc_dev,u32 seq_num, u32 channel, u32 sample, GpioType* port);
bool stm32_adc_with_dma_start(ADC_DMA_Type* adc_dev);
u16 stm32_adc_get_value(ADC_DMA_Type* adc_dev, u32 seq_num);

u32 stm32_get_chip_temp(u16 value);

extern ADC_DMA_Type ADC_InterT;
extern u32 Temp_Chip;
void chip_temp_thread_entry(void* p);

#endif /*_DRV_ADC_H*/

