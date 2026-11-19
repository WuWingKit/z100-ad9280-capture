/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_timestamp.c
 * @brief 用户驱动的共享单调时间戳实现。
 */

#include "app_timestamp.h"

AppTimestampStatusTypedef AppTimestamp_Init(
    AppTimestampTypedef *timestamp,
    AppTimestampNowFunc now,
    void *context)
{
    if((timestamp == (AppTimestampTypedef *)0) ||
       (now == (AppTimestampNowFunc)0))
    {
        return APP_TIMESTAMP_BAD_PARAM;
    }

    timestamp->now = now;
    timestamp->context = context;
    timestamp->initialized = 1u;
    return APP_TIMESTAMP_OK;
}

AppTimestampTickTypedef AppTimestamp_Now(
    const AppTimestampTypedef *timestamp)
{
    if((timestamp == (const AppTimestampTypedef *)0) ||
       (timestamp->initialized == 0u) ||
       (timestamp->now == (AppTimestampNowFunc)0))
    {
        return 0u;
    }
    return timestamp->now(timestamp->context);
}

uint8_t AppTimestamp_HasReached(
    AppTimestampTickTypedef now,
    AppTimestampTickTypedef deadline)
{
    return (uint8_t)((int32_t)(now - deadline) >= 0);
}
