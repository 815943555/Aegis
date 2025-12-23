/*
 * @file: aegis_domain_aggregate.h
 * @brief: 领域聚合（Aggregate）通用抽象（事件暂存/发布）
 * @author: jack liu
 * @req: REQ-DOMAIN-070
 * @design: DES-DOMAIN-070
 * @asil: ASIL-B
 */

#ifndef DOMAIN_AGGREGATE_H
#define DOMAIN_AGGREGATE_H

#include "types.h"
#include "error_codes.h"
#include "domain_aggregate_root.h"
#include "domain_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOMAIN_AGGREGATE_MAX_PENDING_EVENTS
#define DOMAIN_AGGREGATE_MAX_PENDING_EVENTS 8U
#endif

typedef struct {
    AegisDomainAggregateRoot root;
    AegisDomainEvent pending_events[DOMAIN_AGGREGATE_MAX_PENDING_EVENTS];
    uint8_t pending_event_count;
} AegisDomainAggregate;

/*
 * @brief: 初始化聚合（绑定聚合根实体）
 * @param agg: 聚合指针
 * @param root_entity: 根实体指针（EntityBase为首字段）
 * @return: 错误码
 * @req: REQ-DOMAIN-071
 * @design: DES-DOMAIN-071
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_aggregate_init(AegisDomainAggregate* agg, AegisEntityBase* root_entity);

/*
 * @brief: 记录待发布事件（追加到聚合暂存区）
 * @param agg: 聚合指针
 * @param event: 事件
 * @return: 错误码
 * @req: REQ-DOMAIN-072
 * @design: DES-DOMAIN-072
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_aggregate_record_event(AegisDomainAggregate* agg, const AegisDomainEvent* event);

/*
 * @brief: 发布并清空所有暂存事件
 * @param agg: 聚合指针
 * @return: 错误码
 * @req: REQ-DOMAIN-073
 * @design: DES-DOMAIN-073
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_aggregate_publish_pending(AegisDomainAggregate* agg, AegisDomainEventBus* bus);

/*
 * @brief: 清空暂存事件（不发布）
 * @param agg: 聚合指针
 * @req: REQ-DOMAIN-074
 * @design: DES-DOMAIN-074
 * @asil: ASIL-B
 * @isr_safe
 */
void aegis_domain_aggregate_clear_pending(AegisDomainAggregate* agg);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_AGGREGATE_H */
