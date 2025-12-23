/*
 * @file: aegis_app_domain_assembler.c
 * @brief: Domain Assembler 实现（静态注册表）
 * @author: jack liu
 * @req: REQ-APP-070
 * @design: DES-APP-070
 * @asil: ASIL-B
 */

#include "app_domain_assembler.h"
#include "critical.h"

AegisErrorCode aegis_app_asm_init(AegisAppAssembler* assembler) {
    uint8_t i;

    if (assembler == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    for (i = 0; i < (uint8_t)APP_ASSEMBLER_MAX; i++) {
        assembler->entries[i].dto_type = DTO_TYPE_INVALID;
        assembler->entries[i].assembler = NULL;
        assembler->entries[i].ctx = NULL;
    }
    assembler->aegis_entry_count = 0;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_asm_register(AegisAppAssembler* assembler,
                           AegisDtoType dto_type,
                           AppDomainAssembler fn,
                           void* ctx) {
    uint8_t i;

    if (assembler == NULL) {
        return ERR_NULL_PTR;
    }

    if (fn == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    for (i = 0; i < assembler->aegis_entry_count; i++) {
        if (assembler->entries[i].dto_type == dto_type) {
            assembler->entries[i].assembler = fn;
            assembler->entries[i].ctx = ctx;
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    if (assembler->aegis_entry_count >= (uint8_t)APP_ASSEMBLER_MAX) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    assembler->entries[assembler->aegis_entry_count].dto_type = dto_type;
    assembler->entries[assembler->aegis_entry_count].assembler = fn;
    assembler->entries[assembler->aegis_entry_count].ctx = ctx;
    assembler->aegis_entry_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_asm_to_dto(const AegisAppAssembler* registry,
                         AegisDtoType dto_type,
                         const AegisDomainEntity* entity,
                         AegisAppDto* dto) {
    uint8_t i;
    AppDomainAssembler assembler_fn;
    void* ctx;

    if (registry == NULL || entity == NULL || dto == NULL) {
        return ERR_NULL_PTR;
    }

    assembler_fn = NULL;
    ctx = NULL;

    ENTER_CRITICAL();
    for (i = 0; i < registry->aegis_entry_count; i++) {
        if (registry->entries[i].dto_type == dto_type) {
            assembler_fn = registry->entries[i].assembler;
            ctx = registry->entries[i].ctx;
            break;
        }
    }
    EXIT_CRITICAL();

    if (assembler_fn == NULL) {
        return ERR_NOT_FOUND;
    }

    return assembler_fn(entity, dto, ctx);
}
