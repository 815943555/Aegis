/*
 * @file: aegis_domain_repository_write.h
 * @brief: 领域仓储写接口（CQRS Write side）
 * @author: jack liu
 * @req: REQ-DOMAIN-030
 * @design: DES-DOMAIN-030
 * @asil: ASIL-B
 */

#ifndef DOMAIN_REPOSITORY_WRITE_H
#define DOMAIN_REPOSITORY_WRITE_H

#include "domain_repository_read.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: 写仓储接口（包含读接口 + 写操作）
 * @note: AegisCommand 侧依赖此接口；严格DDD下事件由领域层产生并发布，仓储只负责持久化。
 */
typedef struct AegisDomainRepositoryWriteInterface AegisDomainRepositoryWriteInterface;

struct AegisDomainRepositoryWriteInterface {
    AegisDomainRepositoryReadInterface read;

    AegisErrorCode (*init)(const AegisDomainRepositoryWriteInterface* self);
    AegisErrorCode (*create)(const AegisDomainRepositoryWriteInterface* self, AegisDomainEntity* entity);
    AegisErrorCode (*update)(const AegisDomainRepositoryWriteInterface* self, AegisDomainEntity* entity);
    AegisErrorCode (*delete_entity)(const AegisDomainRepositoryWriteInterface* self, AegisEntityId entity_id);
};

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_REPOSITORY_WRITE_H */
