/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          baiduyun.iot_auth.c
*\Description   
*\Note          1/一直适用于百度云
*\Log           2017.05.05    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aliyun_iot_platform_stdio.h"
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_platform_persistence.h"
#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_config.h"
#include "aliyun_iot_common_base64.h"
#include "aliyun_iot_common_json.h"
#include "aliyun_iot_common_md5.h"
#include "aliyun_iot_common_httpclient.h"
#include "aliyun_iot_common_hmac.h"
#include "aliyun_iot_auth.h"
#include "aliyun_iot_mqtt_internal.h"

#include "iap_if.h"
#include "param.h"

#ifndef USING_ALIYUN_SERV
/***********************************************************
* 全局变量: g_userInfo
* 描       述: 用户信息
* 说       明:
************************************************************/
ALIYUN_IOT_USER_INFO_S g_userInfo;
/***********************************************************
* 全局变量: g_authInfo
* 描       述: 鉴权信息内存指针
* 说       明: 初始化时malloc得到authinfo内存保存地址在此变量中
************************************************************/
AUTH_INFO_S *g_authInfo = NULL;

/***********************************************************
* 函数名称: aliyun_iot_set_auth_state
* 描       述: 设置鉴权模式
* 输入参数: USER_AUTH_STATE_E authState
* 输出参数: VOID
* 返 回  值: VOID
* 说       明: EVERY_TIME_AUTH    每次调用aliyun_iot_auth接口都会重新获取证书鉴权
*           FIRST_CONNECT_AUTH 只在设备第一次上线时鉴权，以后使用记录的鉴权信息
************************************************************/
void aliyun_iot_set_auth_state(USER_AUTH_STATE_E authState)
{
    
}

int set_usr_info(AUTH_INFO_S * authInfo)
{
    ALIYUN_IOT_USER_INFO_S *userInfo = &g_userInfo;
    u32 chip_id[3];

    memset(&g_userInfo,0x0,sizeof(ALIYUN_IOT_USER_INFO_S));
    //生成client ID
    Get_ChipID(chip_id);
    aliyun_iot_stdio_snprintf(userInfo->clientId, CLIENT_ID_LEN, "%08X%08X%08X", chip_id[0], chip_id[1], chip_id[2]);
    WRITE_IOT_DEBUG_LOG("userInfo-clientId:%s",userInfo->clientId);

    //生成username
    memcpy(userInfo->userName, dev_param.MQTT_User, strlen((const char*)dev_param.MQTT_User));
    WRITE_IOT_DEBUG_LOG("userInfo-userName:%s",userInfo->userName);

    //password
    memcpy(userInfo->passWord, dev_param.MQTT_Password, strlen((const char*)dev_param.MQTT_Password));
    WRITE_IOT_DEBUG_LOG("userInfo-passWord:%s",userInfo->passWord);

    //host address
    memcpy(userInfo->hostAddress, dev_param.DstAddr, strlen((const char*)dev_param.DstAddr));
    memcpy(userInfo->port, dev_param.DstPort, strlen((const char*)dev_param.DstPort));

    WRITE_IOT_DEBUG_LOG("userInfo->port:%s",userInfo->port);
    WRITE_IOT_DEBUG_LOG("userInfo-hostAddress:%s",userInfo->hostAddress);

    return SUCCESS_RETURN;
}
INT32 aliyun_iot_verify_certificate(AUTH_INFO_S *authInfo,SERVER_PARAM_TYPE_E pullType,SIGN_DATA_TYPE_E signDataType)
{
     return SUCCESS_RETURN;
}
INT32 pull_server_param(SIGN_DATA_TYPE_E signDataType,SERVER_PARAM_TYPE_E pullType,AUTH_INFO_S *info)
{
    return SUCCESS_RETURN;
}
/***********************************************************
* 函数名称: aliyun_iot_auth_init
* 描       述: auth初始化函数
* 输入参数: VOID
* 输出参数: VOID
* 返 回  值: 0 成功，-1 失败
* 说       明: 初始化日志级别，设备信息，鉴权信息文件的保存路径
************************************************************/
INT32 aliyun_iot_auth_init()
{
    aliyun_iot_common_set_log_level(ERROR_IOT_LOG);
    aliyun_iot_common_log_init();

    WRITE_IOT_NOTICE_LOG("auth init success!");
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_auth_release
* 描       述: auth释放函数
* 输入参数: VOID
* 输出参数: VOID
* 返 回  值: 0:成功 -1:失败
* 说      明: 释放authInfo内存
************************************************************/
INT32 aliyun_iot_auth_release()
{
    WRITE_IOT_NOTICE_LOG("auth release!");
    aliyun_iot_common_log_release();
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_set_device_info
* 描       述: 设置设备信息
* 输入参数: IOT_DEVICEINFO_SHADOW_S*deviceInfo
* 输出参数: VOID
* 返 回  值: 0：成功  -1：失败
* 说       明: 将在aliyun注册的设备信息设置到SDK中的设备变量中
************************************************************/
INT32 aliyun_iot_set_device_info(IOT_DEVICEINFO_SHADOW_S*deviceInfo)
{
    WRITE_IOT_DEBUG_LOG("set device info success!");

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_auth
* 描       述: sdk用户鉴权函数
* 输入参数: SIGN_DATA_TYPE_E signDataType 签名类型
*          IOT_BOOL_VALUE_E haveFilesys 是否有文件系统
* 返 回  值: 0：成功  -1：失败
* 说       明: 鉴权得到公钥证书并生成用户信息
************************************************************/
INT32 aliyun_iot_auth(SIGN_DATA_TYPE_E signDataType,IOT_BOOL_VALUE_E haveFilesys)
{
    int rc = SUCCESS_RETURN;

    set_usr_info(NULL);
    return rc;
}
#endif


