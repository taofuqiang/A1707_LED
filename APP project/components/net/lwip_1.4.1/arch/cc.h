/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          cc.h
*\Description   
*\Note          lwip移植头文件
*\Log           2015.04.03    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#include "rthw.h"
#include "rtthread.h"

typedef rt_uint8_t	u8_t;
typedef rt_int8_t	s8_t;
typedef rt_uint16_t	u16_t;
typedef rt_int16_t	s16_t;
typedef rt_uint32_t	u32_t;
typedef rt_int32_t	s32_t;
typedef rt_uint32_t	mem_ptr_t;

#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"

#ifdef RT_USING_NEWLIB
#include <errno.h>
/* some errno not defined in newlib */
#define ENSRNOTFOUND 163  /* Domain name not found */
/* WARNING: ESHUTDOWN also not defined in newlib. We chose
			180 here because the number "108" which is used
			in arch.h has been assigned to another error code. */
#define ESHUTDOWN 180
#elif RT_USING_MINILIBC
#include <errno.h>
#define  EADDRNOTAVAIL  99  /* Cannot assign requested address */
#else
#define LWIP_PROVIDE_ERRNO
#endif

#ifdef RT_USING_MINILIBC
#include <time.h>
#define LWIP_TIMEVAL_PRIVATE 0
#endif

#if defined(__CC_ARM)   /* ARMCC compiler */
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#elif defined(__IAR_SYSTEMS_ICC__)   /* IAR Compiler */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_USE_INCLUDES
#elif defined(__GNUC__)     /* GNU GCC Compiler */
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#elif defined(_MSC_VER)
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#endif

void sys_arch_assert(const char* file, int line);
#define LWIP_PLATFORM_DIAG(x)	do {rt_kprintf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x) do {rt_kprintf(x); sys_arch_assert(__FILE__, __LINE__);}while(0)


#include "string.h"

#endif /*_CC_H*/

