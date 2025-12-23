/*
 * @file: aegis_domain_aggregate_root.c
 * @brief: 领域聚合根（Aggregate Root）实现
 * @author: jack liu
 * @req: REQ-DOMAIN-050
 * @design: DES-DOMAIN-050
 * @asil: ASIL-B
 */

#include "domain_aggregate_root.h"

AegisErrorCode aegis_domain_aggregate_root_init(AegisDomainAggregateRoot* root, AegisEntityBase* entity) {
    if (root == NULL) {
        return ERR_NULL_PTR;
    }

    if (entity == NULL) {
        root->entity = NULL;
        return ERR_NULL_PTR;
    }

    if (!entity->is_valid) {
        root->entity = NULL;
        return ERR_INVALID_STATE;
    }

    root->entity = entity;
    return ERR_OK;
}

AegisEntityId aegis_domain_aggregate_root_id(const AegisDomainAggregateRoot* root) {
    if (root == NULL || root->entity == NULL) {
        return ENTITY_ID_INVALID;
    }

    return root->entity->id;
}

AegisEntityType aegis_domain_aggregate_root_type(const AegisDomainAggregateRoot* root) {
    if (root == NULL || root->entity == NULL) {
        return ENTITY_TYPE_INVALID;
    }

    return root->entity->type;
}

AegisErrorCode aegis_domain_aggregate_root_as_domain_entity(AegisDomainAggregateRoot* root, AegisDomainEntity** out_entity) {
    if (root == NULL || out_entity == NULL) {
        return ERR_NULL_PTR;
    }

    if (root->entity == NULL) {
        *out_entity = NULL;
        return ERR_NOT_INITIALIZED;
    }

    *out_entity = (AegisDomainEntity*)root->entity;
    return ERR_OK;
}
