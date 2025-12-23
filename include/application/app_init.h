/*
 * @file: aegis_app_init.h
 * @brief: 应用层初始化接口
 * @author: jack liu
 * @req: REQ-APP-020
 * @design: DES-APP-020
 * @asil: ASIL-B
 */

#ifndef APP_INIT_H
#define APP_INIT_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 应用层初始化接口 ==================== */
/*
 * @brief: 初始化应用层(包含领域层和命令查询系统)
 * @return: 错误码
 * @req: REQ-APP-021
 * @design: DES-APP-021
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_init_all(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INIT_H */
