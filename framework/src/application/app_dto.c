/*
 * @file: aegis_app_dto.c
 * @brief: DTO 通用承载实现
 * @author: jack liu
 * @req: REQ-APP-060
 * @design: DES-APP-060
 * @asil: ASIL-B
 */

#include "app_dto.h"
#include <string.h>

AegisErrorCode aegis_app_dto_payload_write(AegisAppDto* dto, const void* payload, uint16_t size) {
    uint16_t i;

    if (dto == NULL) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)APP_DTO_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    if (payload == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    dto->payload_size = size;
    for (i = 0; i < size; i++) {
        dto->payload[i] = ((const uint8_t*)payload)[i];
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_dto_payload_read(const AegisAppDto* dto, void* out, uint16_t expected_size) {
    if (dto == NULL || out == NULL) {
        return ERR_NULL_PTR;
    }

    if (dto->payload_size != expected_size) {
        return ERR_OUT_OF_RANGE;
    }

    if (expected_size > (uint16_t)APP_DTO_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(out, dto->payload, expected_size);
    return ERR_OK;
}

