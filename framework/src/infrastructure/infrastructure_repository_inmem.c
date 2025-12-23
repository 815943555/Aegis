/*
 * @file: aegis_infrastructure_repository_inmem.c
 * @brief: Infrastructure 层 - 内存仓储实现（实例化/依赖注入）
 * @author: jack liu
 * @req: REQ-INFRA-010
 * @design: DES-INFRA-010
 * @asil: ASIL-B
 */

#include "infrastructure_repository_inmem.h"
#include "critical.h"
#include <stddef.h>
#include <string.h>

/* ==================== 内部辅助函数 ==================== */
static AegisInfrastructureRepositoryInmem* repo_from_read(const AegisDomainRepositoryReadInterface* self) {
    if (self == NULL) {
        return NULL;
    }
    return (AegisInfrastructureRepositoryInmem*)self->ctx;
}

static AegisInfrastructureRepositoryInmem* repo_from_write(const AegisDomainRepositoryWriteInterface* self) {
    if (self == NULL) {
        return NULL;
    }
    return (AegisInfrastructureRepositoryInmem*)self->read.ctx;
}

static uint32_t repo_now_ms(AegisInfrastructureRepositoryInmem* repo) {
    if (repo == NULL || repo->now_ms == NULL) {
        return 0U;
    }
    return repo->now_ms(repo->now_ms_ctx);
}

static AegisEntityId allocate_entity_id(AegisInfrastructureRepositoryInmem* repo) {
    AegisEntityId id;

    if (repo == NULL) {
        return ENTITY_ID_INVALID;
    }

    ENTER_CRITICAL();
    id = repo->next_entity_id;
    repo->next_entity_id++;
    if (repo->next_entity_id == ENTITY_ID_INVALID) {
        repo->next_entity_id = 1;
    }
    EXIT_CRITICAL();

    return id;
}

static int8_t find_entity_index(AegisInfrastructureRepositoryInmem* repo, AegisEntityId entity_id) {
    uint8_t i;

    if (repo == NULL) {
        return -1;
    }

    for (i = 0; i < repo->entity_count; i++) {
        if (repo->entity_pool[i].base.id == entity_id && repo->entity_pool[i].base.is_valid) {
            return (int8_t)i;
        }
    }

    return -1;
}

