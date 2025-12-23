/*
 * event_driven - C89 DDD+CQRS 框架事件驱动示例（通用化版本）
 *
 * 功能：
 *   - 演示领域事件总线（同步+异步订阅）
 *   - 使用通用实体生命周期事件（created/updated）
 *   - 事件历史与统计
 *
 * 注意：
 *   - 为了保持框架“去业务化”，示例在本文件中定义业务payload与实体类型ID。
 */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "domain_event.h"
#include "domain_repository_write.h"
#include "entry_platform.h"
#include "hal_timer.h"
#include "compile_time.h"

static uint32_t now_ms(void* ctx) {
    (void)ctx;
    return aegis_hal_timer_get_tick_ms();
}

typedef struct {
    uint16_t sensor_type;
    int32_t value;
} SensorPayload;

#define ENTITY_TYPE_SENSOR ((AegisEntityType)1U)

typedef struct {
    uint8_t entity_created_count;
    uint8_t entity_updated_count;
    uint8_t all_events_count;
} EventDrivenStats;

static AegisEventHandlerResult on_entity_created(const AegisDomainEvent* event, void* ctx) {
    EventDrivenStats* stats;
    if (ctx == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventDrivenStats*)ctx;
    stats->entity_created_count++;

    printf("[同步事件] ENTITY_CREATED: ID=%u, 类型=%u, 时间戳=%u\n",
           event->aggregate_id,
           event->data.entity_created.entity_type,
           event->timestamp);
    printf("           追溯编号=%s\n", event->aegis_trace_id);

    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_entity_updated(const AegisDomainEvent* event, void* ctx) {
    EventDrivenStats* stats;
    if (ctx == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventDrivenStats*)ctx;
    stats->entity_updated_count++;

    printf("[异步事件] ENTITY_UPDATED: ID=%u, 时间戳=%u\n",
           event->aggregate_id, event->timestamp);

    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_all_events(const AegisDomainEvent* event, void* ctx) {
    EventDrivenStats* stats;
    if (ctx == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventDrivenStats*)ctx;
    stats->all_events_count++;
    printf("[全局监听] 事件ID=%u, 类型=%u, 聚合根=%u\n",
           event->event_id, event->type, event->aggregate_id);
    return EVENT_HANDLER_OK;
}

static void print_event_history(const AegisDomainEventBus* bus) {
    const AegisDomainEvent* event;
    uint8_t i;
    uint8_t count;

    printf("\n[事件历史] 最近 16 条记录:\n");
    printf("-----------------------------------------\n");

    count = 0;
    for (i = 0; i < 16; i++) {
        event = aegis_domain_event_get_history(bus, i);
        if (event == NULL || event->event_id == 0) {
            break;
        }
        count++;
        printf("%2u. 事件ID=%u, 类型=%u, 聚合根=%u, 时间戳=%u\n",
               count,
               event->event_id,
               event->type,
               event->aggregate_id,
               event->timestamp);
    }

    if (count == 0) {
        printf("（无记录）\n");
    }

    printf("-----------------------------------------\n\n");
}

static void print_event_statistics(const EventDrivenStats* stats) {
    printf("\n[事件统计]\n");
    printf("-----------------------------------------\n");
    if (stats == NULL) {
        printf("（无统计）\n");
    } else {
        printf("ENTITY_CREATED:  %u 次\n", stats->entity_created_count);
        printf("ENTITY_UPDATED:  %u 次\n", stats->entity_updated_count);
        printf("全局监听:        %u 次\n", stats->all_events_count);
    }
    printf("-----------------------------------------\n\n");
}

static AegisErrorCode publish_created_event(AegisDomainEventBus* bus,
                                       const AegisDomainEntity* entity,
                                       const char* aegis_trace_id) {
    AegisDomainEvent event;

    if (bus == NULL || entity == NULL) {
        return ERR_NULL_PTR;
    }

    memset(&event, 0, sizeof(AegisDomainEvent));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = entity->base.id;
    event.aegis_trace_id = aegis_trace_id;
    event.data.entity_created.entity_type = entity->base.type;

    return aegis_domain_event_publish(bus, &event);
}

static AegisErrorCode publish_updated_event(AegisDomainEventBus* bus,
                                       AegisEntityId aggregate_id,
                                       const char* aegis_trace_id) {
    AegisDomainEvent event;

    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    memset(&event, 0, sizeof(AegisDomainEvent));
    event.type = DOMAIN_EVENT_ENTITY_UPDATED;
    event.aggregate_id = aggregate_id;
    event.aegis_trace_id = aegis_trace_id;

    return aegis_domain_event_publish(bus, &event);
}

int main(void) {
    AegisErrorCode ret;
    uint8_t processed;
    const AegisDomainRepositoryWriteInterface* write_repo;
    AegisTraceLog trace;
    AegisDomainEventBus bus;
    AegisDomainEntity sensor;
    EventDrivenStats stats;
    AegisEventSubscription subscriptions[3];

    printf("======================================\n");
    printf("  C89 DDD+CQRS 框架 - 事件驱动示例\n");
    printf("======================================\n\n");

    printf("[步骤1] 初始化追溯日志...\n");
    ret = aegis_trace_log_init(&trace, now_ms, NULL);
    if (ret != ERR_OK) {
        printf("错误: Trace初始化失败, 错误码=%d\n", ret);
        return -1;
    }
    printf("✓ Trace初始化成功\n\n");

    printf("[步骤2] 初始化事件总线...\n");
    memset(&stats, 0, sizeof(EventDrivenStats));

    subscriptions[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subscriptions[0].handler = on_entity_created;
    subscriptions[0].ctx = &stats;
    subscriptions[0].is_sync = TRUE;
    subscriptions[0].priority = 0U;

    subscriptions[1].event_type = DOMAIN_EVENT_ENTITY_UPDATED;
    subscriptions[1].handler = on_entity_updated;
    subscriptions[1].ctx = &stats;
    subscriptions[1].is_sync = FALSE;
    subscriptions[1].priority = 1U;

    subscriptions[2].event_type = 0;
    subscriptions[2].handler = on_all_events;
    subscriptions[2].ctx = &stats;
    subscriptions[2].is_sync = FALSE;
    subscriptions[2].priority = 10U;

    ret = aegis_domain_event_bus_init(&bus, &trace, subscriptions, (uint8_t)FW_ARRAY_SIZE(subscriptions));
    if (ret != ERR_OK) {
        printf("错误: 事件总线初始化失败, 错误码=%d\n", ret);
        return -1;
    }
    printf("✓ 事件总线初始化成功, 订阅数=%u\n\n", (unsigned)FW_ARRAY_SIZE(subscriptions));

    printf("[步骤3] 初始化仓储...\n");
    ret = aegis_entry_platform_init(now_ms, NULL);
    if (ret != ERR_OK) {
        printf("错误: 平台依赖初始化失败, 错误码=%d\n", ret);
        return -1;
    }

    write_repo = aegis_entry_platform_get_write_repo();
    if (write_repo == NULL) {
        printf("错误: 获取写仓储接口失败\n");
        return -1;
    }

    ret = write_repo->init(write_repo);
    if (ret != ERR_OK) {
        printf("错误: 仓储初始化失败, 错误码=%d\n", ret);
        return -1;
    }
    printf("✓ 仓储初始化成功\n\n");

    printf("[步骤4] 创建一个实体（触发同步事件）...\n");
    memset(&sensor, 0, sizeof(AegisDomainEntity));
    (void)aegis_domain_entity_init(&sensor.base, ENTITY_ID_INVALID, ENTITY_TYPE_SENSOR);
    {
        SensorPayload p;
        p.sensor_type = 0U;
        p.value = 10;
        (void)aegis_domain_entity_payload_set(&sensor, &p, (uint16_t)sizeof(SensorPayload));
    }
    ret = write_repo->create(write_repo, &sensor);
    if (ret != ERR_OK) {
        printf("错误: 创建实体失败, 错误码=%d\n", ret);
        return -1;
    }
    (void)publish_created_event(&bus, &sensor, "REQ-EXAMPLE-002-CREATE");
    printf("✓ 创建成功, ID=%u\n\n", sensor.base.id);

    printf("[步骤5] 更新实体（触发异步事件）...\n");
    {
        SensorPayload p2;
        p2.sensor_type = 0U;
        p2.value = 99;
        (void)aegis_domain_entity_payload_set(&sensor, &p2, (uint16_t)sizeof(SensorPayload));
    }
    ret = write_repo->update(write_repo, &sensor);
    if (ret != ERR_OK) {
        printf("错误: 更新实体失败, 错误码=%d\n", ret);
        return -1;
    }
    (void)publish_updated_event(&bus, sensor.base.id, "REQ-EXAMPLE-002-UPDATE");
    printf("✓ 更新成功\n\n");

    printf("[步骤6] 处理异步事件队列...\n");
    processed = aegis_domain_event_process(&bus, 10);
    printf("✓ 已处理 %u 个异步事件\n\n", processed);

    printf("[步骤7] 查询事件历史...\n");
    print_event_history(&bus);

    printf("[步骤8] 打印统计信息...\n");
    print_event_statistics(&stats);

    printf("======================================\n");
    printf("  事件驱动示例执行完成！\n");
    printf("======================================\n");

    return 0;
}
