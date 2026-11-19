/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_protocol.h
 * @brief 用户提供全部缓存的多实例字节流协议解析模块。
 *
 * 每个 AppProtocolTypedef 代表一条独立通信链路。库不申请 RAM，也不依赖
 * HAL/RTOS；用户负责提供 RX 环形队列和 payload 缓冲的地址、长度。
 */

#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_PROTOCOL_ENDIAN_LITTLE = 0u,
    APP_PROTOCOL_ENDIAN_BIG
} AppProtocolEndianTypedef;

typedef enum
{
    APP_PROTOCOL_CHECKSUM_NONE = 0u,
    APP_PROTOCOL_CHECKSUM_XOR8,
    APP_PROTOCOL_CHECKSUM_SUM8
} AppProtocolChecksumTypedef;

typedef enum
{
    APP_PROTOCOL_OK = 0u,
    APP_PROTOCOL_BAD_PARAM,
    APP_PROTOCOL_BAD_STORAGE,
    APP_PROTOCOL_NOT_INITIALIZED,
    APP_PROTOCOL_OUTPUT_TOO_SMALL
} AppProtocolStatusTypedef;

typedef struct AppProtocol AppProtocolTypedef;

/**
 * @brief 完整协议帧通过长度和校验后调用的业务函数类型。
 *
 * 回调只会在 AppProtocol_Process() 的调用上下文执行，不会从 PushByte()
 * 所在 ISR 执行。payload 指向用户提供的 payloadBuffer，下一帧可能覆盖；
 * 需要跨轮保存的数据必须复制到业务 Context。
 */
typedef void (*AppProtocolFrameFunc)(
    AppProtocolTypedef *protocol,
    uint8_t command,
    const uint8_t *payload,
    uint16_t payloadLength,
    void *context);

/**
 * @brief 当前内置包络的可配置格式。
 *
 * 固定字段布局：head0 | head1 | length(2) | command | payload | checksum?。
 * 不同协议实例可以选择不同帧头、长度端序、校验和最大 payload。
 */
typedef struct
{
    uint8_t header0;
    uint8_t header1;
    AppProtocolEndianTypedef lengthEndian;
    AppProtocolChecksumTypedef checksum;
    uint16_t maxPayloadLength;
} AppProtocolFormatTypedef;

/**
 * @brief 可由多个实例共享的只读协议“类定义”。
 */
typedef struct
{
    AppProtocolFormatTypedef format;
    AppProtocolFrameFunc onFrame;
} AppProtocolConfigTypedef;

/**
 * @brief 单个协议实例使用的用户存储描述。
 *
 * RX 与 payload 必须是两块生命周期覆盖协议实例的独立存储。RX 队列使用
 * 8 位原子索引，容量限制为 2..255；payloadBytes 必须不小于格式声明的
 * maxPayloadLength。存储可位于静态 RAM、DMA RAM 或用户指定 section。
 */
typedef struct
{
    volatile uint8_t *rxBuffer;
    uint16_t rxBufferBytes;
    uint8_t *payloadBuffer;
    uint16_t payloadBufferBytes;
} AppProtocolStorageTypedef;

/**
 * @brief 协议实例运行时；每条通信链路声明一个对象。
 */
struct AppProtocol
{
    const AppProtocolConfigTypedef *config;
    void *context;

    volatile uint8_t *rxBuffer;
    uint8_t *payloadBuffer;
    uint8_t rxBytes;
    volatile uint8_t rxHead;
    volatile uint8_t rxTail;
    volatile uint8_t overflow;

    uint16_t payloadLength;
    uint16_t payloadIndex;
    uint8_t parseState;
    uint8_t command;
    uint8_t checksum;
    uint8_t initialized;
};

#define APP_PROTOCOL_FORMAT( \
    head0, head1, endianValue, checksumValue, maxPayload) \
    { (head0), (head1), (endianValue), (checksumValue), (maxPayload) }

#define APP_PROTOCOL_CONFIG(formatValue, callbackValue) \
    { formatValue, (callbackValue) }

#define APP_PROTOCOL_STORAGE(rxArray, payloadArray)                    \
    {                                                                  \
        (rxArray), (uint16_t)sizeof(rxArray),                          \
        (payloadArray), (uint16_t)sizeof(payloadArray)                 \
    }

/**
 * @brief 将只读协议配置和用户缓冲绑定到一个独立协议实例。
 *
 * 本函数不申请、释放或清零用户缓冲，只保存地址并初始化队列/解析游标。
 * config 和 storage 指向的底层缓冲在协议使用期间必须持续有效。
 *
 * @param protocol 要初始化的协议实例。
 * @param config   帧格式与成功回调。
 * @param storage  用户提供的 RX/payload 地址和容量。
 * @param context  每次 onFrame 原样传回的业务指针，可为 NULL。
 * @return 成功返回 OK；缓冲过小/重叠或容量非法返回 BAD_STORAGE。
 */
AppProtocolStatusTypedef AppProtocol_Init(
    AppProtocolTypedef *protocol,
    const AppProtocolConfigTypedef *config,
    const AppProtocolStorageTypedef *storage,
    void *context);

/**
 * @brief 将一个硬件接收字节放入该实例 RX 队列。
 *
 * 此函数只入队，不组帧、不调用业务，适合单生产者 ISR。队满时不覆盖旧
 * 数据，而是置 overflow 并返回 0；Process 会丢弃不连续输入并重新同步。
 */
uint8_t AppProtocol_PushByte(
    AppProtocolTypedef *protocol,
    uint8_t byteValue);

/**
 * @brief 批量入队 DMA/USB/TCP 已接收的数据块。
 * @return 实际成功入队的字节数；小于 length 表示队列已满。
 */
uint16_t AppProtocol_Push(
    AppProtocolTypedef *protocol,
    const uint8_t *data,
    uint16_t length);

/**
 * @brief 在主循环中消费队列、解析帧并同步执行成功回调。
 * @param byteBudget 本轮最多消费字节数；0 表示排空当前队列。
 * @return 本轮成功校验并交付给 onFrame 的帧数。
 */
uint16_t AppProtocol_Process(
    AppProtocolTypedef *protocol,
    uint16_t byteBudget);

/**
 * @brief 丢弃当前半帧并回到等待第一个帧头，不清空 RX 队列。
 */
void AppProtocol_Reset(AppProtocolTypedef *protocol);

/**
 * @brief 计算给定 payload 编码后的完整帧长度。
 * @return 合法完整帧字节数；超出格式限制或 16 位范围返回 0。
 */
uint16_t AppProtocol_FrameBytes(
    const AppProtocolFormatTypedef *format,
    uint16_t payloadLength);

/**
 * @brief 按实例格式编码一帧到用户输出缓冲，但不负责硬件发送。
 */
AppProtocolStatusTypedef AppProtocol_Encode(
    const AppProtocolTypedef *protocol,
    uint8_t command,
    const uint8_t *payload,
    uint16_t payloadLength,
    uint8_t *output,
    uint16_t outputCapacity,
    uint16_t *outputLength);

#ifdef __cplusplus
}
#endif

#endif /* APP_PROTOCOL_H */
