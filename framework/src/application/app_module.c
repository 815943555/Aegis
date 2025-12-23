/*
 * @file: aegis_app_module.c
 * @brief: Application 模块化注册实现
 * @author: jack liu
 * @req: REQ-APP-090
 * @design: DES-APP-090
 * @asil: ASIL-B
 */

#include "app_module.h"

AegisErrorCode aegis_app_init_register_modules(AegisAppRuntime* app, const AegisAppModule* modules, uint8_t count) {
    uint8_t i;
    AegisErrorCode ret;

    if (app == NULL) {
        return ERR_NULL_PTR;
    }

    if (modules == NULL && count > 0U) {
        return ERR_NULL_PTR;
    }

    for (i = 0; i < count; i++) {
        if (modules[i].register_fn == NULL) {
            return ERR_NULL_PTR;
        }

        ret = modules[i].register_fn(app, modules[i].ctx);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    return ERR_OK;
}
