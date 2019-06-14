/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef _DRV_BOARD_H
#define _DRV_BOARD_H
#include "stm32f10x.h"
#include "rtconfig.h"

/* board configuration */
// <o> SDCard Driver <1=>SDIO sdcard <0=>SPI MMC card
// 	<i>Default: 1
//#define STM32_USE_SDIO			0

//FSMC使能
//#define STM32_FSMC              1

/* whether use board external SRAM memory */
//外部RAM定义
//#define STM32_EXT_SRAM          1


#define STM32_EXT_SRAM_BEGIN    0x68000000 /* the begining address of external SRAM */
#define STM32_EXT_SRAM_END      0x68080000 /* the end address of external SRAM */


// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#define STM32_SRAM_SIZE         64
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE * 1024)


/***************************************************************************************************
调试串口定义
***************************************************************************************************/
#define RT_USING_UART2

#if defined RT_USING_UART1 
#define CONSOLE_DEVICE "uart1"
#elif defined RT_USING_UART2
#define CONSOLE_DEVICE "uart2"
#elif defined RT_USING_UART3
#define CONSOLE_DEVICE "uart3"
#elif defined RT_USING_UART6
#define CONSOLE_DEVICE "uart6"
#else
#define CONSOLE_DEVICE "no"
#endif

#define FINSH_DEVICE_NAME   CONSOLE_DEVICE
/***************************************************************************************************
调试串口定义结束
***************************************************************************************************/

/*使用usb库 host FS*/
//#define USE_USB_FS //全速USB
//#define USE_USB_HS //高速USB

//外部ram初始化
#if STM32_EXT_SRAM
void ext_sram_init(void);
#endif

void rt_hw_board_init(void);

#endif /*_DRV_BOARD_H*/
