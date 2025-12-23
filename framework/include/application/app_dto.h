/*
 * @file: aegis_app_dto.h
 * @brief: DTO（Data Transfer Object）通用定义（Application 层）
 * @author: jack liu
 * @req: REQ-APP-060
 * @design: DES-APP-060
 * @asil: ASIL-B
 */

#ifndef APP_DTO_H
#define APP_DTO_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t AegisDtoType;
#define DTO_TYPE_INVALID ((AegisDtoType)0xFFFFU)

#ifndef APP_DTO_PAYLOAD_MAX
#define APP_DTO_PAYLOAD_MAX 64U
#endif

typedef struct {
    AegisDtoType type;
    uint16_t version;
    uint16_t payload_size;
    uint8_t payload[APP_DTO_PAYLOAD_MAX];
} AegisAppDto;

/*
 * @brief: 写入DTO payload（会设置payload_size；超出上限返回ERR_OUT_OF_RANGE）
 * @req: REQ-APP-061
 * @design: DES-APP-061
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_dto_payload_write(AegisAppDto* dto, const void* payload, uint16_t size);

/*
 * @brief: 读取DTO payload到结构体（期望size必须精确匹配）
 * @req: REQ-APP-062
 * @design: DES-APP-062
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_dto_payload_read(const AegisAppDto* dto, void* out, uint16_t expected_size);

/*
 * @note: DTO 的具体结构由业务侧定义；框架只提供统一承载与最大尺寸约束。
 */

#ifdef __cplusplus
}
#endif

#endif /* APP_DTO_H */
