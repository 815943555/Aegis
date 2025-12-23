/*
 * @file: test_domain_event.c
 * @brief: 领域事件总线单元测试（实例化/严格依赖注入）
 * @author: jack liu
 * @req: REQ-TEST-DOMAIN-EVENT
 * @design: DES-TEST-DOMAIN-EVENT
 * @asil: ASIL-B
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "domain_event.h"
#include "trace.h"

typedef struct {
    uint32_t sync_count;
    uint32_t async_count;
    AegisDomainEventType last_type;
    AegisEntityId last_aggregate_id;
} EventStats;

static uint32_t test_now_ms(void* ctx) {
    uint32_t* tick;
    if (ctx == NULL) {
        return 0U;
    }
    tick = (uint32_t*)ctx;
    (*tick)++;
    return *tick;
}

static AegisEventHandlerResult on_sync_created(const AegisDomainEvent* event, void* ctx) {
    EventStats* stats;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventStats*)ctx;
    stats->sync_count++;
    stats->last_type = event->type;
    stats->last_aggregate_id = event->aggregate_id;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_async_created(const AegisDomainEvent* event, void* ctx) {
    EventStats* stats;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventStats*)ctx;
    stats->async_count++;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_async_all(const AegisDomainEvent* event, void* ctx) {
    EventStats* stats;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    stats = (EventStats*)ctx;
    stats->async_count++;
    return EVENT_HANDLER_OK;
}

static void test_event_bus_init(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisTraceLog trace;
    uint32_t tick;

    printf("\n[TEST] test_event_bus_init\n");

    tick = 0U;
    ret = aegis_trace_log_init(&trace, test_now_ms, &tick);
    assert(ret == ERR_OK);

    ret = aegis_domain_event_bus_init(&bus, &trace, NULL, 0);
    assert(ret == ERR_OK);

    ret = aegis_domain_event_bus_init(NULL, &trace, NULL, 0);
    assert(ret == ERR_NULL_PTR);

    ret = aegis_domain_event_bus_init(&bus, &trace, NULL, 1);
    assert(ret == ERR_NULL_PTR);
}

static void test_sync_and_async_dispatch(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisDomainEvent event;
    AegisTraceLog trace;
    uint32_t tick;
    EventStats stats;
    uint8_t pending;
    uint32_t processed;
    uint8_t processed_now;

    AegisEventSubscription subs[3];

    printf("\n[TEST] test_sync_and_async_dispatch\n");

    tick = 0U;
    (void)aegis_trace_log_init(&trace, test_now_ms, &tick);

    memset(&stats, 0, sizeof(stats));
    stats.last_type = DOMAIN_EVENT_NONE;
    stats.last_aggregate_id = ENTITY_ID_INVALID;

    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = on_sync_created;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    subs[1].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[1].handler = on_async_created;
    subs[1].ctx = &stats;
    subs[1].is_sync = FALSE;
    subs[1].priority = 10U;

    subs[2].event_type = 0; /* all */
    subs[2].handler = on_async_all;
    subs[2].ctx = &stats;
    subs[2].is_sync = FALSE;
    subs[2].priority = 20U;

    ret = aegis_domain_event_bus_init(&bus, &trace, subs, 3);
    assert(ret == ERR_OK);

    memset(&event, 0, sizeof(event));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = 42U;
    event.aegis_trace_id = "TEST-EVT-001";
    event.data.entity_created.entity_type = (AegisEntityType)7U;

    ret = aegis_domain_event_publish(&bus, &event);
    assert(ret == ERR_OK);

    assert(stats.sync_count == 1U);
    assert(stats.last_type == DOMAIN_EVENT_ENTITY_CREATED);
    assert(stats.last_aggregate_id == 42U);

    ret = aegis_domain_event_get_stats(&bus, &pending, &processed);
    assert(ret == ERR_OK);
    assert(pending == 1U);
    assert(processed == 0U);

    processed_now = aegis_domain_event_process(&bus, 0);
    assert(processed_now == 1U);
    assert(stats.async_count == 2U); /* async_created + async_all */

    ret = aegis_domain_event_get_stats(&bus, &pending, &processed);
    assert(ret == ERR_OK);
    assert(pending == 0U);
    assert(processed == 1U);
}

static void test_event_history(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisDomainEvent event;
    AegisTraceLog trace;
    uint32_t tick;
    EventStats stats;
    const AegisDomainEvent* latest;

    AegisEventSubscription subs[1];

    printf("\n[TEST] test_event_history\n");

    tick = 100U;
    (void)aegis_trace_log_init(&trace, test_now_ms, &tick);

    memset(&stats, 0, sizeof(stats));

    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = on_sync_created;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    ret = aegis_domain_event_bus_init(&bus, &trace, subs, 1);
    assert(ret == ERR_OK);

    memset(&event, 0, sizeof(event));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = 1U;
    event.aegis_trace_id = "TEST-EVT-H-1";
    event.data.entity_created.entity_type = (AegisEntityType)1U;
    ret = aegis_domain_event_publish(&bus, &event);
    assert(ret == ERR_OK);

    event.aggregate_id = 2U;
    event.aegis_trace_id = "TEST-EVT-H-2";
    ret = aegis_domain_event_publish(&bus, &event);
    assert(ret == ERR_OK);

    latest = aegis_domain_event_get_history(&bus, 0);
    assert(latest != NULL);
    assert(latest->aggregate_id == 2U);
    assert(latest->type == DOMAIN_EVENT_ENTITY_CREATED);
}

int main(void) {
    printf("========================================\n");
    printf("  领域事件总线单元测试\n");
    printf("========================================\n");

    test_event_bus_init();
    test_sync_and_async_dispatch();
    test_event_history();

    printf("\n✅ aegis_domain_event tests finished.\n");
    return 0;
}

