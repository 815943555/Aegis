/*
 * @file: aegis_domain_repository_read.h
 * @brief: 领域仓储读接口（CQRS Read side）
 * @author: jack liu
 * @req: REQ-DOMAIN-020
 * @design: DES-DOMAIN-020
 * @asil: ASIL-B
 */

#ifndef DOMAIN_REPOSITORY_READ_H
#define DOMAIN_REPOSITORY_READ_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: 读仓储接口（只读）
 * @note: Query 侧只应依赖此接口，避免在编译期接触写操作。
 */
typedef struct AegisDomainRepositoryReadInterface AegisDomainRepositoryReadInterface;

struct AegisDomainRepositoryReadInterface {
    void* ctx;
    AegisErrorCode (*get)(const AegisDomainRepositoryReadInterface* self,
                     AegisEntityId entity_id,
                     AegisDomainEntity** entity);
    AegisErrorCode (*find_by_type)(const AegisDomainRepositoryReadInterface* self,
                              AegisEntityType entity_type,
                              AegisDomainEntity** entities,
                              uint8_t max_count,
                              uint8_t* actual_count);
    AegisErrorCode (*count_by_type)(const AegisDomainRepositoryReadInterface* self,
                               AegisEntityType entity_type,
                               uint8_t* count);
};

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_REPOSITORY_READ_H */
