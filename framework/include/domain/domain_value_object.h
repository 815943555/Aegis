/*
 * @file: aegis_domain_value_object.h
 * @brief: 领域值对象（Value Object）通用抽象（无动态内存）
 * @author: jack liu
 * @req: REQ-DOMAIN-060
 * @design: DES-DOMAIN-060
 * @asil: ASIL-B
 */

#ifndef DOMAIN_VALUE_OBJECT_H
#define DOMAIN_VALUE_OBJECT_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t AegisDomainValueObjectType;
#define DOMAIN_VALUE_OBJECT_TYPE_INVALID ((AegisDomainValueObjectType)0xFFFFU)

#ifndef DOMAIN_VALUE_OBJECT_MAX_SIZE
#define DOMAIN_VALUE_OBJECT_MAX_SIZE 32U
#endif

/*
 * @brief: 值对象（以固定大小字节序列承载，业务可自行定义序列化格式）
 * @note: 只要 bytes 内容相同，即视为“相等”（不依赖 identity）
 */
typedef struct {
    AegisDomainValueObjectType type;
    uint16_t size;
    uint8_t bytes[DOMAIN_VALUE_OBJECT_MAX_SIZE];
} AegisDomainValueObject;

/*
 * @brief: 初始化值对象
 * @param vo: 值对象
 * @param type: 值对象类型
 * @param bytes: 序列化字节（可为NULL当size=0）
 * @param size: 字节长度
 * @return: 错误码
 * @req: REQ-DOMAIN-061
 * @design: DES-DOMAIN-061
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_value_object_init(AegisDomainValueObject* vo,
                                   AegisDomainValueObjectType type,
                                   const void* bytes,
                                   uint16_t size);

/*
 * @brief: 比较两个值对象是否相等（type/size/bytes均相等）
 * @param a: 值对象A
 * @param b: 值对象B
 * @return: TRUE相等，否则FALSE
 * @req: REQ-DOMAIN-062
 * @design: DES-DOMAIN-062
 * @asil: ASIL-B
 * @isr_safe
 */
bool_t aegis_domain_value_object_equals(const AegisDomainValueObject* a, const AegisDomainValueObject* b);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_VALUE_OBJECT_H */
