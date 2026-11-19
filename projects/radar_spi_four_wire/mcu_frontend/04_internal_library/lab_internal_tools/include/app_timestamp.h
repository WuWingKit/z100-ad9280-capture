/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_timestamp.h
 * @brief 用户驱动的共享单调时间戳抽象。
 *
 * 本模块不占用硬件定时器、不产生中断，也不申请内存。用户只需提供一个
 * 返回单调递增 uint32_t tick 的函数；同一个 AppTimestampTypedef 可以被
 * 多个状态机实例共享。
 */

#ifndef APP_TIMESTAMP_H
#define APP_TIMESTAMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t AppTimestampTickTypedef;

typedef AppTimestampTickTypedef (*AppTimestampNowFunc)(void *context);

typedef enum
{
    APP_TIMESTAMP_OK = 0u,
    APP_TIMESTAMP_BAD_PARAM,
    APP_TIMESTAMP_NOT_INITIALIZED
} AppTimestampStatusTypedef;

typedef struct
{
    AppTimestampNowFunc now;
    void *context;
    uint8_t initialized;
} AppTimestampTypedef;

/** @brief 绑定用户时间戳驱动；不读取时间、不申请资源。 */
AppTimestampStatusTypedef AppTimestamp_Init(
    AppTimestampTypedef *timestamp,
    AppTimestampNowFunc now,
    void *context);

/** @brief 读取当前 tick；实例无效时返回 0。 */
AppTimestampTickTypedef AppTimestamp_Now(
    const AppTimestampTypedef *timestamp);

/**
 * @brief 回绕安全地判断 now 是否已经到达 deadline。
 *
 * 要求任意单次等待跨度小于 2^31 tick。
 */
uint8_t AppTimestamp_HasReached(
    AppTimestampTickTypedef now,
    AppTimestampTickTypedef deadline);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIMESTAMP_H */
