/*
 * @file: aegis_domain_value_object.c
 * @brief: 领域值对象（Value Object）实现
 * @author: jack liu
 * @req: REQ-DOMAIN-060
 * @design: DES-DOMAIN-060
 * @asil: ASIL-B
 */

#include "domain_value_object.h"
#include <string.h>

AegisErrorCode aegis_domain_value_object_init(AegisDomainValueObject* vo,
                                   AegisDomainValueObjectType type,
                                   const void* bytes,
                                   uint16_t size) {
    uint16_t i;
    const uint8_t* src;

    if (vo == NULL) {
        return ERR_NULL_PTR;
    }

    if (bytes == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)DOMAIN_VALUE_OBJECT_MAX_SIZE) {
        return ERR_OUT_OF_RANGE;
    }

    vo->type = type;
    vo->size = size;

    src = (const uint8_t*)bytes;
    for (i = 0; i < size; i++) {
        vo->bytes[i] = src[i];
    }
    for (i = size; i < (uint16_t)DOMAIN_VALUE_OBJECT_MAX_SIZE; i++) {
        vo->bytes[i] = 0U;
    }

    return ERR_OK;
}

bool_t aegis_domain_value_object_equals(const AegisDomainValueObject* a, const AegisDomainValueObject* b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (a->type != b->type) {
        return FALSE;
    }

    if (a->size != b->size) {
        return FALSE;
    }

    if (a->size == 0U) {
        return TRUE;
    }

    return (memcmp(a->bytes, b->bytes, a->size) == 0) ? TRUE : FALSE;
}

