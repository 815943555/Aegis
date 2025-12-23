/*
 * @file: aegis_entry_main.h
 * @brief: 主循环入口
 * @author: jack liu
 * @req: REQ-ENTRY-010
 * @design: DES-ENTRY-010
 * @asil: ASIL-B
 */

#ifndef ENTRY_MAIN_H
#define ENTRY_MAIN_H

#include "types.h"
#include "error_codes.h"
#include "entry_init.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 主循环接口 ==================== */
/*
 * @brief: 主循环（循环执行命令队列）
 * @param runtime: 入口运行时实例
 * @return: 错误码
 * @req: REQ-ENTRY-011
 * @design: DES-ENTRY-011
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_entry_main_loop(AegisEntryRuntime* runtime);

/*
 * @brief: 主循环单次迭代（用于单元测试）
 * @param runtime: 入口运行时实例
 * @return: 错误码
 * @req: REQ-ENTRY-012
 * @design: DES-ENTRY-012
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_entry_main_loop_once(AegisEntryRuntime* runtime);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_MAIN_H */
