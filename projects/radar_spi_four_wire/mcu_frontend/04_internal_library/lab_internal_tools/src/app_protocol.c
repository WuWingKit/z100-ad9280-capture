/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_protocol.c
 * @brief 队列、组帧与校验实现。
 */

#include "app_protocol.h"

typedef enum
{
    APP_PROTOCOL_WAIT_HEAD0 = 0u,
    APP_PROTOCOL_WAIT_HEAD1,
    APP_PROTOCOL_WAIT_LEN0,
    APP_PROTOCOL_WAIT_LEN1,
    APP_PROTOCOL_WAIT_COMMAND,
    APP_PROTOCOL_WAIT_PAYLOAD,
    APP_PROTOCOL_WAIT_CHECKSUM
} AppProtocolParseStateTypedef;

/*
 * 用一个新字节更新当前校验累计值。NONE 模式保持原值，使解析主流程无需
 * 为三种校验方式复制代码。
 */
static uint8_t AppProtocol_ChecksumUpdate(
    AppProtocolChecksumTypedef type,
    uint8_t value,
    uint8_t byteValue)
{
    if(type == APP_PROTOCOL_CHECKSUM_XOR8)
    {
        return (uint8_t)(value ^ byteValue);
    }
    if(type == APP_PROTOCOL_CHECKSUM_SUM8)
    {
        return (uint8_t)(value + byteValue);
    }
    return value;
}

/* 计算环形队列的下一索引，并在到达容量时回卷到 0。 */
static uint8_t AppProtocol_NextIndex(uint8_t index, uint8_t size)
{
    index++;
    return (index >= size) ? 0u : index;
}

/*
 * 单消费者出队函数。只由 Process 上下文调用；队列为空返回 0，不阻塞。
 */
static uint8_t AppProtocol_PopByte(
    AppProtocolTypedef *protocol,
    uint8_t *byteValue)
{
    if(protocol->rxTail == protocol->rxHead)
    {
        return 0u;
    }
    *byteValue = protocol->rxBuffer[protocol->rxTail];
    protocol->rxTail = AppProtocol_NextIndex(
        protocol->rxTail, protocol->rxBytes);
    return 1u;
}

/*
 * 将已经验证的 command/payload 同步交给该实例唯一的业务回调。payload
 * 指向实例内部缓存，回调返回后解析器可以在下一帧中覆盖它。
 */
static void AppProtocol_Deliver(AppProtocolTypedef *protocol)
{
    protocol->config->onFrame(
        protocol,
        protocol->command,
        protocol->payloadBuffer,
        protocol->payloadLength,
        protocol->context);
}

/*
 * 协议核心逐字节状态机。依次识别帧头、长度、命令、payload 和校验；
 * 长度越界或校验失败时不调用业务，并回到等待帧头状态。返回值表示本字节
 * 是否刚好完成并交付了一帧。
 */
static uint8_t AppProtocol_ParseByte(
    AppProtocolTypedef *protocol,
    uint8_t byteValue)
{
    const AppProtocolFormatTypedef *format = &protocol->config->format;
    uint8_t delivered = 0u;

    switch((AppProtocolParseStateTypedef)protocol->parseState)
    {
        case APP_PROTOCOL_WAIT_HEAD0:
            if(byteValue == format->header0)
            {
                protocol->checksum = AppProtocol_ChecksumUpdate(
                    format->checksum, 0u, byteValue);
                protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_HEAD1;
            }
            break;

        case APP_PROTOCOL_WAIT_HEAD1:
            if(byteValue == format->header1)
            {
                protocol->checksum = AppProtocol_ChecksumUpdate(
                    format->checksum, protocol->checksum, byteValue);
                protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_LEN0;
            }
            else if(byteValue == format->header0)
            {
                protocol->checksum = AppProtocol_ChecksumUpdate(
                    format->checksum, 0u, byteValue);
            }
            else
            {
                AppProtocol_Reset(protocol);
            }
            break;

        case APP_PROTOCOL_WAIT_LEN0:
            protocol->payloadLength = byteValue;
            protocol->checksum = AppProtocol_ChecksumUpdate(
                format->checksum, protocol->checksum, byteValue);
            protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_LEN1;
            break;

        case APP_PROTOCOL_WAIT_LEN1:
            if(format->lengthEndian == APP_PROTOCOL_ENDIAN_LITTLE)
            {
                protocol->payloadLength |= (uint16_t)byteValue << 8;
            }
            else
            {
                protocol->payloadLength =
                    (uint16_t)((protocol->payloadLength << 8) | byteValue);
            }
            protocol->checksum = AppProtocol_ChecksumUpdate(
                format->checksum, protocol->checksum, byteValue);
            if(protocol->payloadLength > format->maxPayloadLength)
            {
                AppProtocol_Reset(protocol);
            }
            else
            {
                protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_COMMAND;
            }
            break;

        case APP_PROTOCOL_WAIT_COMMAND:
            protocol->command = byteValue;
            protocol->checksum = AppProtocol_ChecksumUpdate(
                format->checksum, protocol->checksum, byteValue);
            protocol->payloadIndex = 0u;
            if(protocol->payloadLength > 0u)
            {
                protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_PAYLOAD;
            }
            else if(format->checksum == APP_PROTOCOL_CHECKSUM_NONE)
            {
                AppProtocol_Deliver(protocol);
                AppProtocol_Reset(protocol);
                delivered = 1u;
            }
            else
            {
                protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_CHECKSUM;
            }
            break;

        case APP_PROTOCOL_WAIT_PAYLOAD:
            protocol->payloadBuffer[protocol->payloadIndex++] = byteValue;
            protocol->checksum = AppProtocol_ChecksumUpdate(
                format->checksum, protocol->checksum, byteValue);
            if(protocol->payloadIndex >= protocol->payloadLength)
            {
                if(format->checksum == APP_PROTOCOL_CHECKSUM_NONE)
                {
                    AppProtocol_Deliver(protocol);
                    AppProtocol_Reset(protocol);
                    delivered = 1u;
                }
                else
                {
                    protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_CHECKSUM;
                }
            }
            break;

        case APP_PROTOCOL_WAIT_CHECKSUM:
            if(byteValue == protocol->checksum)
            {
                AppProtocol_Deliver(protocol);
                delivered = 1u;
            }
            AppProtocol_Reset(protocol);
            break;

        default:
            AppProtocol_Reset(protocol);
            break;
    }
    return delivered;
}

