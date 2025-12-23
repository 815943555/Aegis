/*
 * @file: aegis_domain_aggregate.c
 * @brief: 领域聚合（Aggregate）实现
 * @author: jack liu
 * @req: REQ-DOMAIN-070
 * @design: DES-DOMAIN-070
 * @asil: ASIL-B
 */

#include "domain_aggregate.h"
#include <string.h>

AegisErrorCode aegis_domain_aggregate_init(AegisDomainAggregate* agg, AegisEntityBase* root_entity) {
    AegisErrorCode ret;

    if (agg == NULL) {
        return ERR_NULL_PTR;
    }

    memset(agg, 0, sizeof(AegisDomainAggregate));

    ret = aegis_domain_aggregate_root_init(&agg->root, root_entity);
    if (ret != ERR_OK) {
        return ret;
    }

    agg->pending_event_count = 0;
    return ERR_OK;
}

AegisErrorCode aegis_domain_aggregate_record_event(AegisDomainAggregate* agg, const AegisDomainEvent* event) {
    if (agg == NULL || event == NULL) {
        return ERR_NULL_PTR;
    }

    if (agg->pending_event_count >= (uint8_t)DOMAIN_AGGREGATE_MAX_PENDING_EVENTS) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(&agg->pending_events[agg->pending_event_count], event, sizeof(AegisDomainEvent));
    agg->pending_event_count++;
    return ERR_OK;
}

AegisErrorCode aegis_domain_aggregate_publish_pending(AegisDomainAggregate* agg, AegisDomainEventBus* bus) {
    uint8_t i;
    AegisErrorCode ret;

    if (agg == NULL) {
        return ERR_NULL_PTR;
    }

    for (i = 0; i < agg->pending_event_count; i++) {
        ret = aegis_domain_event_publish(bus, &agg->pending_events[i]);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    agg->pending_event_count = 0;
    return ERR_OK;
}

void aegis_domain_aggregate_clear_pending(AegisDomainAggregate* agg) {
    if (agg == NULL) {
        return;
    }
    agg->pending_event_count = 0;
}
