#ifndef ALIYUN_IOT_PLATFORM_PTHREAD_H
#define ALIYUN_IOT_PLATFORM_PTHREAD_H

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"

#include "rtthread.h"

typedef struct 
{
    rt_mutex_t mutex_sub;
}ALIYUN_IOT_MUTEX_S;

typedef struct 
{
    rt_thread_t tid;
}ALIYUN_IOT_PTHREAD_S;

typedef struct ALIYUN_IOT_PTHREAD_PARAM
{
    INT8 threadName[RT_NAME_MAX];
    UINT32 stackDepth;
    UINT32 priority;
    rt_uint32_t tick;
}ALIYUN_IOT_PTHREAD_PARAM_S;

INT32 aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S*mutex);

INT32 aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S*mutex);

INT32 aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle,void(*func)(void*),void *arg,ALIYUN_IOT_PTHREAD_PARAM_S* param);

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle);

INT32 aliyun_iot_pthread_taskdelay( INT32 MsToDelay);

INT32 aliyun_iot_pthread_setname(char* name);

//INT32 aliyun_iot_pthread_get_stack_size();

#endif