/* 校验只读配置和用户缓冲，随后只保存地址；协议核心不申请任何 RAM。 */
AppProtocolStatusTypedef AppProtocol_Init(
    AppProtocolTypedef *protocol,
    const AppProtocolConfigTypedef *config,
    const AppProtocolStorageTypedef *storage,
    void *context)
{
    uintptr_t rxStart;
    uintptr_t rxEnd;
    uintptr_t payloadStart;
    uintptr_t payloadEnd;

    if(protocol == (AppProtocolTypedef *)0)
    {
        return APP_PROTOCOL_BAD_PARAM;
    }
    protocol->initialized = 0u;

    if((config == (const AppProtocolConfigTypedef *)0) ||
       (storage == (const AppProtocolStorageTypedef *)0) ||
       (config->onFrame == (AppProtocolFrameFunc)0) ||
       (config->format.maxPayloadLength == 0u) ||
       (config->format.lengthEndian > APP_PROTOCOL_ENDIAN_BIG) ||
       (config->format.checksum > APP_PROTOCOL_CHECKSUM_SUM8))
    {
        return APP_PROTOCOL_BAD_PARAM;
    }

    if((storage->rxBuffer == (volatile uint8_t *)0) ||
       (storage->rxBufferBytes < 2u) ||
       (storage->rxBufferBytes > 255u) ||
       (storage->payloadBuffer == (uint8_t *)0) ||
       (storage->payloadBufferBytes < config->format.maxPayloadLength))
    {
        return APP_PROTOCOL_BAD_STORAGE;
    }

    rxStart = (uintptr_t)storage->rxBuffer;
    payloadStart = (uintptr_t)storage->payloadBuffer;
    rxEnd = rxStart + storage->rxBufferBytes;
    payloadEnd = payloadStart + storage->payloadBufferBytes;
    if((rxEnd < rxStart) ||
       (payloadEnd < payloadStart) ||
       ((rxStart < payloadEnd) && (payloadStart < rxEnd)))
    {
        return APP_PROTOCOL_BAD_STORAGE;
    }

    protocol->config = config;
    protocol->context = context;
    protocol->rxBuffer = storage->rxBuffer;
    protocol->payloadBuffer = storage->payloadBuffer;
    protocol->rxBytes = (uint8_t)storage->rxBufferBytes;

    protocol->rxHead = 0u;
    protocol->rxTail = 0u;
    protocol->overflow = 0u;
    protocol->initialized = 1u;
    AppProtocol_Reset(protocol);
    return APP_PROTOCOL_OK;
}

/* ISR 侧最小路径：检查队满、写入一个字节、推进单字节 head。 */
uint8_t AppProtocol_PushByte(
    AppProtocolTypedef *protocol,
    uint8_t byteValue)
{
    uint8_t nextHead;

    if((protocol == (AppProtocolTypedef *)0) ||
       (protocol->initialized == 0u))
    {
        return 0u;
    }

    nextHead = AppProtocol_NextIndex(
        protocol->rxHead, protocol->rxBytes);
    if(nextHead == protocol->rxTail)
    {
        protocol->overflow = 1u;
        return 0u;
    }

    protocol->rxBuffer[protocol->rxHead] = byteValue;
    protocol->rxHead = nextHead;
    return 1u;
}

