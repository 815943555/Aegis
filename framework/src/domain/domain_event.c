/*
 * @file: aegis_domain_event.c
 * @brief: 领域事件总线实现（支持同步和异步事件处理）
 * @author: jack liu
 * @req: REQ-EVENT-001
 * @design: DES-EVENT-001
 * @asil: ASIL-B
 */

#include "domain_event.h"
#include "critical.h"
#include "trace.h"
#include <string.h>

/* ==================== 内部辅助函数 ==================== */
/*
 * @brief: 异步事件队列入队（使用环形缓冲区）
 * @param event: 事件指针
 * @return: 错误码
 */
static AegisErrorCode event_queue_enqueue(AegisDomainEventBus* bus, const AegisDomainEvent* event)
{
    uint16_t written;
    const uint8_t* event_bytes;

    if (bus == NULL || event == NULL) {
        return ERR_NULL_PTR;
    }

    /* 检查环形缓冲区剩余空间是否足够存储一个事件 */
    if (aegis_ring_buffer_get_free(&bus->async_queue) < sizeof(AegisDomainEvent)) {
        bus->dropped_events++;
        return ERR_CMD_QUEUE_FULL;  /* 复用命令队列满错误码 */
    }

    /* 将事件结构体作为字节流写入环形缓冲区 */
    event_bytes = (const uint8_t*)event;
    written = aegis_ring_buffer_write(&bus->async_queue, event_bytes, sizeof(AegisDomainEvent));

    if (written != sizeof(AegisDomainEvent)) {
        bus->dropped_events++;
        return ERR_CMD_QUEUE_FULL;
    }

    return ERR_OK;
}

/*
 * @brief: 异步事件队列出队（使用环形缓冲区）
 * @param event: 输出事件指针
 * @return: 错误码
 */
static AegisErrorCode event_queue_dequeue(AegisDomainEventBus* bus, AegisDomainEvent* event)
{
    uint16_t read_count;
    uint8_t* event_bytes;

    if (bus == NULL || event == NULL) {
        return ERR_NULL_PTR;
    }

    /* 检查环形缓冲区是否有足够数据 */
    if (aegis_ring_buffer_get_count(&bus->async_queue) < sizeof(AegisDomainEvent)) {
        return ERR_EMPTY;
    }

    /* 从环形缓冲区读取一个事件 */
    event_bytes = (uint8_t*)event;
    read_count = aegis_ring_buffer_read(&bus->async_queue, event_bytes, sizeof(AegisDomainEvent));

    if (read_count != sizeof(AegisDomainEvent)) {
        return ERR_EMPTY;
    }

    return ERR_OK;
}

/*
 * @brief: 添加事件到历史记录
 * @param event: 事件指针
 */
static void event_history_add(AegisDomainEventBus* bus, const AegisDomainEvent* event)
{
    AegisDomainEventHistory* history;

    if (bus == NULL || event == NULL) {
        return;
    }

    history = &bus->history;

    /* 拷贝事件到历史记录（环形缓冲区） */
    memcpy(&history->history[history->head], event, sizeof(AegisDomainEvent));

    /* 更新历史记录状态 */
    history->head = (uint8_t)((history->head + 1) % DOMAIN_EVENT_HISTORY_SIZE);
    if (history->count < DOMAIN_EVENT_HISTORY_SIZE) {
        history->count++;
    }
}

/*
 * @brief: 调用事件处理器
 * @param subscription: 订阅配置
 * @param event: 事件指针
 * @return: 处理结果
 */
static AegisEventHandlerResult invoke_handler(const AegisEventSubscription* subscription,
                                         const AegisDomainEvent* event)
{
    if (subscription == NULL || subscription->handler == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }

    /* 检查递归深度 */
    /* 递归深度由调用方总线控制，invoke_handler假定外层已处理 */
    return subscription->handler(event, subscription->ctx);
}

