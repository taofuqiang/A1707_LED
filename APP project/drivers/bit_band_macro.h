
/***************************************************************************************************
*                    (c) Copyright 2008-2012 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          bit_band_macro.h
*\Description   
*\Note          
*\Log           2012.12.07    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _BIT_BAND_MACRO_H_
#define _BIT_BAND_MACRO_H_

/******************************************************************************

* Macros for hardware access, both direct and via the bit-band region.
*
******************************************************************************/
#define HWREG(x)                                                              \
        (*((volatile unsigned long *)(x)))
#define HWREGH(x)                                                             \
        (*((volatile unsigned short *)(x)))
#define HWREGB(x)                                                             \
        (*((volatile unsigned char *)(x)))
#define HWREGBIT(x,b)                                                       \
        HWREG(((unsigned long)(x) & 0xF0000000) | 0x02000000 |                \
              (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define HWREGBITH(x,b)                                                       \
        HWREGH(((unsigned long)(x) & 0xF0000000) | 0x02000000 |               \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define HWREGBITB(x,b)                                                       \
        HWREGB(((unsigned long)(x) & 0xF0000000) | 0x02000000 |               \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))

#define SET_BIT_BAND(x,b)  HWREGBIT(x,b)=1 /*使用这两个宏进行BIT-BAND的映射时，注意所要操作的地址是否是可读而且可写的，
                                            ，若是写1清零则注意不要使用该宏进行置位和清零的操作，以防产生其他的不希望的结果*/
#define CLR_BIT_BAND(x,b)  HWREGBIT(x,b)=0

#endif   // ----- #ifndef_BIT_BAND_MACRO_H_ -----

