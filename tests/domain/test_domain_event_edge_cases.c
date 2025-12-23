/*
 * @file: test_domain_event_edge_cases.c
 * @brief: 领域事件总线边界条件测试（严格依赖注入）
 * @author: jack liu
 * @req: REQ-TEST-DOMAIN-EVENT-EDGE
 * @design: DES-TEST-DOMAIN-EVENT-EDGE
 * @asil: ASIL-B
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "domain_event.h"
#include "trace.h"

typedef struct {
    uint32_t ok_calls;
    uint32_t err_calls;
} EdgeStats;

static uint32_t test_now_ms(void* ctx) {
    (void)ctx;
    return 1U;
}

static AegisEventHandlerResult ok_handler(const AegisDomainEvent* event, void* ctx) {
    EdgeStats* s;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (EdgeStats*)ctx;
    s->ok_calls++;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult error_handler(const AegisDomainEvent* event, void* ctx) {
    EdgeStats* s;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (EdgeStats*)ctx;
    s->err_calls++;
    return EVENT_HANDLER_ERROR;
}

static void test_null_and_not_initialized(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisDomainEvent event;
    uint8_t pending;
    uint32_t processed;

    printf("\n[TEST] test_null_and_not_initialized\n");

    memset(&event, 0, sizeof(event));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = 1U;

    ret = aegis_domain_event_publish(NULL, &event);
    assert(ret == ERR_NULL_PTR);

    ret = aegis_domain_event_publish(&bus, NULL);
    assert(ret == ERR_NULL_PTR);

    memset(&bus, 0, sizeof(bus));
    ret = aegis_domain_event_publish(&bus, &event);
    assert(ret == ERR_NOT_INITIALIZED);

    ret = aegis_domain_event_get_stats(NULL, &pending, &processed);
    assert(ret == ERR_NULL_PTR);

    ret = aegis_domain_event_get_stats(&bus, NULL, &processed);
    assert(ret == ERR_NULL_PTR);

    ret = aegis_domain_event_get_stats(&bus, &pending, NULL);
    assert(ret == ERR_NULL_PTR);

    assert(aegis_domain_event_get_history(NULL, 0) == NULL);
    assert(aegis_domain_event_get_history(&bus, 0) == NULL);
}

static void test_max_subscriptions_limit(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisTraceLog trace;
    EdgeStats stats;
    AegisEventSubscription subs[MAX_EVENT_SUBSCRIPTIONS + 1];
    uint8_t i;

    printf("\n[TEST] test_max_subscriptions_limit\n");

    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    memset(&stats, 0, sizeof(stats));

    for (i = 0; i < (uint8_t)(MAX_EVENT_SUBSCRIPTIONS + 1); i++) {
        subs[i].event_type = DOMAIN_EVENT_ENTITY_CREATED;
        subs[i].handler = ok_handler;
        subs[i].ctx = &stats;
        subs[i].is_sync = TRUE;
        subs[i].priority = i;
    }

    ret = aegis_domain_event_bus_init(&bus, &trace, subs, (uint8_t)(MAX_EVENT_SUBSCRIPTIONS + 1));
    assert(ret == ERR_OUT_OF_RANGE);
}

static void test_handler_error_does_not_break_publish(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisTraceLog trace;
    AegisDomainEvent event;
    EdgeStats stats;
    AegisEventSubscription subs[2];
    uint8_t pending;
    uint32_t processed;

    printf("\n[TEST] test_handler_error_does_not_break_publish\n");

    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    memset(&stats, 0, sizeof(stats));

    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = error_handler;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    subs[1].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[1].handler = ok_handler;
    subs[1].ctx = &stats;
    subs[1].is_sync = TRUE;
    subs[1].priority = 1U;

    ret = aegis_domain_event_bus_init(&bus, &trace, subs, 2);
    assert(ret == ERR_OK);

    memset(&event, 0, sizeof(event));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = 9U;

    ret = aegis_domain_event_publish(&bus, &event);
    assert(ret == ERR_OK);

    assert(stats.err_calls == 1U);
    assert(stats.ok_calls == 1U);

    ret = aegis_domain_event_get_stats(&bus, &pending, &processed);
    assert(ret == ERR_OK);
    assert(pending == 1U);
}

static void test_queue_and_history_overflow(void) {
    AegisErrorCode ret;
    AegisDomainEventBus bus;
    AegisTraceLog trace;
    AegisDomainEvent event;
    uint8_t i;
    uint8_t pending;
    uint32_t processed;
    const AegisDomainEvent* latest;
    const AegisDomainEvent* oldest;

    printf("\n[TEST] test_queue_and_history_overflow\n");

    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    ret = aegis_domain_event_bus_init(&bus, &trace, NULL, 0);
    assert(ret == ERR_OK);

    memset(&event, 0, sizeof(event));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;

    /* 发布超过异步队列容量的事件（publish 会尽力入队，满了会丢弃但仍返回 OK） */
    for (i = 0; i < (uint8_t)(DOMAIN_EVENT_QUEUE_SIZE + 8U); i++) {
        event.aggregate_id = (AegisEntityId)(100U + i);
        ret = aegis_domain_event_publish(&bus, &event);
        assert(ret == ERR_OK);
    }

    ret = aegis_domain_event_get_stats(&bus, &pending, &processed);
    assert(ret == ERR_OK);
    assert(pending <= (uint8_t)DOMAIN_EVENT_QUEUE_SIZE);

    /* 发布超过历史容量，验证历史环形覆盖 */
    for (i = 0; i < (uint8_t)(DOMAIN_EVENT_HISTORY_SIZE + 5U); i++) {
        event.aggregate_id = (AegisEntityId)(200U + i);
        ret = aegis_domain_event_publish(&bus, &event);
        assert(ret == ERR_OK);
    }

    latest = aegis_domain_event_get_history(&bus, 0);
    assert(latest != NULL);
    assert(latest->aggregate_id == (AegisEntityId)(200U + (DOMAIN_EVENT_HISTORY_SIZE + 5U - 1U)));

    oldest = aegis_domain_event_get_history(&bus, (uint8_t)(DOMAIN_EVENT_HISTORY_SIZE - 1U));
    assert(oldest != NULL);
    assert(oldest->aggregate_id == (AegisEntityId)(200U + 5U));

    (void)aegis_domain_event_process(&bus, 0);
}

int main(void) {
    printf("========================================\n");
    printf("  领域事件总线边界条件测试\n");
    printf("========================================\n");

    test_null_and_not_initialized();
    test_max_subscriptions_limit();
    test_handler_error_does_not_break_publish();
    test_queue_and_history_overflow();

    printf("\n✅ aegis_domain_event edge-case tests finished.\n");
    return 0;
}