static AegisEventHandlerResult invoke_handler_with_recursion(AegisDomainEventBus* bus,
                                                        const AegisEventSubscription* subscription,
                                                        const AegisDomainEvent* event)
{
    AegisEventHandlerResult result;

    if (bus == NULL) {
        return EVENT_HANDLER_ERROR;
    }

    if (subscription == NULL || subscription->handler == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }

    if (bus->recursion_depth >= MAX_EVENT_RECURSION_DEPTH) {
        if (bus->trace != NULL) {
            aegis_trace_log_event(bus->trace, TRACE_EVENT_DOMAIN_ERR, "REQ-EVENT-008",
                           (uint32_t)event->type, bus->recursion_depth);
        }
        return EVENT_HANDLER_ERROR;
    }

    /* 增加递归深度 */
    bus->recursion_depth++;

    /* 调用处理器 */
    result = invoke_handler(subscription, event);

    /* 恢复递归深度 */
    bus->recursion_depth--;

    return result;
}

/*
 * @brief: 按优先级排序订阅者并分发事件
 * @param event: 事件指针
 * @param is_sync: TRUE=处理同步订阅者，FALSE=处理异步订阅者
 * @return: 处理的订阅者数量
 */
static uint8_t dispatch_to_subscribers(AegisDomainEventBus* bus, const AegisDomainEvent* event, bool_t is_sync)
{
    uint8_t i;
    uint8_t handled_count;
    const AegisEventSubscription* sub;
    AegisEventHandlerResult result;

    handled_count = 0;

    if (bus == NULL || event == NULL) {
        return 0;
    }

    /* 遍历所有订阅 */
    for (i = 0; i < bus->subscription_count; i++) {
        sub = &bus->subscriptions[i];

        /* 检查订阅类型是否匹配（同步/异步） */
        if (sub->is_sync != is_sync) {
            continue;
        }

        /* 检查事件类型是否匹配（0表示订阅所有事件） */
        if (sub->event_type != 0 && sub->event_type != event->type) {
            continue;
        }

        /* 调用处理器 */
        result = invoke_handler_with_recursion(bus, sub, event);

        if (result == EVENT_HANDLER_OK) {
            handled_count++;
        }

        /* 记录处理失败 */
        if (result == EVENT_HANDLER_ERROR) {
            if (bus->trace != NULL) {
                aegis_trace_log_event(bus->trace, TRACE_EVENT_APP_ERROR, "REQ-EVENT-009",
                               (uint32_t)event->type, i);
            }
        }
    }

    return handled_count;
}

/* ==================== 公共接口实现 ==================== */
/*
 * @brief: 初始化领域事件总线
 */
AegisErrorCode aegis_domain_event_bus_init(AegisDomainEventBus* bus,
                                AegisTraceLog* trace,
                                const AegisEventSubscription* subscriptions,
                                uint8_t count)
{
    AegisErrorCode ret;

    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    if (subscriptions == NULL && count > 0) {
        return ERR_NULL_PTR;
    }

    if (count > MAX_EVENT_SUBSCRIPTIONS) {
        return ERR_OUT_OF_RANGE;
    }

    /* 清空事件总线状态 */
    memset(bus, 0, sizeof(AegisDomainEventBus));

    /* 初始化异步事件队列的环形缓冲区 */
    ret = aegis_ring_buffer_init(&bus->async_queue,
                           bus->async_queue_buffer,
                           (uint16_t)sizeof(bus->async_queue_buffer));
    if (ret != ERR_OK) {
        return ret;
    }

    /* 设置订阅表 */
    bus->subscriptions = subscriptions;
    bus->subscription_count = count;

    /* 初始化事件ID */
    bus->next_event_id = 1;

    /* 标记已初始化 */
    bus->is_initialized = TRUE;
    bus->trace = trace;

    /* 记录追溯日志 */
    if (bus->trace != NULL) {
        aegis_trace_log_event(bus->trace, TRACE_EVENT_SYSTEM_INIT, "REQ-EVENT-001",
                       (uint32_t)count, 0);
    }

    return ERR_OK;
}

/*
 * @brief: 发布领域事件（ISR安全）
 */
