/*
 * @file: aegis_entry_platform.h
 * @brief: 平台适配入口（提供外部依赖的注入点）
 * @author: jack liu
 * @req: REQ-ENTRY-100
 * @design: DES-ENTRY-100
 * @asil: ASIL-B
 *
 * @note:
 * - 为了保证 Application/Entry 层不直接依赖 Infrastructure，本头文件只暴露“领域仓储接口”。
 * - 具体仓储实现由 port(基础设施/平台) 提供并在链接时选定。
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

/*
 * @brief: 初始化平台侧依赖（如默认仓储实现）
 * @param now_ms_fn: 时间戳回调（可为NULL）
 * @param now_ms_ctx: 回调上下文
 * @return: 错误码
 * @req: REQ-ENTRY-101
 * @design: DES-ENTRY-101
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_entry_platform_init(EntryPlatformNowMsFn now_ms_fn, void* now_ms_ctx);

/*
 * @brief: 获取写仓储接口（注入给 aegis_entry_init_all）
 * @return: 写仓储接口指针；未初始化返回NULL
 * @req: REQ-ENTRY-102
 * @design: DES-ENTRY-102
 * @asil: ASIL-B
 * @isr_safe
 */
const AegisDomainRepositoryWriteInterface* aegis_entry_platform_get_write_repo(void);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_PLATFORM_H */

