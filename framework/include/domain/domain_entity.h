/*
 * @file: aegis_domain_entity.h
 * @brief: 领域实体基础定义
 * @author: jack liu
 * @req: REQ-DOMAIN-001
 * @design: DES-DOMAIN-001
 * @asil: ASIL-B
 */

#ifndef DOMAIN_ENTITY_H
#define DOMAIN_ENTITY_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 实体ID定义 ==================== */
typedef uint16_t AegisEntityId;

#define ENTITY_ID_INVALID   0xFFFFU

/* ==================== 实体类型 ==================== */
typedef uint16_t AegisEntityType;

#define ENTITY_TYPE_INVALID  ((AegisEntityType)0xFFFFU)

/* ==================== 实体状态 ==================== */
typedef enum {
    ENTITY_STATE_INACTIVE = 0,      /* 未激活 */
    ENTITY_STATE_ACTIVE,            /* 激活 */
    ENTITY_STATE_ERROR,             /* 错误 */
    ENTITY_STATE_MAINTENANCE        /* 维护 */
} AegisEntityState;

/* ==================== 实体基类 ==================== */
typedef struct {
    AegisEntityId id;                    /* 实体唯一标识 */
    AegisEntityType type;                /* 实体类型 */
    AegisEntityState state;              /* 实体状态 */
    uint32_t created_at;            /* 创建时间戳 */
    uint32_t updated_at;            /* 最后更新时间戳 */
    bool_t is_valid;                /* 有效性标记 */
} AegisEntityBase;

/* ==================== 通用实体（可选payload） ==================== */
#ifndef DOMAIN_ENTITY_PAYLOAD_MAX
#define DOMAIN_ENTITY_PAYLOAD_MAX 64U
#endif

typedef struct {
    AegisEntityBase base;                        /* 实体基础信息（必须为首字段） */
    uint16_t payload_size;                  /* payload有效长度 */
    uint8_t payload[DOMAIN_ENTITY_PAYLOAD_MAX]; /* 业务payload（由使用方定义结构并序列化/反序列化） */
} AegisDomainEntity;

/* ==================== 实体接口 ==================== */
/*
 * @brief: 初始化实体基类
 * @param base: 实体基类指针
 * @param id: 实体ID（允许 ENTITY_ID_INVALID 作为“待分配”占位）
 * @param type: 实体类型
 * @return: 错误码
 * @req: REQ-DOMAIN-002
 * @design: DES-DOMAIN-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_entity_init(AegisEntityBase* base, AegisEntityId id, AegisEntityType type);

/*
 * @brief: 验证实体有效性
 * @param base: 实体基类指针
 * @return: TRUE=有效, FALSE=无效
 * @req: REQ-DOMAIN-003
 * @design: DES-DOMAIN-003
 * @asil: ASIL-B
 * @isr_safe
 */
bool_t aegis_domain_entity_is_valid(const AegisEntityBase* base);

/*
 * @brief: 更新实体时间戳
 * @param base: 实体基类指针
 * @param timestamp: 新时间戳
 * @return: 错误码
 * @req: REQ-DOMAIN-004
 * @design: DES-DOMAIN-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_entity_update_timestamp(AegisEntityBase* base, uint32_t timestamp);

/*
 * @brief: 设置通用实体payload
 * @param entity: 通用实体指针
 * @param payload: payload数据
 * @param payload_size: payload长度
 * @return: 错误码
 * @req: REQ-DOMAIN-006
 * @design: DES-DOMAIN-006
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_entity_payload_set(AegisDomainEntity* entity, const void* payload, uint16_t payload_size);

/*
 * @brief: 获取通用实体payload（返回只读视图）
 * @param entity: 通用实体指针
 * @param payload: 输出payload指针
 * @param payload_size: 输出payload长度
 * @return: 错误码
 * @req: REQ-DOMAIN-007
 * @design: DES-DOMAIN-007
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_entity_payload_get(const AegisDomainEntity* entity, const void** payload, uint16_t* payload_size);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_ENTITY_H */
