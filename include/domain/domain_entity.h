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
typedef enum {
    ENTITY_TYPE_SENSOR = 0,         /* 传感器实体 */
    ENTITY_TYPE_ACTUATOR,           /* 执行器实体 */
    ENTITY_TYPE_CONTROLLER,         /* 控制器实体 */
    ENTITY_TYPE_MAX
} AegisEntityType;

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

/* ==================== 传感器实体 ==================== */
typedef struct {
    AegisEntityBase base;                /* 实体基础信息 */
    uint8_t sensor_type;            /* 传感器类型 (0=温度, 1=压力, 2=速度) */
    int32_t value;                  /* 当前值 */
    int32_t min_value;              /* 最小值 */
    int32_t max_value;              /* 最大值 */
    uint16_t sample_interval_ms;    /* 采样间隔（毫秒） */
} SensorEntity;

/* ==================== 执行器实体 ==================== */
typedef struct {
    AegisEntityBase base;                /* 实体基础信息 */
    uint8_t actuator_type;          /* 执行器类型 (0=电机, 1=阀门, 2=加热器) */
    uint8_t command;                /* 当前命令 (0=停止, 1=启动, 2=调速) */
    uint8_t power_level;            /* 功率等级 (0-100) */
    bool_t is_running;              /* 运行状态 */
} ActuatorEntity;

/* ==================== 实体接口 ==================== */
/*
 * @brief: 初始化实体基类
 * @param base: 实体基类指针
 * @param id: 实体ID
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

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_ENTITY_H */
