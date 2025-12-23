/*
 * @file: aegis_entry_init.h
 * @brief: 系统初始化入口
 * @author: jack liu
 * @req: REQ-ENTRY-001
 * @design: DES-ENTRY-001
 * @asil: ASIL-B
 */

#ifndef ENTRY_INIT_H
#define ENTRY_INIT_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 系统初始化接口 ==================== */
/*
 * @brief: 初始化所有子系统
 * @return: 错误码
 * @req: REQ-ENTRY-002
 * @design: DES-ENTRY-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_entry_init_all(void);

/*
 * @brief: 获取系统初始化状态
 * @return: TRUE=已初始化，FALSE=未初始化
 * @req: REQ-ENTRY-003
 * @design: DES-ENTRY-003
 * @asil: ASIL-B
 * @isr_safe
 */
bool_t aegis_entry_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_INIT_H */
