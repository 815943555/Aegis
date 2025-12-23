/*
 * @file: aegis_app_domain_converter.h
 * @brief: Domain Converter 抽象（DTO -> Domain）
 * @author: jack liu
 * @req: REQ-APP-080
 * @design: DES-APP-080
 * @asil: ASIL-B
 */

#ifndef APP_DOMAIN_CONVERTER_H
#define APP_DOMAIN_CONVERTER_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "app_dto.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef AegisErrorCode (*AppDomainConverter)(const AegisAppDto* dto,
                                        AegisDomainEntity* entity,
                                        void* ctx);

#ifndef APP_CONVERTER_MAX
#define APP_CONVERTER_MAX 16U
#endif

typedef struct {
    AegisDtoType dto_type;
    AppDomainConverter converter;
    void* ctx;
} AegisAppConverterEntry;

typedef struct {
    AegisAppConverterEntry entries[APP_CONVERTER_MAX];
    uint8_t aegis_entry_count;
} AegisAppConverter;

/*
 * @brief: 初始化 Converter 注册表
 * @return: 错误码
 * @req: REQ-APP-081
 * @design: DES-APP-081
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_conv_init(AegisAppConverter* converter);

/*
 * @brief: 注册 Converter（按 DTO 类型）
 * @param dto_type: DTO 类型
 * @param converter: 转换函数
 * @param ctx: 业务上下文
 * @return: 错误码
 * @req: REQ-APP-082
 * @design: DES-APP-082
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_conv_register(AegisAppConverter* converter,
                            AegisDtoType dto_type,
                            AppDomainConverter fn,
                            void* ctx);

/*
 * @brief: 将 DTO 转换为 AegisDomainEntity
 * @param dto_type: DTO 类型（选择对应Converter）
 * @param dto: DTO 输入
 * @param entity: 输出领域实体
 * @return: 错误码
 * @req: REQ-APP-083
 * @design: DES-APP-083
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_conv_from_dto(const AegisAppConverter* converter,
                            AegisDtoType dto_type,
                            const AegisAppDto* dto,
                            AegisDomainEntity* entity);

#ifdef __cplusplus
}
#endif

#endif /* APP_DOMAIN_CONVERTER_H */
