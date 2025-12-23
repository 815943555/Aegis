/*
 * @file: aegis_domain_entity.c
 * @brief: 领域实体基础实现
 * @author: jack liu
 */

#include "domain_entity.h"
#include "critical.h"
#include <string.h>

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_domain_entity_init(AegisEntityBase* base, AegisEntityId id, AegisEntityType type) {
    if (base == NULL) {
        return ERR_NULL_PTR;
    }

    if (type == ENTITY_TYPE_INVALID) {
        return ERR_INVALID_PARAM;
    }

    ENTER_CRITICAL();

    base->id = id;
    base->type = type;
    base->state = ENTITY_STATE_INACTIVE;
    base->created_at = 0;
    base->updated_at = 0;
    base->is_valid = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

bool_t aegis_domain_entity_is_valid(const AegisEntityBase* base) {
    bool_t valid;

    if (base == NULL) {
        return FALSE;
    }

    ENTER_CRITICAL();
    valid = base->is_valid;
    EXIT_CRITICAL();

    return valid;
}

AegisErrorCode aegis_domain_entity_update_timestamp(AegisEntityBase* base, uint32_t timestamp) {
    if (base == NULL) {
        return ERR_NULL_PTR;
    }

    if (!base->is_valid) {
        return ERR_INVALID_STATE;
    }

    ENTER_CRITICAL();
    base->updated_at = timestamp;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_domain_entity_payload_set(AegisDomainEntity* entity, const void* payload, uint16_t payload_size) {
    if (entity == NULL) {
        return ERR_NULL_PTR;
    }

    if (payload_size > (uint16_t)DOMAIN_ENTITY_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    ENTER_CRITICAL();

    entity->payload_size = payload_size;
    if (payload_size == 0U) {
        EXIT_CRITICAL();
        return ERR_OK;
    }

    if (payload == NULL) {
        EXIT_CRITICAL();
        return ERR_NULL_PTR;
    }

    memcpy(entity->payload, payload, payload_size);

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_domain_entity_payload_get(const AegisDomainEntity* entity, const void** payload, uint16_t* payload_size) {
    if (entity == NULL || payload == NULL || payload_size == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    *payload = (const void*)entity->payload;
    *payload_size = entity->payload_size;
    EXIT_CRITICAL();

    return ERR_OK;
}
