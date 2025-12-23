/*
 * @file: aegis_entry_platform.h
 * @brief: 平台适配入口（提供外部依赖的注入点）
 * @author: jack liu
 * @req: REQ-ENTRY-100
 * @design: DES-ENTRY-100
 * @asil: ASIL-B
 */

#ifndef ENTRY_PLATFORM_H
#define ENTRY_PLATFORM_H

#include "types.h"
#include "error_codes.h"
#include "domain_repository_write.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*EntryPlatformNowMsFn)(void* ctx);

AegisErrorCode aegis_entry_platform_init(EntryPlatformNowMsFn now_ms_fn, void* now_ms_ctx);
const AegisDomainRepositoryWriteInterface* aegis_entry_platform_get_write_repo(void);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_PLATFORM_H */

