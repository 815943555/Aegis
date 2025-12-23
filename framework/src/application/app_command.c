/*
 * @file: aegis_app_command.c
 * @brief: CQRS命令系统实现（基于ring_buffer）
 * @author: jack liu
 */

#include "app_command.h"
#include "critical.h"
#include <string.h>

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_app_cmd_init(AegisAppCmdQueue* queue, AegisTraceLog* trace) {
    if (queue == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    memset(queue, 0, sizeof(AegisAppCmdQueue));
    queue->trace = trace;

    /* 初始化环形缓冲区 */
    aegis_ring_buffer_init(&queue->ring, queue->buffer,
                     (uint16_t)sizeof(queue->buffer));

    queue->is_initialized = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_enqueue(AegisAppCmdQueue* queue, const AegisCommand* cmd) {
    AegisCommand cmd_copy;
    uint32_t timestamp;
    uint16_t written;
    uint16_t free_space;

    if (queue == NULL || cmd == NULL) {
        return ERR_NULL_PTR;
    }

    if (!queue->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (cmd->type == CMD_TYPE_INVALID) {
        return ERR_INVALID_PARAM;
    }

    /* 检查队列是否已满 */
    free_space = aegis_ring_buffer_get_free(&queue->ring);
    if (free_space < sizeof(AegisCommand)) {
        return ERR_OUT_OF_RANGE;
    }

    /* 复制命令并设置时间戳 */
    memcpy(&cmd_copy, cmd, sizeof(AegisCommand));
    timestamp = (queue->trace != NULL) ? aegis_trace_get_timestamp(queue->trace) : 0U;
    cmd_copy.timestamp = timestamp;

    /* 写入环形缓冲区 */
    ENTER_CRITICAL();
    written = aegis_ring_buffer_write(&queue->ring, (const uint8_t*)&cmd_copy, sizeof(AegisCommand));
    EXIT_CRITICAL();

    if (written != sizeof(AegisCommand)) {
        return ERR_OUT_OF_RANGE;
    }

    /* 记录入队事件 */
    if (queue->trace != NULL) {
        aegis_trace_log_event(queue->trace, TRACE_EVENT_CMD_ENQUEUE, "CMD-ENQUEUE", cmd->type, timestamp);
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_dequeue(AegisAppCmdQueue* queue, AegisCommand* cmd) {
    uint16_t read_len;
    uint16_t count;

    if (queue == NULL || cmd == NULL) {
        return ERR_NULL_PTR;
    }

    if (!queue->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    /* 检查队列是否为空 */
    count = aegis_ring_buffer_get_count(&queue->ring);
    if (count < sizeof(AegisCommand)) {
        return ERR_EMPTY;
    }

    /* 从环形缓冲区读取命令 */
    ENTER_CRITICAL();
    read_len = aegis_ring_buffer_read(&queue->ring, (uint8_t*)cmd, sizeof(AegisCommand));
    EXIT_CRITICAL();

    if (read_len != sizeof(AegisCommand)) {
        return ERR_EMPTY;
    }
    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_get_count(const AegisAppCmdQueue* queue, uint8_t* count) {
    uint16_t byte_count;
    uint8_t cmd_count;

    if (queue == NULL || count == NULL) {
        return ERR_NULL_PTR;
    }

    if (!queue->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    byte_count = aegis_ring_buffer_get_count(&queue->ring);
    cmd_count = (uint8_t)(byte_count / sizeof(AegisCommand));

    *count = cmd_count;

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_clear(AegisAppCmdQueue* queue) {
    if (queue == NULL) {
        return ERR_NULL_PTR;
    }

    if (!queue->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    aegis_ring_buffer_clear(&queue->ring);

    if (queue->trace != NULL) {
        aegis_trace_log_event(queue->trace, TRACE_EVENT_CMD_EXEC, "CMD-CLEAR", 0, 0);
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_payload_write(AegisCommand* cmd, const void* payload, uint16_t size) {
    uint16_t i;

    if (cmd == NULL) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)APP_CMD_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    if (payload == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    cmd->payload_size = size;
    for (i = 0; i < size; i++) {
        cmd->payload[i] = ((const uint8_t*)payload)[i];
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_payload_read(const AegisCommand* cmd, void* out, uint16_t expected_size) {
    if (cmd == NULL || out == NULL) {
        return ERR_NULL_PTR;
    }

    if (cmd->payload_size != expected_size) {
        return ERR_OUT_OF_RANGE;
    }

    if (expected_size > (uint16_t)APP_CMD_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(out, cmd->payload, expected_size);
    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_result_payload_write(AegisCommandResult* result, const void* payload, uint16_t size) {
    uint16_t i;

    if (result == NULL) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)APP_CMD_RESULT_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    if (payload == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    result->payload_size = size;
    for (i = 0; i < size; i++) {
        result->payload[i] = ((const uint8_t*)payload)[i];
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_result_payload_read(const AegisCommandResult* result, void* out, uint16_t expected_size) {
    if (result == NULL || out == NULL) {
        return ERR_NULL_PTR;
    }

    if (result->payload_size != expected_size) {
        return ERR_OUT_OF_RANGE;
    }

    if (expected_size > (uint16_t)APP_CMD_RESULT_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(out, result->payload, expected_size);
    return ERR_OK;
}
