/*
 * @file: aegis_trace_log.c
 * @brief: 追溯日志实现（基于ring_buffer）
 * @author: jack liu
 */

#include "trace.h"
#include "critical.h"
#include <string.h>

/* ==================== 公共接口实现 ==================== */
uint32_t aegis_trace_get_timestamp(AegisTraceLog* log) {
    uint32_t ts;

    if (log == NULL) {
        return 0U;
    }

    if (log->clock.now != NULL) {
        ts = log->clock.now(log->clock.ctx);
        return ts;
    }

    /* fallback：简单自增（x86模拟/无硬件时钟时使用） */
    ts = log->fallback_tick;
    log->fallback_tick++;
    return ts;
}

AegisErrorCode aegis_trace_log_init(AegisTraceLog* log, TraceTimestampFn now_fn, void* now_ctx) {
    if (log == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    memset(log, 0, sizeof(AegisTraceLog));

    /* 初始化环形缓冲区 */
    aegis_ring_buffer_init(&log->ring, log->buffer,
                     (uint16_t)(TRACE_LOG_SIZE * sizeof(AegisTraceEvent)));

    log->clock.now = now_fn;
    log->clock.ctx = now_ctx;
    log->fallback_tick = 0U;
    log->is_initialized = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

void aegis_trace_log_event(AegisTraceLog* log, AegisTraceEventType type, const char* aegis_trace_id,
                     uint32_t param1, uint32_t param2) {
    AegisTraceEvent event;
    uint16_t written;

    if (log == NULL || !log->is_initialized) {
        return;  /* 未初始化，忽略 */
    }

    /* 构造事件 */
    event.timestamp = aegis_trace_get_timestamp(log);
    event.event_type = type;
    event.aegis_trace_id = aegis_trace_id;
    event.param1 = param1;
    event.param2 = param2;

    /* 写入环形缓冲区（如果满了会覆盖旧数据） */
    ENTER_CRITICAL();
    written = aegis_ring_buffer_write(&log->ring, (const uint8_t*)&event, sizeof(AegisTraceEvent));
    EXIT_CRITICAL();

    /* 如果缓冲区满了，丢弃最早的事件 */
    if (written < sizeof(AegisTraceEvent)) {
        /* 缓冲区满，先读出一个旧事件腾出空间 */
        AegisTraceEvent dummy;
        ENTER_CRITICAL();
        aegis_ring_buffer_read(&log->ring, (uint8_t*)&dummy, sizeof(AegisTraceEvent));
        /* 再次尝试写入 */
        aegis_ring_buffer_write(&log->ring, (const uint8_t*)&event, sizeof(AegisTraceEvent));
        EXIT_CRITICAL();
    }
}

uint8_t aegis_trace_log_get_count(const AegisTraceLog* log) {
    uint16_t count;
    uint8_t event_count;

    if (log == NULL || !log->is_initialized) {
        return 0U;
    }

    count = aegis_ring_buffer_get_count(&log->ring);
    event_count = (uint8_t)(count / sizeof(AegisTraceEvent));

    return event_count;
}

const AegisTraceEvent* aegis_trace_log_get_event(const AegisTraceLog* log, uint8_t index) {
    /* 注意：ring_buffer不支持随机访问，这里返回NULL
     * 实际使用中应该提供迭代器或改用其他方式访问历史事件 */
    (void)log;
    (void)index;
    return NULL;
}
