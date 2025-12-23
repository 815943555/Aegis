/*
 * @file: trace.h
 * @brief: ISO 26262 追溯信息和运行时日志
 * @author: jack liu
 * @req: REQ-COMMON-003
 * @design: DES-COMMON-003
 * @asil: ASIL-B
 */

#ifndef TRACE_H
#define TRACE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 追溯标记宏 ==================== */
/*
 * 这些宏在代码中作为注释形式存在，
 * 由 check_traceability.py 脚本解析验证
 */
#define TRACE_REQ(id)       /* @req: id */
#define TRACE_DESIGN(id)    /* @design: id */
#define TRACE_ASIL(level)   /* @asil: level */

/* ==================== 运行时追溯日志 ==================== */
/* 追溯事件类型 */
typedef enum {
    TRACE_EVENT_SYSTEM_INIT = 0,    /* 系统初始化 */
    TRACE_EVENT_CMD_ENQUEUE = 1,    /* 命令入队 */
    TRACE_EVENT_CMD_EXEC    = 2,    /* 命令执行 */
    TRACE_EVENT_MEM_ALLOC   = 3,    /* 内存分配 */
    TRACE_EVENT_MEM_FREE    = 4,    /* 内存释放 */
    TRACE_EVENT_HAL_ERROR   = 5,    /* HAL 错误 */
    TRACE_EVENT_DOMAIN_ERR  = 6,    /* 领域错误 */
    TRACE_EVENT_APP_ERROR   = 7,    /* 应用错误 */
    TRACE_EVENT_SYSTEM_ERROR= 8,    /* 系统错误 */
    TRACE_EVENT_MAX         = 255
} AegisTraceEventType;

/* 追溯事件结构 */
typedef struct {
    uint32_t timestamp;         /* 系统滴答时间戳 */
    AegisTraceEventType event_type;  /* 事件类型 */
    const char* aegis_trace_id;       /* 追溯编号 (如 "REQ-001") */
    uint32_t param1;            /* 上下文参数1 */
    uint32_t param2;            /* 上下文参数2 */
} AegisTraceEvent;

/* ==================== 追溯日志配置 ==================== */
#ifndef TRACE_LOG_SIZE
#define TRACE_LOG_SIZE  32
#endif

/* ==================== 追溯日志接口 ==================== */
/*
 * @brief: 初始化追溯日志
 * @req: REQ-TRACE-001
 * @design: DES-TRACE-001
 * @asil: ASIL-B
 * @isr_unsafe
 */
void aegis_trace_log_init(void);

/*
 * @brief: 记录追溯事件
 * @param type: 事件类型
 * @param aegis_trace_id: 追溯编号
 * @param param1: 参数1
 * @param param2: 参数2
 * @req: REQ-TRACE-002
 * @design: DES-TRACE-002
 * @asil: ASIL-B
 * @isr_safe
 */
void aegis_trace_log_event(AegisTraceEventType type, const char* aegis_trace_id,
                     uint32_t param1, uint32_t param2);

/*
 * @brief: 获取日志事件数量
 * @return: 当前日志中的事件数量
 * @req: REQ-TRACE-003
 * @design: DES-TRACE-003
 * @asil: ASIL-B
 * @isr_unsafe
 */
uint8_t aegis_trace_log_get_count(void);

/*
 * @brief: 获取指定索引的事件
 * @param index: 事件索引
 * @return: 事件指针，失败返回 NULL
 * @req: REQ-TRACE-004
 * @design: DES-TRACE-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
const AegisTraceEvent* aegis_trace_log_get_event(uint8_t index);

/*
 * @brief: 获取系统时间戳（由平台实现）
 * @return: 系统滴答时间戳
 * @req: REQ-TRACE-005
 * @design: DES-TRACE-005
 * @asil: ASIL-B
 * @isr_safe
 */
uint32_t aegis_trace_get_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif /* TRACE_H */
