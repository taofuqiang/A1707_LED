/***************************************************************************************************
*                    (c) Copyright 1992-2014 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          cpuusage.c
*\Description   cpu占用率测量
*\Note          
*\Log           2014.12.09    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#include <rtthread.h>
#include <rthw.h>
#include "stm32f10x.h"
#include "public_type.h"

#define CPU_USAGE_CALC_TICK	10
#define CPU_USAGE_LOOP		100

static rt_uint8_t  cpu_usage_major = 0, cpu_usage_minor= 0;
static rt_uint8_t  cpu_max_usage_major = 0, cpu_max_usage_minor = 0;//最大占用率
static rt_uint32_t total_count = 0;

static void cpu_usage_idle_hook()
{
	rt_tick_t tick;
	rt_uint32_t count;
	volatile rt_uint32_t loop;

	if (total_count == 0)
	{
		/* get total count */
		rt_enter_critical();
		tick = rt_tick_get();
		while(rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
		{
			total_count ++;
			loop = 0;

			while (loop < CPU_USAGE_LOOP) loop ++;
		}
		rt_exit_critical();
	}

	count = 0;
	/* get CPU usage */
	tick = rt_tick_get();
	while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
	{
		count ++;
		loop  = 0;
		while (loop < CPU_USAGE_LOOP) loop ++;
	}

	/* calculate major and minor */
	if (count < total_count)
	{
		count = total_count - count;
		cpu_usage_major = (count * 100) / total_count;
		cpu_usage_minor = ((count * 100) % total_count) * 100 / total_count;
        cpu_max_usage_major = Max(cpu_max_usage_major, cpu_usage_major);
        cpu_max_usage_minor = Max(cpu_max_usage_minor, cpu_usage_minor);//此处会有误差1%
	}
	else
	{
		total_count = count;

		/* no CPU usage */
		cpu_usage_major = 0;
		cpu_usage_minor = 0;
	}
}

void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor)
{
	RT_ASSERT(major != RT_NULL);
	RT_ASSERT(minor != RT_NULL);

	*major = cpu_usage_major;
	*minor = cpu_usage_minor;
}

void cpu_usage_init(void)
{
	/* set idle thread hook */
	rt_thread_idle_sethook(cpu_usage_idle_hook);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void cpu_usage(void)
{
    rt_kprintf("Cur CPU Usage:%d.%d\n", cpu_usage_major, cpu_usage_minor);
    rt_kprintf("Max CPU Usage:%d.%d\n", cpu_max_usage_major, cpu_max_usage_minor);
}
FINSH_FUNCTION_EXPORT(cpu_usage, cpu usage info.) 
#endif
