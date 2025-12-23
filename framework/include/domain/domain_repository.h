/*
 * @file: aegis_domain_repository.h
 * @brief: 领域仓储导出（Read/Write 分离）
 * @author: jack liu
 * @req: REQ-DOMAIN-010
 * @design: DES-DOMAIN-010
 * @asil: ASIL-B
 */

#ifndef DOMAIN_REPOSITORY_H
#define DOMAIN_REPOSITORY_H

#include "domain_repository_read.h"
#include "domain_repository_write.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: 领域仓储端口（读写分离）
 * @note: 严格DDD依赖下，Domain 只定义接口；Infrastructure 提供实现并在 Entry/Application 侧注入。
 */
typedef struct {
    const AegisDomainRepositoryReadInterface* read;
    const AegisDomainRepositoryWriteInterface* write;
} AegisDomainRepositoryPorts;

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_REPOSITORY_H */
