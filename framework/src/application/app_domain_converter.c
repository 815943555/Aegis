/*
 * @file: aegis_app_domain_converter.c
 * @brief: Domain Converter 实现（静态注册表）
 * @author: jack liu
 * @req: REQ-APP-080
 * @design: DES-APP-080
 * @asil: ASIL-B
 */

#include "app_domain_converter.h"
#include "critical.h"

AegisErrorCode aegis_app_conv_init(AegisAppConverter* converter) {
    uint8_t i;

    if (converter == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    for (i = 0; i < (uint8_t)APP_CONVERTER_MAX; i++) {
        converter->entries[i].dto_type = DTO_TYPE_INVALID;
        converter->entries[i].converter = NULL;
        converter->entries[i].ctx = NULL;
    }
    converter->aegis_entry_count = 0;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_conv_register(AegisAppConverter* converter,
                            AegisDtoType dto_type,
                            AppDomainConverter fn,
                            void* ctx) {
    uint8_t i;

    if (converter == NULL) {
        return ERR_NULL_PTR;
    }

    if (fn == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    for (i = 0; i < converter->aegis_entry_count; i++) {
        if (converter->entries[i].dto_type == dto_type) {
            converter->entries[i].converter = fn;
            converter->entries[i].ctx = ctx;
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    if (converter->aegis_entry_count >= (uint8_t)APP_CONVERTER_MAX) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    converter->entries[converter->aegis_entry_count].dto_type = dto_type;
    converter->entries[converter->aegis_entry_count].converter = fn;
    converter->entries[converter->aegis_entry_count].ctx = ctx;
    converter->aegis_entry_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_conv_from_dto(const AegisAppConverter* registry,
                            AegisDtoType dto_type,
                            const AegisAppDto* dto,
                            AegisDomainEntity* entity) {
    uint8_t i;
    AppDomainConverter converter_fn;
    void* ctx;

    if (registry == NULL || dto == NULL || entity == NULL) {
        return ERR_NULL_PTR;
    }

    converter_fn = NULL;
    ctx = NULL;

    ENTER_CRITICAL();
    for (i = 0; i < registry->aegis_entry_count; i++) {
        if (registry->entries[i].dto_type == dto_type) {
            converter_fn = registry->entries[i].converter;
            ctx = registry->entries[i].ctx;
            break;
        }
    }
    EXIT_CRITICAL();

    if (converter_fn == NULL) {
        return ERR_NOT_FOUND;
    }

    return converter_fn(dto, entity, ctx);
}
