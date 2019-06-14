#include "aliyun_iot_platform_threadsync.h"

INT32 aliyun_iot_sem_init( ALIYUN_IOT_SEM_S * sem)
{
    sem->sem_sub = rt_sem_create("ali_sem", 0, RT_IPC_FLAG_FIFO);
    if (sem->sem_sub == RT_NULL)
    {
        return NULL_VALUE_ERROR; 
    }
    else
    {
        return (SUCCESS_RETURN);
    }
}

INT32 aliyun_iot_sem_destory( ALIYUN_IOT_SEM_S * sem)
{
    if (sem->sem_sub == RT_NULL)
    {
        return NULL_VALUE_ERROR; 
    }

    rt_sem_delete(sem->sem_sub);

    return (SUCCESS_RETURN);
}


INT32 aliyun_iot_sem_gettimeout( ALIYUN_IOT_SEM_S *sem, UINT32 time_ms)
{
    rt_uint32_t delay = rt_tick_from_millisecond(time_ms);

    if (sem->sem_sub == RT_NULL)
    {
        return NULL_VALUE_ERROR; 
    }
    rt_sem_take(sem->sem_sub, delay);

	return (SUCCESS_RETURN);
}

INT32 aliyun_iot_sem_post( ALIYUN_IOT_SEM_S * sem)
{
    if (sem->sem_sub == RT_NULL)
    {
        return NULL_VALUE_ERROR; 
    }
    rt_sem_release(sem->sem_sub);

    return (SUCCESS_RETURN);
}


