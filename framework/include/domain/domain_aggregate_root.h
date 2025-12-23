/*
 * @file: aegis_domain_aggregate_root.h
 * @brief: 领域聚合根（Aggregate Root）抽象：统一封装根实体的访问与类型安全转换
 * @author: jack liu
 * @req: REQ-DOMAIN-050
 * @design: DES-DOMAIN-050
 * @asil: ASIL-B
 */

#ifndef DOMAIN_AGGREGATE_ROOT_H
#define DOMAIN_AGGREGATE_ROOT_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 聚合根句柄 ==================== */
typedef struct {
    AegisEntityBase* entity;      /* 指向聚合根实体（仓储返回的根对象） */
} AegisDomainAggregateRoot;

/* ==================== 聚合根通用接口 ==================== */
/*
 * @brief: 从实体指针初始化聚合根句柄
 * @param root: 聚合根句柄
 * @param entity: 根实体指针
 * @return: 错误码
 * @req: REQ-DOMAIN-051
 * @design: DES-DOMAIN-051
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_aggregate_root_init(AegisDomainAggregateRoot* root, AegisEntityBase* entity);

/*
 * @brief: 获取聚合根ID（无效则返回 ENTITY_ID_INVALID）
 * @param root: 聚合根句柄
 * @return: 实体ID
 * @req: REQ-DOMAIN-052
 * @design: DES-DOMAIN-052
 * @asil: ASIL-B
 * @isr_safe
 */
AegisEntityId aegis_domain_aggregate_root_id(const AegisDomainAggregateRoot* root);

/*
 * @brief: 获取聚合根类型（无效则返回 ENTITY_TYPE_INVALID）
 * @param root: 聚合根句柄
 * @return: 实体类型
 * @req: REQ-DOMAIN-053
 * @design: DES-DOMAIN-053
 * @asil: ASIL-B
 * @isr_safe
 */
AegisEntityType aegis_domain_aggregate_root_type(const AegisDomainAggregateRoot* root);

/*
 * @brief: 转换为通用DomainEntity（要求EntityBase为首字段）
 * @param root: 聚合根句柄
 * @param out_entity: 输出通用实体指针
 * @return: 错误码
 * @req: REQ-DOMAIN-054
 * @design: DES-DOMAIN-054
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_aggregate_root_as_domain_entity(AegisDomainAggregateRoot* root, AegisDomainEntity** out_entity);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_AGGREGATE_ROOT_H */
