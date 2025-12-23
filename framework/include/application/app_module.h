/*
 * @file: aegis_app_module.h
 * @brief: Application 模块化注册接口（提升框架可组合性）
 * @author: jack liu
 * @req: REQ-APP-090
 * @design: DES-APP-090
 * @asil: ASIL-B
 *
 * @note:
 * - 通过“模块注册函数列表”，将不同业务模块（BC/子域）按需组合到同一 AegisAppRuntime。
 * - 该抽象仅做组织与错误聚合，不引入任何全局状态。
 */

#ifndef APP_MODULE_H
#define APP_MODULE_H

#include "types.h"
#include "error_codes.h"
#include "app_init.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef AegisErrorCode (*AppModuleRegisterFn)(AegisAppRuntime* app, void* ctx);

typedef struct {
    AppModuleRegisterFn register_fn;
    void* ctx;
} AegisAppModule;

/*
 * @brief: 注册一组模块
 * @param app: AegisAppRuntime
 * @param modules: 模块数组
 * @param count: 模块数量
 * @return: 错误码（任一模块失败则返回对应错误）
 * @req: REQ-APP-091
 * @design: DES-APP-091
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_init_register_modules(AegisAppRuntime* app, const AegisAppModule* modules, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* APP_MODULE_H */
