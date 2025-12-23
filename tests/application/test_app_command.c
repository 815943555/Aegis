/*
 * @file: test_app_command.c
 * @brief: Application(CQRS) 命令/查询分发测试（严格依赖注入）
 * @author: jack liu
 * @req: REQ-TEST-APP-CQRS
 * @design: DES-TEST-APP-CQRS
 * @asil: ASIL-B
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "app_init.h"
#include "app_command.h"
#include "app_cmd_service.h"
#include "app_query.h"
#include "domain_entity.h"
#include "domain_event.h"
#include "domain_repository_read.h"
#include "domain_repository_write.h"
#include "infrastructure_repository_inmem.h"
#include "trace.h"

#define TEST_ENTITY_TYPE_CHARGER ((AegisEntityType)1U)

#define TEST_CMD_CREATE_CHARGER  ((AegisCommandType)1U)
#define TEST_CMD_SET_POWER       ((AegisCommandType)2U)

#define TEST_QUERY_GET_CHARGER   ((AegisQueryType)1U)

#define TEST_EVENT_POWER_CHANGED ((AegisDomainEventType)(DOMAIN_EVENT_USER_BASE + 1U))

typedef struct {
    uint16_t charger_model;
    uint8_t power_level;
} TestChargerState;

typedef struct {
    uint8_t old_power;
    uint8_t new_power;
} TestPowerChangedPayload;

typedef struct {
    const AegisDomainRepositoryWriteInterface* write_repo;
    AegisDomainEventBus* bus;
} TestCommandCtx;

typedef struct {
    const AegisDomainRepositoryReadInterface* read_repo;
} TestQueryCtx;

typedef struct {
    uint32_t created_count;
    uint32_t power_changed_count;
    AegisEntityId last_created_id;
    TestPowerChangedPayload last_power_change;
} TestEventStats;

static uint32_t test_now_ms(void* ctx) {
    uint32_t* tick;
    if (ctx == NULL) {
        return 0U;
    }
    tick = (uint32_t*)ctx;
    (*tick)++;
    return *tick;
}

static AegisEventHandlerResult on_entity_created(const AegisDomainEvent* event, void* ctx) {
    TestEventStats* stats;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (TestEventStats*)ctx;
    stats->created_count++;
    stats->last_created_id = event->aggregate_id;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_power_changed(const AegisDomainEvent* event, void* ctx) {
    TestEventStats* stats;
    TestPowerChangedPayload payload;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (TestEventStats*)ctx;
    memset(&payload, 0, sizeof(payload));
    memcpy(&payload, event->data.custom_data, sizeof(payload));
    stats->power_changed_count++;
    stats->last_power_change = payload;
    return EVENT_HANDLER_OK;
}

static AegisErrorCode handle_create_charger(const AegisCommand* cmd, AegisCommandResult* result, void* ctx) {
    TestCommandCtx* c;
    AegisDomainEntity entity;
    TestChargerState state;
    AegisDomainEvent ev;
    AegisErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    c = (TestCommandCtx*)ctx;
    if (c->write_repo == NULL || c->bus == NULL) {
        return ERR_NULL_PTR;
    }

    if (cmd->payload_size != (uint16_t)sizeof(TestChargerState)) {
        return ERR_OUT_OF_RANGE;
    }

    memset(&state, 0, sizeof(state));
    memcpy(&state, cmd->payload, sizeof(state));

    memset(&entity, 0, sizeof(entity));
    ret = aegis_domain_entity_init(&entity.base, ENTITY_ID_INVALID, TEST_ENTITY_TYPE_CHARGER);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = aegis_domain_entity_payload_set(&entity, &state, (uint16_t)sizeof(state));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = c->write_repo->create(c->write_repo, &entity);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&ev, 0, sizeof(ev));
    ev.type = DOMAIN_EVENT_ENTITY_CREATED;
    ev.aggregate_id = entity.base.id;
    ev.data.entity_created.entity_type = TEST_ENTITY_TYPE_CHARGER;
    (void)aegis_domain_event_publish(c->bus, &ev);

    memset(result, 0, sizeof(*result));
    result->result = ERR_OK;
    result->created_id = entity.base.id;
    return ERR_OK;
}

static AegisErrorCode handle_set_power(const AegisCommand* cmd, AegisCommandResult* result, void* ctx) {
    TestCommandCtx* c;
    AegisDomainEntity* stored;
    AegisDomainEntity updated;
    const void* payload_ptr;
    uint16_t payload_size;
    TestChargerState old_state;
    TestChargerState new_state;
    TestPowerChangedPayload change;
    AegisDomainEvent ev;
    AegisErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    c = (TestCommandCtx*)ctx;
    if (c->write_repo == NULL || c->bus == NULL) {
        return ERR_NULL_PTR;
    }

    if (cmd->entity_id == ENTITY_ID_INVALID || cmd->payload_size != (uint16_t)sizeof(uint8_t)) {
        return ERR_OUT_OF_RANGE;
    }

    ret = c->write_repo->read.get(&c->write_repo->read, cmd->entity_id, &stored);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = aegis_domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    if (ret != ERR_OK || payload_ptr == NULL || payload_size != (uint16_t)sizeof(TestChargerState)) {
        return ERR_OUT_OF_RANGE;
    }

    memset(&old_state, 0, sizeof(old_state));
    memcpy(&old_state, payload_ptr, sizeof(old_state));

    new_state = old_state;
    new_state.power_level = cmd->payload[0];

    updated = *stored;
    ret = aegis_domain_entity_payload_set(&updated, &new_state, (uint16_t)sizeof(new_state));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = c->write_repo->update(c->write_repo, &updated);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&change, 0, sizeof(change));
    change.old_power = old_state.power_level;
    change.new_power = new_state.power_level;

    memset(&ev, 0, sizeof(ev));
    ev.type = TEST_EVENT_POWER_CHANGED;
    ev.aggregate_id = cmd->entity_id;
    memcpy(ev.data.custom_data, &change, sizeof(change));
    (void)aegis_domain_event_publish(c->bus, &ev);

    memset(result, 0, sizeof(*result));
    result->result = ERR_OK;
    return ERR_OK;
}

typedef struct {
    AegisEntityId id;
    uint16_t charger_model;
    uint8_t power_level;
} TestChargerDto;

static AegisErrorCode handle_get_charger(const AegisQueryRequest* req, AegisQueryResponse* resp, void* ctx) {
    TestQueryCtx* qctx;
    AegisDomainEntity* stored;
    const void* payload_ptr;
    uint16_t payload_size;
    TestChargerState state;
    TestChargerDto dto;
    AegisErrorCode ret;

    if (req == NULL || resp == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    qctx = (TestQueryCtx*)ctx;
    if (qctx->read_repo == NULL) {
        return ERR_NULL_PTR;
    }

    ret = qctx->read_repo->get(qctx->read_repo, req->entity_id, &stored);
    if (ret != ERR_OK) {
        resp->result = ret;
        resp->payload_size = 0U;
        return ERR_OK;
    }

    ret = aegis_domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    if (ret != ERR_OK || payload_ptr == NULL || payload_size != (uint16_t)sizeof(TestChargerState)) {
        resp->result = ERR_OUT_OF_RANGE;
        resp->payload_size = 0U;
        return ERR_OK;
    }

    memset(&state, 0, sizeof(state));
    memcpy(&state, payload_ptr, sizeof(state));

    memset(&dto, 0, sizeof(dto));
    dto.id = stored->base.id;
    dto.charger_model = state.charger_model;
    dto.power_level = state.power_level;

    memset(resp, 0, sizeof(*resp));
    resp->result = ERR_OK;
    resp->payload_size = (uint16_t)sizeof(dto);
    memcpy(resp->payload, &dto, sizeof(dto));
    return ERR_OK;
}

static void drain_one_command(AegisAppRuntime* app, AegisCommandResult* out_result) {
    AegisErrorCode ret;
    AegisCommand cmd;
    AegisCommandResult res;

    memset(&cmd, 0, sizeof(cmd));
    ret = aegis_app_cmd_dequeue(&app->cmd_queue, &cmd);
    assert(ret == ERR_OK);

    memset(&res, 0, sizeof(res));
    ret = aegis_app_cmd_service_execute(&app->cmd_service, &cmd, &res);
    assert(ret == ERR_OK);

    if (out_result != NULL) {
        *out_result = res;
    }
}

int main(void) {
    AegisErrorCode ret;
    AegisTraceLog trace;
    uint32_t tick;
    AegisInfrastructureRepositoryInmem repo;
    const AegisDomainRepositoryWriteInterface* write_repo;
    AegisAppRuntime app;
    AegisAppInitConfig cfg;

    TestCommandCtx cmd_ctx;
    TestQueryCtx query_ctx;
    TestEventStats stats;
    AegisEventSubscription subs[2];

    AegisCommand cmd;
    uint8_t count;
    AegisCommandResult result;
    AegisQueryRequest q;
    AegisQueryResponse qr;
    TestChargerDto dto;
    TestChargerState create_payload;

    printf("========================================\n");
    printf("  Application(CQRS) 单元测试\n");
    printf("========================================\n");

    tick = 0U;
    ret = aegis_trace_log_init(&trace, test_now_ms, &tick);
    assert(ret == ERR_OK);

    ret = aegis_infrastructure_repository_inmem_init(&repo, test_now_ms, &tick);
    assert(ret == ERR_OK);

    write_repo = aegis_infrastructure_repository_inmem_write(&repo);
    assert(write_repo != NULL);

    memset(&stats, 0, sizeof(stats));
    stats.last_created_id = ENTITY_ID_INVALID;

    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = on_entity_created;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    subs[1].event_type = TEST_EVENT_POWER_CHANGED;
    subs[1].handler = on_power_changed;
    subs[1].ctx = &stats;
    subs[1].is_sync = TRUE;
    subs[1].priority = 0U;

    memset(&cfg, 0, sizeof(cfg));
    cfg.trace = &trace;
    cfg.write_repo = write_repo;
    cfg.event_subscriptions = subs;
    cfg.event_subscription_count = 2U;

    ret = aegis_app_init_all(&app, &cfg);
    assert(ret == ERR_OK);

    cmd_ctx.write_repo = write_repo;
    cmd_ctx.bus = &app.event_bus;

    query_ctx.read_repo = &write_repo->read;

    ret = aegis_app_cmd_service_register_handler(&app.cmd_service, TEST_CMD_CREATE_CHARGER, handle_create_charger, &cmd_ctx);
    assert(ret == ERR_OK);
    ret = aegis_app_cmd_service_register_handler(&app.cmd_service, TEST_CMD_SET_POWER, handle_set_power, &cmd_ctx);
    assert(ret == ERR_OK);

    ret = aegis_app_query_register_handler(&app.query, TEST_QUERY_GET_CHARGER, handle_get_charger, &query_ctx);
    assert(ret == ERR_OK);

    /* 1) 命令队列：入队/计数/出队 */
    memset(&cmd, 0, sizeof(cmd));
    APP_CMD_INIT(&cmd, TEST_CMD_CREATE_CHARGER);
    memset(&create_payload, 0, sizeof(create_payload));
    create_payload.charger_model = 1001U;
    create_payload.power_level = 10U;
    APP_CMD_SET_PAYLOAD_BYTES(&cmd, &create_payload, (uint16_t)sizeof(create_payload));

    ret = aegis_app_cmd_enqueue(&app.cmd_queue, &cmd);
    assert(ret == ERR_OK);

    ret = aegis_app_cmd_get_count(&app.cmd_queue, &count);
    assert(ret == ERR_OK);
    assert(count == 1U);

    drain_one_command(&app, &result);
    assert(result.result == ERR_OK);
    assert(result.created_id != ENTITY_ID_INVALID);
    assert(stats.created_count == 1U);
    assert(stats.last_created_id == result.created_id);

    /* 2) 设置功率：触发领域事件 */
    memset(&cmd, 0, sizeof(cmd));
    APP_CMD_INIT(&cmd, TEST_CMD_SET_POWER);
    APP_CMD_SET_ENTITY_ID(&cmd, result.created_id);
    cmd.payload_size = 1U;
    cmd.payload[0] = 55U;

    ret = aegis_app_cmd_enqueue(&app.cmd_queue, &cmd);
    assert(ret == ERR_OK);
    drain_one_command(&app, NULL);

    assert(stats.power_changed_count == 1U);
    assert(stats.last_power_change.old_power == 10U);
    assert(stats.last_power_change.new_power == 55U);

    /* 3) 查询：返回 DTO */
    memset(&q, 0, sizeof(q));
    q.type = TEST_QUERY_GET_CHARGER;
    q.entity_id = result.created_id;
    q.payload_size = 0U;

    memset(&qr, 0, sizeof(qr));
    ret = aegis_app_query_execute(&app.query, &q, &qr);
    assert(ret == ERR_OK);
    assert(qr.result == ERR_OK);
    assert(qr.payload_size == (uint16_t)sizeof(TestChargerDto));

    memset(&dto, 0, sizeof(dto));
    memcpy(&dto, qr.payload, sizeof(dto));

    assert(dto.id == result.created_id);
    assert(dto.charger_model == 1001U);
    assert(dto.power_level == 55U);

    printf("\n✅ CQRS command/query tests passed.\n");
    return 0;
}

