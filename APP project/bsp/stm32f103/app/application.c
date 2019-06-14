/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <drv_board.h>
#include <rtthread.h>

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include "eprt_server.h"
#endif
#ifdef RT_USING_DFS
#include <dfs_posix.h>
#include "dfs_init.h"
#endif
#include "param.h"
#include "drv_watchdog.h"
#include "app_board.h"

void rt_init_thread_entry(void* parameter)
{
    rt_thread_t tid;

    //驱动配置初始化，必须在前面
    app_board_init();

#ifdef RT_USING_FINSH
    /* init finsh */
    finsh_system_init();
    finsh_set_device(FINSH_DEVICE_NAME);
#endif

    /*========================================================*/
    //线程名称：指示灯   
    //优先级	14 
    /*========================================================*/
//    tid = rt_thread_create("msgled", msg_led_thread_entry, RT_NULL, 512, 14, 5);
//    if (tid != RT_NULL)
//        rt_thread_startup(tid); 

}

#include "canbus_middle.h"
int rt_application_init()
{
    rt_thread_t tid;

    //获取复位方式
    GetRestFlag();
    //下载参数
    stm32_param_load();

    tid = rt_thread_create("init", rt_init_thread_entry, RT_NULL, 2048, 3, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    /*========================================================*/
    //线程名称：看门狗任务   
    //优先级	15 
    /*========================================================*/
    tid = rt_thread_create("watchdog", rt_watchdog_thread_entry, RT_NULL, 512, 15, 5);
    if (tid != RT_NULL)
        rt_thread_startup(tid);    
	
    tid = rt_thread_create("can", canbus_middle_thread_entry, RT_NULL, 512, 15, 5);
    if (tid != RT_NULL)
        rt_thread_startup(tid);    
    //cpu占用率统计
#ifdef CPU_USAGE_ENABLE
    {
        void cpu_usage_init(void);
        cpu_usage_init();
    }
#endif

    return 0;
}

/*@}*/