AegisErrorCode aegis_domain_event_publish(AegisDomainEventBus* bus, const AegisDomainEvent* event)
{
    AegisDomainEvent event_copy;
    uint8_t sync_count;
    AegisErrorCode err;

    if (bus == NULL || event == NULL) {
        return ERR_NULL_PTR;
    }

    if (!bus->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    /* 进入临界区 */
    ENTER_CRITICAL();

    /* 拷贝事件并分配事件ID */
    memcpy(&event_copy, event, sizeof(AegisDomainEvent));
    event_copy.event_id = bus->next_event_id;
    bus->next_event_id++;

    /* 如果时间戳为0，自动填充 */
    if (event_copy.timestamp == 0) {
        if (bus->trace != NULL) {
            event_copy.timestamp = aegis_trace_get_timestamp(bus->trace);
        }
    }

    /* 添加到历史记录 */
    event_history_add(bus, &event_copy);

    /* 更新统计 */
    bus->total_published++;

    /* 退出临界区 */
    EXIT_CRITICAL();

    /* 1. 同步分发（不在临界区内执行，避免长时间占用） */
    sync_count = dispatch_to_subscribers(bus, &event_copy, TRUE);
    bus->sync_handled += sync_count;

    /* 2. 异步订阅者：将事件入队 */
    ENTER_CRITICAL();
    err = event_queue_enqueue(bus, &event_copy);
    EXIT_CRITICAL();

    if (err != ERR_OK) {
        /* 队列满，记录错误但不影响同步分发 */
        if (bus->trace != NULL) {
            aegis_trace_log_event(bus->trace, TRACE_EVENT_DOMAIN_ERR, "REQ-EVENT-010",
                           (uint32_t)event_copy.type, bus->async_queue.count);
        }
    }

    return ERR_OK;
}

/*
 * @brief: 处理异步事件队列（主循环调用）
 */
uint8_t aegis_domain_event_process(AegisDomainEventBus* bus, uint8_t max_events)
{
    uint8_t processed;
    AegisDomainEvent event;
    AegisErrorCode err;
    uint8_t async_count;

    if (bus == NULL || !bus->is_initialized) {
        return 0;
    }

    processed = 0;

    /* 处理队列中的事件 */
    while (processed < max_events || max_events == 0) {
        /* 从队列取出事件 */
        ENTER_CRITICAL();
        err = event_queue_dequeue(bus, &event);
        EXIT_CRITICAL();

        if (err != ERR_OK) {
            /* 队列为空，结束处理 */
            break;
        }

        /* 分发给异步订阅者 */
        async_count = dispatch_to_subscribers(bus, &event, FALSE);

        /* 更新统计 */
        bus->async_handled += async_count;
        bus->total_processed++;
        processed++;
    }

    return processed;
}

/*
 * @brief: 获取事件队列状态
 */
AegisErrorCode aegis_domain_event_get_stats(const AegisDomainEventBus* bus, uint8_t* pending_count, uint32_t* processed_count)
{
    uint16_t byte_count;
    uint8_t event_count;

    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    if (pending_count == NULL || processed_count == NULL) {
        return ERR_NULL_PTR;
    }

    if (!bus->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    /* 获取环形缓冲区中的字节数，除以事件大小得到事件数量 */
    byte_count = aegis_ring_buffer_get_count(&bus->async_queue);
    event_count = (uint8_t)(byte_count / sizeof(AegisDomainEvent));
    *pending_count = event_count;
    *processed_count = bus->total_processed;
    EXIT_CRITICAL();

    return ERR_OK;
}

/*
 * @brief: 获取事件历史记录
 */
const AegisDomainEvent* aegis_domain_event_get_history(const AegisDomainEventBus* bus, uint8_t index)
{
    AegisDomainEventHistory* history;
    uint8_t actual_index;

    if (bus == NULL || !bus->is_initialized) {
        return NULL;
    }

    history = (AegisDomainEventHistory*)&bus->history;

    /* 检查索引是否有效 */
    if (index >= history->count) {
        return NULL;
    }

    /* 计算实际索引（从最新往前数） */
    if (history->head > index) {
        actual_index = (uint8_t)(history->head - index - 1);
    } else {
        actual_index = (uint8_t)(DOMAIN_EVENT_HISTORY_SIZE + history->head - index - 1);
    }

    return &history->history[actual_index];
}

/*
 * @brief: 清空事件队列
 */
AegisErrorCode aegis_domain_event_clear_queue(AegisDomainEventBus* bus)
{
    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    if (!bus->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    aegis_ring_buffer_clear(&bus->async_queue);

    return ERR_OK;
}

/*
 * @brief: 获取当前递归深度
 */
uint8_t aegis_domain_event_get_recursion_depth(const AegisDomainEventBus* bus)
{
    if (bus == NULL) {
        return 0U;
    }
    return bus->recursion_depth;
}
