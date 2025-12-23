/*
 * @file: aegis_app_query.h
 * @brief: CQRS查询系统接口（只读查询）
 * @author: jack liu
 * @req: REQ-APP-010
 * @design: DES-APP-010
 * @asil: ASIL-B
 */

#ifndef APP_QUERY_H
#define APP_QUERY_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 查询结果数据结构 ==================== */
typedef struct {
    uint8_t sensor_count;               /* 传感器数量 */
    uint8_t actuator_count;             /* 执行器数量 */
    uint8_t total_count;                /* 总实体数量 */
} RepositoryStatsQuery;

typedef struct {
    AegisEntityId id;                        /* 实体ID */
    uint8_t sensor_type;                /* 传感器类型 */
    int32_t value;                      /* 当前值 */
    int32_t min_value;                  /* 最小值 */
    int32_t max_value;                  /* 最大值 */
    AegisEntityState state;                  /* 实体状态 */
    uint32_t updated_at;                /* 最后更新时间 */
} SensorDetailsQuery;

typedef struct {
    AegisEntityId id;                        /* 实体ID */
    uint8_t actuator_type;              /* 执行器类型 */
    uint8_t command;                    /* 当前命令 */
    uint8_t power_level;                /* 功率等级 */
    bool_t is_running;                  /* 运行状态 */
    AegisEntityState state;                  /* 实体状态 */
    uint32_t updated_at;                /* 最后更新时间 */
} ActuatorDetailsQuery;

/* ==================== 查询接口 ==================== */
/*
 * @brief: 查询仓储统计信息
 * @param result: 输出统计结果
 * @return: 错误码
 * @req: REQ-APP-011
 * @design: DES-APP-011
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_repository_stats(RepositoryStatsQuery* result);

/*
 * @brief: 查询传感器详情
 * @param sensor_id: 传感器ID
 * @param result: 输出传感器详情
 * @return: 错误码
 * @req: REQ-APP-012
 * @design: DES-APP-012
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_sensor_details(AegisEntityId sensor_id, SensorDetailsQuery* result);

/*
 * @brief: 查询执行器详情
 * @param actuator_id: 执行器ID
 * @param result: 输出执行器详情
 * @return: 错误码
 * @req: REQ-APP-013
 * @design: DES-APP-013
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_actuator_details(AegisEntityId actuator_id, ActuatorDetailsQuery* result);

/*
 * @brief: 检查实体是否存在
 * @param entity_id: 实体ID
 * @param exists: 输出是否存在
 * @return: 错误码
 * @req: REQ-APP-014
 * @design: DES-APP-014
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_entity_exists(AegisEntityId entity_id, bool_t* exists);

#ifdef __cplusplus
}
#endif

#endif /* APP_QUERY_H */
