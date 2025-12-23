/*
 * minimal_app - DDD依赖正确的最小示例：充电桩功率控制
 *
 * 依赖关系：
 * - entry(本文件) 作为组合根：装配 infrastructure + framework runtime
 * - application(demo_application.*)：注册命令/查询处理器，编排用例
 * - domain(demo_domain.*)：领域规则与领域事件（power_level 属于业务字段）
 * - infrastructure：仓储实现由 Entry 注入（inmem repo）
 */

#include <stdio.h>
#include <string.h>
#include "entry_init.h"
#include "entry_main.h"
#include "app_command.h"
#include "app_query.h"
#include "app_module.h"
#include "app_macros.h"
#include "demo_application.h"
#include "entry_platform.h"
#include "hal_timer.h"
#include "compile_time.h"

static uint32_t now_ms(void* ctx) {
    (void)ctx;
    return aegis_hal_timer_get_tick_ms();
}

typedef struct {
    uint32_t power_changed_count;
    DemoPowerChangedEventData last_change;
    AegisEntityId last_created_id;
} DemoEventStats;

static AegisEventHandlerResult on_power_changed(const AegisDomainEvent* event, void* ctx) {
    DemoEventStats* stats;
    DemoPowerChangedEventData payload;

    if (ctx == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }

    stats = (DemoEventStats*)ctx;
    memset(&payload, 0, sizeof(DemoPowerChangedEventData));
    memcpy(&payload, event->data.custom_data, sizeof(DemoPowerChangedEventData));

    stats->power_changed_count++;
    stats->last_change = payload;

    printf("[领域事件] POWER_CHANGED: id=%u old=%u new=%u\n",
           event->aggregate_id, payload.old_power, payload.new_power);

    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_entity_created(const AegisDomainEvent* event, void* ctx) {
    DemoEventStats* stats;

    if (ctx == NULL || event == NULL) {
        return EVENT_HANDLER_ERROR;
    }

    stats = (DemoEventStats*)ctx;
    if (event->data.entity_created.entity_type == DEMO_ENTITY_TYPE_CHARGER) {
        stats->last_created_id = event->aggregate_id;
        printf("[领域事件] ENTITY_CREATED: id=%u type=%u\n",
               event->aggregate_id, event->data.entity_created.entity_type);
    }

    return EVENT_HANDLER_OK;
}

int main(void) {
    AegisEntryRuntime runtime;
    AegisEntryConfig cfg;
    const AegisDomainRepositoryWriteInterface* write_repo;
    DemoEventStats stats;
    DemoApplicationModule demo_module;
    AegisAppModule modules[1];
    AegisEventSubscription subs[2];
    AegisErrorCode ret;
    AegisCommand cmd;
    AegisCommandResult result;
    AegisEntityId charger_id;
    AegisQueryRequest q;
    AegisQueryResponse qr;
    DemoChargerDto dto;

    printf("======================================\n");
    printf("  minimal_app - 充电桩功率控制（DDD依赖正确）\n");
    printf("======================================\n\n");

    memset(&stats, 0, sizeof(DemoEventStats));

    /* 事件订阅表：通过ctx注入统计对象（避免全局变量） */
    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = on_entity_created;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    subs[1].event_type = DEMO_EVENT_POWER_LEVEL_CHANGED;
    subs[1].handler = on_power_changed;
    subs[1].ctx = &stats;
    subs[1].is_sync = TRUE;
    subs[1].priority = 1U;

    ret = aegis_entry_platform_init(now_ms, NULL);
    if (ret != ERR_OK) {
        printf("错误: 平台依赖初始化失败, 错误码=%d\n", ret);
        return (int)ret;
    }
    write_repo = aegis_entry_platform_get_write_repo();
    if (write_repo == NULL) {
        printf("错误: 获取写仓储接口失败\n");
        return (int)ERR_NULL_PTR;
    }

    cfg.aegis_trace_now_fn = now_ms;
    cfg.aegis_trace_now_ctx = NULL;
    cfg.write_repo = write_repo;
    cfg.event_subscriptions = subs;
    cfg.event_subscription_count = (uint8_t)FW_ARRAY_SIZE(subs);

    ret = aegis_entry_init_all(&runtime, &cfg);
    if (ret != ERR_OK) {
        printf("错误: aegis_entry_init_all 失败, 错误码=%d\n", ret);
        return (int)ret;
    }

    memset(&demo_module, 0, sizeof(DemoApplicationModule));
    APP_MODULE_SET(&modules[0], demo_application_register, &demo_module);

    APP_REGISTER_MODULES(ret, &runtime.app, modules);
    if (ret != ERR_OK) {
        printf("错误: 注册用例处理器失败, 错误码=%d\n", ret);
        return (int)ret;
    }

    /* 1) 发起创建命令：CreateCharger(model=1001, initial_power=10) */
    memset(&cmd, 0, sizeof(AegisCommand));
    APP_CMD_INIT(&cmd, DEMO_CMD_CREATE_CHARGER);
    {
        DemoCreateChargerCmd p;
        p.charger_model = 1001U;
        p.initial_power_level = 10U;
        APP_CMD_PAYLOAD_SET(ret, &cmd, &p);
        if (ret != ERR_OK) {
            printf("错误: 设置命令payload失败, 错误码=%d\n", ret);
            return (int)ret;
        }
    }

    ret = aegis_app_cmd_enqueue(&runtime.app.cmd_queue, &cmd);
    if (ret != ERR_OK) {
        printf("错误: 命令入队失败, 错误码=%d\n", ret);
        return (int)ret;
    }

    (void)aegis_entry_main_loop_once(&runtime);
    (void)result;

    charger_id = stats.last_created_id;
    if (charger_id == ENTITY_ID_INVALID) {
        printf("错误: 未获取到创建的charger_id（ENTITY_CREATED未触发）\n");
        return -1;
    }

    /* 2) 发起设置功率命令：SetPowerLevel(id, new_power=55) */
    memset(&cmd, 0, sizeof(AegisCommand));
    APP_CMD_INIT(&cmd, DEMO_CMD_SET_POWER_LEVEL);
    APP_CMD_SET_ENTITY_ID(&cmd, charger_id);
    {
        DemoSetPowerCmd p2;
        p2.new_power_level = 55U;
        APP_CMD_PAYLOAD_SET(ret, &cmd, &p2);
        if (ret != ERR_OK) {
            printf("错误: 设置命令payload失败, 错误码=%d\n", ret);
            return (int)ret;
        }
    }

    ret = aegis_app_cmd_enqueue(&runtime.app.cmd_queue, &cmd);
    if (ret != ERR_OK) {
        printf("错误: 命令入队失败, 错误码=%d\n", ret);
        return (int)ret;
    }

    (void)aegis_entry_main_loop_once(&runtime);

    /* 3) 发起查询：GetCharger(id) */
    memset(&q, 0, sizeof(AegisQueryRequest));
    q.type = DEMO_QUERY_GET_CHARGER;
    q.entity_id = charger_id;
    q.payload_size = 0U;

    memset(&qr, 0, sizeof(AegisQueryResponse));
    ret = aegis_app_query_execute(&runtime.app.query, &q, &qr);
    if (ret != ERR_OK || qr.result != ERR_OK) {
        printf("错误: 查询失败, ret=%d result=%d payload_size=%u\n", ret, qr.result, qr.payload_size);
        return -1;
    }

    memset(&dto, 0, sizeof(DemoChargerDto));
    APP_QUERY_RESULT_PAYLOAD_GET(ret, &qr, &dto);
    if (ret != ERR_OK) {
        printf("错误: 读取查询结果payload失败, 错误码=%d payload_size=%u\n", ret, qr.payload_size);
        return -1;
    }

    printf("\n[查询结果] id=%u model=%u power=%u\n",
           dto.id, dto.charger_model, dto.power_level);

    printf("\n[事件统计] power_changed_count=%lu last(old=%u,new=%u)\n",
           (unsigned long)stats.power_changed_count,
           stats.last_change.old_power, stats.last_change.new_power);

    return 0;
}
