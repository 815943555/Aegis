/*
 * @file: aegis_domain_event.h
 * @brief: 领域事件总线（支持同步和异步事件处理）
 * @author: jack liu
 * @req: REQ-EVENT-001
 * @design: DES-EVENT-001
 * @asil: ASIL-B
 */

#ifndef DOMAIN_EVENT_H
#define DOMAIN_EVENT_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 事件配置 ==================== */
#ifndef DOMAIN_EVENT_QUEUE_SIZE
#define DOMAIN_EVENT_QUEUE_SIZE     32      /* 异步事件队列大小 */
#endif

#ifndef DOMAIN_EVENT_HISTORY_SIZE
#define DOMAIN_EVENT_HISTORY_SIZE   16      /* 事件历史记录大小 */
#endif

#ifndef MAX_EVENT_SUBSCRIPTIONS
#define MAX_EVENT_SUBSCRIPTIONS     16      /* 最大订阅数量 */
#endif

#ifndef MAX_EVENT_RECURSION_DEPTH
#define MAX_EVENT_RECURSION_DEPTH   3       /* 最大事件递归深度（防止死循环） */
#endif

/* ==================== 事件ID和类型 ==================== */
typedef uint16_t AegisDomainEventId;

/* 事件类型枚举 */
typedef enum {
    DOMAIN_EVENT_NONE = 0,

    /* 实体生命周期事件 (1-99) */
    DOMAIN_EVENT_ENTITY_CREATED     = 1,
    DOMAIN_EVENT_ENTITY_UPDATED     = 2,
    DOMAIN_EVENT_ENTITY_DELETED     = 3,

    /* 传感器事件 (100-199) */
    DOMAIN_EVENT_SENSOR_CREATED         = 100,
    DOMAIN_EVENT_SENSOR_VALUE_CHANGED   = 101,
    DOMAIN_EVENT_SENSOR_STATE_CHANGED   = 102,

    /* 执行器事件 (200-299) */
    DOMAIN_EVENT_ACTUATOR_CREATED           = 200,
    DOMAIN_EVENT_ACTUATOR_COMMAND_CHANGED   = 201,
    DOMAIN_EVENT_ACTUATOR_STATE_CHANGED     = 202,

    /* 用户自定义事件 (1000+) */
    DOMAIN_EVENT_USER_BASE  = 1000,
    DOMAIN_EVENT_MAX        = 65535
} AegisDomainEventType;

/* ==================== 事件数据结构 ==================== */
/* 事件基础结构 */
typedef struct {
    AegisDomainEventId event_id;         /* 事件唯一ID（自增） */
    AegisDomainEventType type;           /* 事件类型 */
    AegisEntityId aggregate_id;          /* 聚合根ID */
    uint32_t timestamp;             /* 时间戳 */
    const char* aegis_trace_id;           /* 追溯编号 */

    /* 事件数据（联合体节省空间） */
    union {
        /* 实体创建事件 */
        struct {
            AegisEntityType entity_type;
        } entity_created;

        /* 传感器值变化事件 */
        struct {
            int32_t old_value;
            int32_t new_value;
        } sensor_value_changed;

        /* 传感器状态变化事件 */
        struct {
            AegisEntityState old_state;
            AegisEntityState new_state;
        } sensor_state_changed;

        /* 执行器命令变化事件 */
        struct {
            uint8_t old_command;
            uint8_t new_command;
            uint8_t power_level;
        } actuator_command_changed;

        /* 执行器状态变化事件 */
        struct {
            AegisEntityState old_state;
            AegisEntityState new_state;
        } actuator_state_changed;

        /* 自定义事件数据（最大64字节） */
        uint8_t custom_data[64];
    } data;
} AegisDomainEvent;

/* ==================== 事件处理器 ==================== */
/* 事件处理结果 */
typedef enum {
    EVENT_HANDLER_OK        = 0,    /* 处理成功 */
    EVENT_HANDLER_ERROR     = 1,    /* 处理失败 */
    EVENT_HANDLER_RETRY     = 2     /* 需要重试 */
} AegisEventHandlerResult;

/* 事件处理器回调函数类型 */
typedef AegisEventHandlerResult (*DomainEventHandler)(const AegisDomainEvent* event);

/* 事件订阅配置 */
typedef struct {
    AegisDomainEventType event_type;     /* 订阅的事件类型（0表示订阅所有事件） */
    DomainEventHandler handler;     /* 处理函数 */
    bool_t is_sync;                 /* TRUE=同步处理，FALSE=异步处理 */
    uint8_t priority;               /* 优先级（0-255，数字越小优先级越高） */
} AegisEventSubscription;

/* ==================== 事件总线接口 ==================== */
/*
 * @brief: 初始化领域事件总线
 * @param subscriptions: 事件订阅表（静态数组）
 * @param count: 订阅数量
 * @return: 错误码
 * @req: REQ-EVENT-001
 * @design: DES-EVENT-001
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_event_bus_init(const AegisEventSubscription* subscriptions, uint8_t count);

/*
 * @brief: 发布领域事件（ISR安全）
 * @param event: 事件指针
 * @return: 错误码
 * @note: 同步订阅者会立即执行，异步订阅者会将事件入队
 * @req: REQ-EVENT-002
 * @design: DES-EVENT-002
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_event_publish(const AegisDomainEvent* event);

/*
 * @brief: 处理异步事件队列（主循环调用）
 * @param max_events: 本次最多处理的事件数量（0=处理所有）
 * @return: 实际处理的事件数量
 * @req: REQ-EVENT-003
 * @design: DES-EVENT-003
 * @asil: ASIL-B
 * @isr_unsafe
 */
uint8_t aegis_domain_event_process(uint8_t max_events);

/*
 * @brief: 获取事件队列状态
 * @param pending_count: 输出待处理事件数量
 * @param processed_count: 输出已处理事件总数
 * @return: 错误码
 * @req: REQ-EVENT-004
 * @design: DES-EVENT-004
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_event_get_stats(uint8_t* pending_count, uint32_t* processed_count);

/*
 * @brief: 获取事件历史记录（用于事件溯源）
 * @param index: 事件索引（0=最新，1=次新，...）
 * @return: 事件指针，失败返回 NULL
 * @req: REQ-EVENT-005
 * @design: DES-EVENT-005
 * @asil: ASIL-B
 * @isr_unsafe
 */
const AegisDomainEvent* aegis_domain_event_get_history(uint8_t index);

/*
 * @brief: 清空事件队列（用于异常恢复）
 * @return: 错误码
 * @req: REQ-EVENT-006
 * @design: DES-EVENT-006
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_event_clear_queue(void);

/*
 * @brief: 获取当前递归深度（用于调试）
 * @return: 当前事件处理递归深度
 * @req: REQ-EVENT-007
 * @design: DES-EVENT-007
 * @asil: ASIL-B
 * @isr_safe
 */
uint8_t aegis_domain_event_get_recursion_depth(void);

/* ==================== 便捷宏 ==================== */
/* 创建事件的便捷宏（自动填充时间戳） */
#define DOMAIN_EVENT_INIT(event_type_val, aggregate_id_val, aegis_trace_id_val) \
    { \
        .event_id = 0, \
        .type = (event_type_val), \
        .aggregate_id = (aggregate_id_val), \
        .timestamp = 0, \
        .aegis_trace_id = (aegis_trace_id_val), \
        .data = {0} \
    }

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_EVENT_H */
