/*
 * @file: aegis_infrastructure_repository_inmem.h
 * @brief: Infrastructure 层 - 内存仓储实现（In-Memory Repository）
 * @author: jack liu
 * @req: REQ-INFRA-010
 * @design: DES-INFRA-010
 * @asil: ASIL-B
 *
 * @note:
 * - 严格DDD依赖：Domain 仅定义仓储接口；本模块提供默认实现（静态数组，MCU友好）
 * - 本仓储不发布领域事件；领域事件应由 AegisDomainService / Aggregate 在领域层产生并发布
 */

#ifndef INFRASTRUCTURE_REPOSITORY_INMEM_H
#define INFRASTRUCTURE_REPOSITORY_INMEM_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "domain_repository_read.h"
#include "domain_repository_write.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*InfrastructureNowMsFn)(void* ctx);

#ifndef REPOSITORY_MAX_ENTITIES
#define REPOSITORY_MAX_ENTITIES 32U
#endif

typedef struct {
    AegisDomainEntity entity_pool[REPOSITORY_MAX_ENTITIES];
    uint8_t entity_count;
    AegisEntityId next_entity_id;
    bool_t is_initialized;

    InfrastructureNowMsFn now_ms;
    void* now_ms_ctx;

    AegisDomainRepositoryReadInterface read_if;
    AegisDomainRepositoryWriteInterface write_if;
} AegisInfrastructureRepositoryInmem;

/*
 * @brief: 初始化内存仓储实例
 * @param repo: 仓储实例
 * @param now_ms_fn: 时间戳回调（可为NULL，则使用0）
 * @param now_ms_ctx: 时间戳回调上下文
 * @return: 错误码
 * @req: REQ-INFRA-010
 * @design: DES-INFRA-010
 * @asil: ASIL-B
 */
AegisErrorCode aegis_infrastructure_repository_inmem_init(AegisInfrastructureRepositoryInmem* repo,
                                               InfrastructureNowMsFn now_ms_fn,
                                               void* now_ms_ctx);

/*
 * @brief: 获取读仓储接口（绑定到实例）
 * @param repo: 仓储实例
 * @return: 读仓储接口指针
 * @req: REQ-INFRA-011
 * @design: DES-INFRA-011
 * @asil: ASIL-B
 */
const AegisDomainRepositoryReadInterface* aegis_infrastructure_repository_inmem_read(AegisInfrastructureRepositoryInmem* repo);

/*
 * @brief: 获取写仓储接口（绑定到实例）
 * @param repo: 仓储实例
 * @return: 写仓储接口指针
 * @req: REQ-INFRA-012
 * @design: DES-INFRA-012
 * @asil: ASIL-B
 */
const AegisDomainRepositoryWriteInterface* aegis_infrastructure_repository_inmem_write(AegisInfrastructureRepositoryInmem* repo);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_REPOSITORY_INMEM_H */