/* 批量入口保持与单字节入口完全一致的溢出语义。 */
uint16_t AppProtocol_Push(
    AppProtocolTypedef *protocol,
    const uint8_t *data,
    uint16_t length)
{
    uint16_t accepted = 0u;

    if((data == (const uint8_t *)0) && (length > 0u))
    {
        return 0u;
    }
    while((accepted < length) &&
          (AppProtocol_PushByte(protocol, data[accepted]) != 0u))
    {
        accepted++;
    }
    return accepted;
}

/*
 * 主循环侧消费入口。溢出意味着字节流已经不连续，因此先整体丢弃再同步；
 * 正常情况下按预算逐字节解析，并累计成功交付帧数。
 */
uint16_t AppProtocol_Process(
    AppProtocolTypedef *protocol,
    uint16_t byteBudget)
{
    uint16_t bytes = 0u;
    uint16_t frames = 0u;
    uint8_t byteValue;

    if((protocol == (AppProtocolTypedef *)0) ||
       (protocol->initialized == 0u))
    {
        return 0u;
    }
    if(protocol->overflow != 0u)
    {
        protocol->rxHead = 0u;
        protocol->rxTail = 0u;
        protocol->overflow = 0u;
        AppProtocol_Reset(protocol);
        return 0u;
    }

    while(((byteBudget == 0u) || (bytes < byteBudget)) &&
          (AppProtocol_PopByte(protocol, &byteValue) != 0u))
    {
        frames = (uint16_t)(frames + AppProtocol_ParseByte(protocol, byteValue));
        bytes++;
    }
    return frames;
}

/* 仅重置半帧解析字段；已排队但尚未消费的字节保持不变。 */
void AppProtocol_Reset(AppProtocolTypedef *protocol)
{
    if(protocol == (AppProtocolTypedef *)0)
    {
        return;
    }
    protocol->payloadLength = 0u;
    protocol->payloadIndex = 0u;
    protocol->parseState = (uint8_t)APP_PROTOCOL_WAIT_HEAD0;
    protocol->command = 0u;
    protocol->checksum = 0u;
}

/* 使用 32 位中间值计算完整帧长度，防止 16 位长度加固定开销后回绕。 */
uint16_t AppProtocol_FrameBytes(
    const AppProtocolFormatTypedef *format,
    uint16_t payloadLength)
{
    uint32_t bytes;

    if((format == (const AppProtocolFormatTypedef *)0) ||
       (payloadLength > format->maxPayloadLength))
    {
        return 0u;
    }
    bytes = 5u + payloadLength +
            ((format->checksum == APP_PROTOCOL_CHECKSUM_NONE) ? 0u : 1u);
    return (bytes > 65535u) ? 0u : (uint16_t)bytes;
}

/*
 * 发送侧镜像编码：写包络固定字段、按配置写长度、复制 payload，并以与
 * 接收解析器相同的更新函数生成可选校验字节。
 */
AppProtocolStatusTypedef AppProtocol_Encode(
    const AppProtocolTypedef *protocol,
    uint8_t command,
    const uint8_t *payload,
    uint16_t payloadLength,
    uint8_t *output,
    uint16_t outputCapacity,
    uint16_t *outputLength)
{
    const AppProtocolFormatTypedef *format;
    uint16_t frameBytes;
    uint16_t i;
    uint8_t checksum = 0u;

    if((protocol == (const AppProtocolTypedef *)0) ||
       (protocol->initialized == 0u))
    {
        return APP_PROTOCOL_NOT_INITIALIZED;
    }
    format = &protocol->config->format;
    frameBytes = AppProtocol_FrameBytes(format, payloadLength);
    if((output == (uint8_t *)0) ||
       (outputLength == (uint16_t *)0) ||
       ((payload == (const uint8_t *)0) && (payloadLength > 0u)) ||
       (frameBytes == 0u))
    {
        return APP_PROTOCOL_BAD_PARAM;
    }
    if(outputCapacity < frameBytes)
    {
        return APP_PROTOCOL_OUTPUT_TOO_SMALL;
    }

    output[0] = format->header0;
    output[1] = format->header1;
    if(format->lengthEndian == APP_PROTOCOL_ENDIAN_LITTLE)
    {
        output[2] = (uint8_t)(payloadLength & 0xffu);
        output[3] = (uint8_t)(payloadLength >> 8);
    }
    else
    {
        output[2] = (uint8_t)(payloadLength >> 8);
        output[3] = (uint8_t)(payloadLength & 0xffu);
    }
    output[4] = command;
    for(i = 0u; i < 5u; ++i)
    {
        checksum = AppProtocol_ChecksumUpdate(
            format->checksum, checksum, output[i]);
    }
    for(i = 0u; i < payloadLength; ++i)
    {
        output[5u + i] = payload[i];
        checksum = AppProtocol_ChecksumUpdate(
            format->checksum, checksum, payload[i]);
    }
    if(format->checksum != APP_PROTOCOL_CHECKSUM_NONE)
    {
        output[frameBytes - 1u] = checksum;
    }
    *outputLength = frameBytes;
    return APP_PROTOCOL_OK;
}
