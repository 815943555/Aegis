/*
 * @file: demo_domain.c
 * @brief: 示例领域层（充电桩功率控制）实现
 * @author: jack liu
 */

#include "demo_domain.h"
#include <string.h>

static AegisErrorCode validate_power_level(uint8_t level) {
    if (level > 100U) {
        return ERR_OUT_OF_RANGE;
    }
    return ERR_OK;
}

static AegisErrorCode publish_entity_created(AegisDomainEventBus* bus, AegisEntityId id, AegisEntityType type) {
    AegisDomainEvent event;

    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    memset(&event, 0, sizeof(AegisDomainEvent));
    event.type = DOMAIN_EVENT_ENTITY_CREATED;
    event.aggregate_id = id;
    event.aegis_trace_id = "REQ-DEMO-CHARGER-CREATE";
    event.data.entity_created.entity_type = type;

    return aegis_domain_event_publish(bus, &event);
}

static AegisErrorCode publish_power_changed(AegisDomainEventBus* bus, AegisEntityId id, uint8_t old_power, uint8_t new_power) {
    AegisDomainEvent event;
    DemoPowerChangedEventData payload;

    if (bus == NULL) {
        return ERR_NULL_PTR;
    }

    memset(&event, 0, sizeof(AegisDomainEvent));
    event.type = DEMO_EVENT_POWER_LEVEL_CHANGED;
    event.aggregate_id = id;
    event.aegis_trace_id = "REQ-DEMO-POWER-CHANGED";

    payload.old_power = old_power;
    payload.new_power = new_power;
    memcpy(event.data.custom_data, &payload, sizeof(DemoPowerChangedEventData));

    return aegis_domain_event_publish(bus, &event);
}

AegisErrorCode demo_domain_charger_create(const AegisDomainRepositoryWriteInterface* repo,
                                     AegisDomainEventBus* bus,
                                     uint16_t charger_model,
                                     uint8_t initial_power_level,
                                     AegisEntityId* out_id) {
    AegisErrorCode ret;
    AegisDomainEntity entity;
    DemoChargerState state;

    if (repo == NULL || bus == NULL || out_id == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_power_level(initial_power_level);
    if (ret != ERR_OK) {
        return ret;
    }

    memset(&entity, 0, sizeof(AegisDomainEntity));
    (void)aegis_domain_entity_init(&entity.base, ENTITY_ID_INVALID, DEMO_ENTITY_TYPE_CHARGER);

    state.charger_model = charger_model;
    state.power_level = initial_power_level;
    ret = aegis_domain_entity_payload_set(&entity, &state, (uint16_t)sizeof(DemoChargerState));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = repo->create(repo, &entity);
    if (ret != ERR_OK) {
        return ret;
    }

    *out_id = entity.base.id;

    (void)publish_entity_created(bus, entity.base.id, entity.base.type);
    (void)publish_power_changed(bus, entity.base.id, 0U, initial_power_level);

    return ERR_OK;
}

AegisErrorCode demo_domain_charger_set_power_level(const AegisDomainRepositoryWriteInterface* repo,
                                              AegisDomainEventBus* bus,
                                              AegisEntityId charger_id,
                                              uint8_t new_power_level) {
    AegisErrorCode ret;
    AegisDomainEntity* entity;
    const void* payload;
    uint16_t payload_size;
    DemoChargerState state;
    uint8_t old_power;

    if (repo == NULL || bus == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_power_level(new_power_level);
    if (ret != ERR_OK) {
        return ret;
    }

    entity = NULL;
    ret = repo->read.get(&repo->read, charger_id, &entity);
    if (ret != ERR_OK || entity == NULL) {
        return ERR_NOT_FOUND;
    }

    payload = NULL;
    payload_size = 0U;
    ret = aegis_domain_entity_payload_get(entity, &payload, &payload_size);
    if (ret != ERR_OK) {
        return ret;
    }

    if (payload_size != (uint16_t)sizeof(DemoChargerState)) {
        return ERR_INVALID_STATE;
    }

    memcpy(&state, payload, sizeof(DemoChargerState));
    old_power = state.power_level;

    if (old_power == new_power_level) {
        return ERR_OK;
    }

    state.power_level = new_power_level;
    ret = aegis_domain_entity_payload_set(entity, &state, (uint16_t)sizeof(DemoChargerState));
    if (ret != ERR_OK) {
        return ret;
    }

    ret = repo->update(repo, entity);
    if (ret != ERR_OK) {
        return ret;
    }

    (void)publish_power_changed(bus, charger_id, old_power, new_power_level);
    return ERR_OK;
}

AegisErrorCode demo_domain_charger_get(const AegisDomainRepositoryReadInterface* repo,
                                  AegisEntityId charger_id,
                                  DemoChargerState* out_state) {
    AegisErrorCode ret;
    AegisDomainEntity* entity;
    const void* payload;
    uint16_t payload_size;

    if (repo == NULL || out_state == NULL) {
        return ERR_NULL_PTR;
    }

    entity = NULL;
    ret = repo->get(repo, charger_id, &entity);
    if (ret != ERR_OK || entity == NULL) {
        return ERR_NOT_FOUND;
    }

    payload = NULL;
    payload_size = 0U;
    ret = aegis_domain_entity_payload_get(entity, &payload, &payload_size);
    if (ret != ERR_OK) {
        return ret;
    }

    if (payload_size != (uint16_t)sizeof(DemoChargerState)) {
        return ERR_INVALID_STATE;
    }

    memcpy(out_state, payload, sizeof(DemoChargerState));
    return ERR_OK;
}

