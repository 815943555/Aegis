/*
 * @file: aegis_entry_platform.c
 * @brief: stm32f030 平台默认依赖装配（Infrastructure 在此处完成）
 * @author: jack liu
 * @req: REQ-ENTRY-100
 * @design: DES-ENTRY-100
 * @asil: ASIL-B
 */

#include "types.h"
#include "error_codes.h"
#include "domain_repository_write.h"
#include "infrastructure_repository_inmem.h"
#include <string.h>

static AegisInfrastructureRepositoryInmem g_repo;
static bool_t g_is_inited = FALSE;

typedef uint32_t (*EntryPlatformNowMsFn)(void* ctx);

AegisErrorCode aegis_entry_platform_init(EntryPlatformNowMsFn now_ms_fn, void* now_ms_ctx);
const AegisDomainRepositoryWriteInterface* aegis_entry_platform_get_write_repo(void);

AegisErrorCode aegis_entry_platform_init(EntryPlatformNowMsFn now_ms_fn, void* now_ms_ctx) {
    AegisErrorCode ret;

    if (g_is_inited) {
        return ERR_OK;
    }

    ret = aegis_infrastructure_repository_inmem_init(&g_repo,
                                               (InfrastructureNowMsFn)now_ms_fn,
                                               now_ms_ctx);
    if (ret != ERR_OK) {
        memset(&g_repo, 0, sizeof(g_repo));
        return ret;
    }

    g_is_inited = TRUE;
    return ERR_OK;
}

const AegisDomainRepositoryWriteInterface* aegis_entry_platform_get_write_repo(void) {
    if (!g_is_inited) {
        return NULL;
    }
    return aegis_infrastructure_repository_inmem_write(&g_repo);
}