/* ==================== 仓储接口实现 ==================== */
static AegisErrorCode repository_init_impl(const AegisDomainRepositoryWriteInterface* self) {
    AegisInfrastructureRepositoryInmem* repo;

    repo = repo_from_write(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    memset(repo->entity_pool, 0, sizeof(repo->entity_pool));
    repo->entity_count = 0;
    repo->next_entity_id = 1;
    repo->is_initialized = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_get_impl(const AegisDomainRepositoryReadInterface* self,
                                     AegisEntityId entity_id,
                                     AegisDomainEntity** entity) {
    AegisInfrastructureRepositoryInmem* repo;
    int8_t index;

    if (entity == NULL) {
        return ERR_NULL_PTR;
    }

    repo = repo_from_read(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();

    index = find_entity_index(repo, entity_id);
    if (index < 0) {
        EXIT_CRITICAL();
        return ERR_NOT_FOUND;
    }

    *entity = &repo->entity_pool[index];

    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_find_by_type_impl(const AegisDomainRepositoryReadInterface* self,
                                              AegisEntityType entity_type,
                                              AegisDomainEntity** entities,
                                              uint8_t max_count,
                                              uint8_t* actual_count) {
    AegisInfrastructureRepositoryInmem* repo;
    uint8_t i;
    uint8_t found_count;

    if (entities == NULL || actual_count == NULL) {
        return ERR_NULL_PTR;
    }

    repo = repo_from_read(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    found_count = 0;

    ENTER_CRITICAL();

    for (i = 0; i < repo->entity_count && found_count < max_count; i++) {
        if (repo->entity_pool[i].base.is_valid &&
            repo->entity_pool[i].base.type == entity_type) {
            entities[found_count] = &repo->entity_pool[i];
            found_count++;
        }
    }

    *actual_count = found_count;

    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_count_by_type_impl(const AegisDomainRepositoryReadInterface* self,
                                               AegisEntityType entity_type,
                                               uint8_t* count) {
    AegisInfrastructureRepositoryInmem* repo;
    uint8_t i;
    uint8_t type_count;

    if (count == NULL) {
        return ERR_NULL_PTR;
    }

    repo = repo_from_read(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    type_count = 0;

    ENTER_CRITICAL();
    for (i = 0; i < repo->entity_count; i++) {
        if (repo->entity_pool[i].base.is_valid &&
            repo->entity_pool[i].base.type == entity_type) {
            type_count++;
        }
    }
    *count = type_count;
    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_create_impl(const AegisDomainRepositoryWriteInterface* self, AegisDomainEntity* entity) {
    AegisInfrastructureRepositoryInmem* repo;
    uint32_t timestamp;

    repo = repo_from_write(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (entity == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (entity->payload_size > (uint16_t)DOMAIN_ENTITY_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    ENTER_CRITICAL();

    if (repo->entity_count >= (uint8_t)REPOSITORY_MAX_ENTITIES) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    if (entity->base.id == ENTITY_ID_INVALID) {
        entity->base.id = allocate_entity_id(repo);
    }

    timestamp = repo_now_ms(repo);
    entity->base.created_at = timestamp;
    entity->base.updated_at = timestamp;
    entity->base.is_valid = TRUE;

    memcpy(&repo->entity_pool[repo->entity_count], entity, sizeof(AegisDomainEntity));
    repo->entity_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_update_impl(const AegisDomainRepositoryWriteInterface* self, AegisDomainEntity* entity) {
    AegisInfrastructureRepositoryInmem* repo;
    int8_t index;
    uint32_t timestamp;
    AegisDomainEntity* stored;

    repo = repo_from_write(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (entity == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (entity->payload_size > (uint16_t)DOMAIN_ENTITY_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    ENTER_CRITICAL();

    index = find_entity_index(repo, entity->base.id);
    if (index < 0) {
        EXIT_CRITICAL();
        return ERR_NOT_FOUND;
    }

    stored = &repo->entity_pool[index];

    /* 保留存储中的created_at，避免调用方覆盖 */
    entity->base.created_at = stored->base.created_at;
    entity->base.is_valid = TRUE;

    timestamp = repo_now_ms(repo);
    entity->base.updated_at = timestamp;

    memcpy(stored, entity, sizeof(AegisDomainEntity));

    EXIT_CRITICAL();

    return ERR_OK;
}

static AegisErrorCode repository_delete_impl(const AegisDomainRepositoryWriteInterface* self, AegisEntityId entity_id) {
    AegisInfrastructureRepositoryInmem* repo;
    int8_t index;

    repo = repo_from_write(self);
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    if (!repo->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();

    index = find_entity_index(repo, entity_id);
    if (index < 0) {
        EXIT_CRITICAL();
        return ERR_NOT_FOUND;
    }

    repo->entity_pool[index].base.is_valid = FALSE;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_infrastructure_repository_inmem_init(AegisInfrastructureRepositoryInmem* repo,
                                               InfrastructureNowMsFn now_ms_fn,
                                               void* now_ms_ctx) {
    if (repo == NULL) {
        return ERR_NULL_PTR;
    }

    memset(repo, 0, sizeof(AegisInfrastructureRepositoryInmem));

    repo->now_ms = now_ms_fn;
    repo->now_ms_ctx = now_ms_ctx;

    repo->read_if.ctx = repo;
    repo->read_if.get = repository_get_impl;
    repo->read_if.find_by_type = repository_find_by_type_impl;
    repo->read_if.count_by_type = repository_count_by_type_impl;

    repo->write_if.read = repo->read_if;
    repo->write_if.init = repository_init_impl;
    repo->write_if.create = repository_create_impl;
    repo->write_if.update = repository_update_impl;
    repo->write_if.delete_entity = repository_delete_impl;

    return ERR_OK;
}

const AegisDomainRepositoryReadInterface* aegis_infrastructure_repository_inmem_read(AegisInfrastructureRepositoryInmem* repo) {
    if (repo == NULL) {
        return NULL;
    }
    return &repo->read_if;
}

const AegisDomainRepositoryWriteInterface* aegis_infrastructure_repository_inmem_write(AegisInfrastructureRepositoryInmem* repo) {
    if (repo == NULL) {
        return NULL;
    }
    return &repo->write_if;
}

