/*
 * @file: test_repository_event_integration.c
 * @brief: In-Memory Repository + AegisDomainEventBus 集成测试（读写分离/严格注入）
 * @author: jack liu
 * @req: REQ-TEST-REPO-EVENT
 * @design: DES-TEST-REPO-EVENT
 * @asil: ASIL-B
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "domain_entity.h"
#include "domain_event.h"
#include "domain_repository_read.h"
#include "domain_repository_write.h"
#include "infrastructure_repository_inmem.h"
#include "trace.h"

#define TEST_ENTITY_TYPE_DEVICE ((AegisEntityType)1U)
#define TEST_EVENT_VALUE_CHANGED ((AegisDomainEventType)(DOMAIN_EVENT_USER_BASE + 1U))

typedef struct {
    uint16_t model;
    uint8_t value;
} DeviceState;

typedef struct {
    uint8_t old_value;
    uint8_t new_value;
} ValueChangedEvent;

typedef struct {
    uint32_t created;
    uint32_t updated;
    uint32_t deleted;
    uint32_t value_changed;
    AegisEntityId last_id;
    ValueChangedEvent last_change;
} IntegrationStats;

static uint32_t test_now_ms(void* ctx) {
    uint32_t* tick;
    if (ctx == NULL) {
        return 0U;
    }
    tick = (uint32_t*)ctx;
    (*tick)++;
    return *tick;
}

static AegisEventHandlerResult on_created(const AegisDomainEvent* event, void* ctx) {
    IntegrationStats* s;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (IntegrationStats*)ctx;
    s->created++;
    s->last_id = event->aggregate_id;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_updated(const AegisDomainEvent* event, void* ctx) {
    IntegrationStats* s;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (IntegrationStats*)ctx;
    s->updated++;
    s->last_id = event->aggregate_id;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_deleted(const AegisDomainEvent* event, void* ctx) {
    IntegrationStats* s;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (IntegrationStats*)ctx;
    s->deleted++;
    s->last_id = event->aggregate_id;
    return EVENT_HANDLER_OK;
}

static AegisEventHandlerResult on_value_changed(const AegisDomainEvent* event, void* ctx) {
    IntegrationStats* s;
    ValueChangedEvent payload;
    if (event == NULL || ctx == NULL) {
        return EVENT_HANDLER_ERROR;
    }
    s = (IntegrationStats*)ctx;
    memset(&payload, 0, sizeof(payload));
    memcpy(&payload, event->data.custom_data, sizeof(payload));
    s->value_changed++;
    s->last_change = payload;
    return EVENT_HANDLER_OK;
}

static AegisErrorCode aegis_domain_create_device(const AegisDomainRepositoryWriteInterface* repo,
                                      AegisDomainEventBus* bus,
                                      uint16_t model,
                                      uint8_t value,
                                      AegisEntityId* out_id) {
    AegisErrorCode ret;
    AegisDomainEntity entity;
    DeviceState state;
    AegisDomainEvent ev;

    if (repo == NULL || bus == NULL || out_id == NULL) {
        return ERR_NULL_PTR;
    }

    memset(&entity, 0, sizeof(entity));
    ret = aegis_domain_entity_init(&entity.base, ENTITY_ID_INVALID, TEST_ENTITY_TYPE_DEVICE);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&state, 0, sizeof(state));
    state.model = model;
    state.value = value;
    ret = aegis_domain_entity_payload_set(&entity, &state, (uint16_t)sizeof(state));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = repo->create(repo, &entity);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&ev, 0, sizeof(ev));
    ev.type = DOMAIN_EVENT_ENTITY_CREATED;
    ev.aggregate_id = entity.base.id;
    ev.data.entity_created.entity_type = TEST_ENTITY_TYPE_DEVICE;
    (void)aegis_domain_event_publish(bus, &ev);

    *out_id = entity.base.id;
    return ERR_OK;
}

static AegisErrorCode aegis_domain_set_device_value(const AegisDomainRepositoryWriteInterface* repo,
                                        AegisDomainEventBus* bus,
                                        AegisEntityId id,
                                        uint8_t new_value) {
    AegisErrorCode ret;
    AegisDomainEntity* stored;
    AegisDomainEntity updated;
    const void* payload_ptr;
    uint16_t payload_size;
    DeviceState old_state;
    DeviceState new_state;
    ValueChangedEvent change;
    AegisDomainEvent ev;

    if (repo == NULL || bus == NULL) {
        return ERR_NULL_PTR;
    }

    ret = repo->read.get(&repo->read, id, &stored);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = aegis_domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    if (ret != ERR_OK || payload_ptr == NULL || payload_size != (uint16_t)sizeof(DeviceState)) {
        return ERR_OUT_OF_RANGE;
    }

    memset(&old_state, 0, sizeof(old_state));
    memcpy(&old_state, payload_ptr, sizeof(old_state));

    new_state = old_state;
    new_state.value = new_value;

    updated = *stored;
    ret = aegis_domain_entity_payload_set(&updated, &new_state, (uint16_t)sizeof(new_state));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = repo->update(repo, &updated);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&change, 0, sizeof(change));
    change.old_value = old_state.value;
    change.new_value = new_state.value;

    memset(&ev, 0, sizeof(ev));
    ev.type = TEST_EVENT_VALUE_CHANGED;
    ev.aggregate_id = id;
    memcpy(ev.data.custom_data, &change, sizeof(change));
    (void)aegis_domain_event_publish(bus, &ev);

    memset(&ev, 0, sizeof(ev));
    ev.type = DOMAIN_EVENT_ENTITY_UPDATED;
    ev.aggregate_id = id;
    (void)aegis_domain_event_publish(bus, &ev);

    return ERR_OK;
}

static AegisErrorCode aegis_domain_delete_device(const AegisDomainRepositoryWriteInterface* repo,
                                     AegisDomainEventBus* bus,
                                     AegisEntityId id) {
    AegisErrorCode ret;
    AegisDomainEvent ev;

    if (repo == NULL || bus == NULL) {
        return ERR_NULL_PTR;
    }

    ret = repo->delete_entity(repo, id);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&ev, 0, sizeof(ev));
    ev.type = DOMAIN_EVENT_ENTITY_DELETED;
    ev.aggregate_id = id;
    (void)aegis_domain_event_publish(bus, &ev);

    return ERR_OK;
}

int main(void) {
    AegisErrorCode ret;
    AegisTraceLog trace;
    uint32_t tick;
    AegisInfrastructureRepositoryInmem repo;
    const AegisDomainRepositoryWriteInterface* write_repo;
    const AegisDomainRepositoryReadInterface* read_repo;
    AegisDomainEventBus bus;

    IntegrationStats stats;
    AegisEventSubscription subs[4];
    AegisEntityId id;
    AegisDomainEntity* stored;
    const void* payload_ptr;
    uint16_t payload_size;
    DeviceState state;

    printf("========================================\n");
    printf("  Repository + AegisDomainEvent 集成测试\n");
    printf("========================================\n");

    tick = 0U;
    ret = aegis_trace_log_init(&trace, test_now_ms, &tick);
    assert(ret == ERR_OK);

    ret = aegis_infrastructure_repository_inmem_init(&repo, test_now_ms, &tick);
    assert(ret == ERR_OK);

    write_repo = aegis_infrastructure_repository_inmem_write(&repo);
    read_repo = aegis_infrastructure_repository_inmem_read(&repo);
    assert(write_repo != NULL);
    assert(read_repo != NULL);

    memset(&stats, 0, sizeof(stats));
    stats.last_id = ENTITY_ID_INVALID;

    subs[0].event_type = DOMAIN_EVENT_ENTITY_CREATED;
    subs[0].handler = on_created;
    subs[0].ctx = &stats;
    subs[0].is_sync = TRUE;
    subs[0].priority = 0U;

    subs[1].event_type = DOMAIN_EVENT_ENTITY_UPDATED;
    subs[1].handler = on_updated;
    subs[1].ctx = &stats;
    subs[1].is_sync = TRUE;
    subs[1].priority = 0U;

    subs[2].event_type = DOMAIN_EVENT_ENTITY_DELETED;
    subs[2].handler = on_deleted;
    subs[2].ctx = &stats;
    subs[2].is_sync = TRUE;
    subs[2].priority = 0U;

    subs[3].event_type = TEST_EVENT_VALUE_CHANGED;
    subs[3].handler = on_value_changed;
    subs[3].ctx = &stats;
    subs[3].is_sync = TRUE;
    subs[3].priority = 0U;

    ret = aegis_domain_event_bus_init(&bus, &trace, subs, 4);
    assert(ret == ERR_OK);

    ret = write_repo->init(write_repo);
    assert(ret == ERR_OK);

    /* 1) 创建 */
    ret = aegis_domain_create_device(write_repo, &bus, 1001U, 10U, &id);
    assert(ret == ERR_OK);
    assert(id != ENTITY_ID_INVALID);
    assert(stats.created == 1U);
    assert(stats.last_id == id);

    /* 2) 更新并校验仓储读模型 */
    ret = aegis_domain_set_device_value(write_repo, &bus, id, 55U);
    assert(ret == ERR_OK);
    assert(stats.value_changed == 1U);
    assert(stats.last_change.old_value == 10U);
    assert(stats.last_change.new_value == 55U);
    assert(stats.updated == 1U);

    ret = read_repo->get(read_repo, id, &stored);
    assert(ret == ERR_OK);
    ret = aegis_domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    assert(ret == ERR_OK);
    assert(payload_ptr != NULL);
    assert(payload_size == (uint16_t)sizeof(DeviceState));

    memset(&state, 0, sizeof(state));
    memcpy(&state, payload_ptr, sizeof(state));
    assert(state.model == 1001U);
    assert(state.value == 55U);

    /* 3) 删除 */
    ret = aegis_domain_delete_device(write_repo, &bus, id);
    assert(ret == ERR_OK);
    assert(stats.deleted == 1U);

    ret = read_repo->get(read_repo, id, &stored);
    assert(ret == ERR_NOT_FOUND);

    printf("\n✅ repository + event integration tests passed.\n");
    return 0;
}

