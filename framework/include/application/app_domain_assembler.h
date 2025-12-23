/*
 * @file: aegis_app_domain_assembler.h
 * @brief: Domain Assembler 抽象（Domain -> DTO）
 * @author: jack liu
 * @req: REQ-APP-070
 * @design: DES-APP-070
 * @asil: ASIL-B
 */

#ifndef APP_DOMAIN_ASSEMBLER_H
#define APP_DOMAIN_ASSEMBLER_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "app_dto.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef AegisErrorCode (*AppDomainAssembler)(const AegisDomainEntity* entity,
                                        AegisAppDto* dto,
                                        void* ctx);

#ifndef APP_ASSEMBLER_MAX
#define APP_ASSEMBLER_MAX 16U
#endif

typedef struct {
    AegisDtoType dto_type;
    AppDomainAssembler assembler;
    void* ctx;
} AegisAppAssemblerEntry;

typedef struct {
    AegisAppAssemblerEntry entries[APP_ASSEMBLER_MAX];
    uint8_t aegis_entry_count;
} AegisAppAssembler;

/*
 * @brief: 初始化 Assembler 注册表
 * @return: 错误码
 * @req: REQ-APP-071
 * @design: DES-APP-071
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_asm_init(AegisAppAssembler* assembler);

/*
 * @brief: 注册 Assembler（按 DTO 类型）
 * @param dto_type: DTO 类型
 * @param assembler: 组装函数
 * @param ctx: 业务上下文
 * @return: 错误码
 * @req: REQ-APP-072
 * @design: DES-APP-072
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_asm_register(AegisAppAssembler* assembler,
                           AegisDtoType dto_type,
                           AppDomainAssembler fn,
                           void* ctx);

/*
 * @brief: 将 AegisDomainEntity 组装为 DTO
 * @param dto_type: DTO 类型（选择对应Assembler）
 * @param entity: 领域实体
 * @param dto: 输出DTO
 * @return: 错误码
 * @req: REQ-APP-073
 * @design: DES-APP-073
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_asm_to_dto(const AegisAppAssembler* assembler,
                         AegisDtoType dto_type,
                         const AegisDomainEntity* entity,
                         AegisAppDto* dto);

#ifdef __cplusplus
}
#endif

#endif /* APP_DOMAIN_ASSEMBLER_H */
