#include "aliyun_iot_platform_pthread.h"

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
    mutex->mutex_sub = rt_mutex_create("iot_mutex", RT_IPC_FLAG_FIFO);
    if (mutex->mutex_sub == RT_NULL)
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
    else
    {
        IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
    }	
}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
    if(mutex->mutex_sub == RT_NULL)
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

    rt_mutex_delete(mutex->mutex_sub);

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex->mutex_sub == RT_NULL)
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

    rt_mutex_take(mutex->mutex_sub, RT_WAITING_FOREVER);

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex->mutex_sub == RT_NULL)
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
    rt_mutex_release(mutex->mutex_sub);
	
    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle, void(*func)(void*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S* param)
{
    static rt_uint8_t pthread_num = 0;
    static rt_uint8_t pthread_prio = 20;
    char pthread_name[RT_NAME_MAX];

    if (param == NULL)
    {
        rt_snprintf(pthread_name, RT_NAME_MAX, "%s%d", "ali_pt", pthread_num++);
        handle->tid = rt_thread_create(pthread_name, func, arg, 1024+512, pthread_prio, RT_TICK_PER_SECOND / 20);
    }
    else
    {
        handle->tid = rt_thread_create(param->threadName, func, arg, param->stackDepth, param->priority, param->tick);
    }
	
    if (handle->tid != RT_NULL)
    {
        rt_thread_startup(handle->tid);
        return SUCCESS_RETURN;
    }
    else
    {
        return ERROR_NULL_VALUE;
    }
}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)
{
    rt_thread_delete(handle->tid);

	return SUCCESS_RETURN;
}

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
    rt_uint32_t delay = rt_tick_from_millisecond(MsToDelay);

	rt_thread_delay(delay);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

/***********************************************************
* 函数名称: aliyun_iot_pthread_setname
* 描       述: 设置线程名字
* 输入参数: char* name
* 输出参数:
* 返 回  值:
* 说       明:
************************************************************/
INT32 aliyun_iot_pthread_setname(char* name)
{
    return SUCCESS_RETURN;
}


/*
INT32 aliyun_iot_pthread_get_stack_size()
{
     return uxTaskGetStackHighWaterMark(NULL);
}
*/
