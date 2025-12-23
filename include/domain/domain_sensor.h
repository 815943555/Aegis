/*
 * @file: aegis_domain_sensor.h
 * @brief: 传感器领域服务层
 * @author: jack liu
 * @req: REQ-DOMAIN-020
 * @design: DES-DOMAIN-020
 * @asil: ASIL-B
 * @note: 负责传感器的业务逻辑，不包含存储细节
 */

#ifndef DOMAIN_SENSOR_H
#define DOMAIN_SENSOR_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 传感器类型定义 ==================== */
#define SENSOR_TYPE_TEMPERATURE  0  /* 温度传感器 */
#define SENSOR_TYPE_PRESSURE     1  /* 压力传感器 */
#define SENSOR_TYPE_SPEED        2  /* 速度传感器 */

/* ==================== 传感器领域服务接口 ==================== */

/*
 * @brief: 创建传感器
 * @param sensor_type: 传感器类型
 * @param sensor_id: 输出创建的传感器ID
 * @return: 错误码
 * @req: REQ-DOMAIN-021
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_sensor_create(uint8_t sensor_type, AegisEntityId* sensor_id);

/*
 * @brief: 获取传感器实体
 * @param sensor_id: 传感器ID
 * @param sensor: 输出传感器实体指针
 * @return: 错误码
 * @req: REQ-DOMAIN-022
 * @isr_safe
 */
AegisErrorCode aegis_domain_sensor_get(AegisEntityId sensor_id, SensorEntity** sensor);

/*
 * @brief: 更新传感器值
 * @param sensor_id: 传感器ID
 * @param value: 新值
 * @return: 错误码
 * @note: 包含值范围验证逻辑
 * @req: REQ-DOMAIN-023
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_sensor_update_value(AegisEntityId sensor_id, int32_t value);

/*
 * @brief: 验证传感器值是否在有效范围内
 * @param sensor: 传感器实体
 * @param value: 待验证的值
 * @return: TRUE=有效, FALSE=无效
 * @req: REQ-DOMAIN-024
 * @isr_safe
 */
bool_t aegis_domain_sensor_validate_value(const SensorEntity* sensor, int32_t value);

/*
 * @brief: 删除传感器
 * @param sensor_id: 传感器ID
 * @return: 错误码
 * @req: REQ-DOMAIN-025
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_sensor_delete(AegisEntityId sensor_id);

/*
 * @brief: 获取指定类型的所有传感器
 * @param sensor_type: 传感器类型
 * @param sensors: 输出传感器数组（由调用者分配）
 * @param max_count: 数组最大容量
 * @param actual_count: 输出实际找到的数量
 * @return: 错误码
 * @req: REQ-DOMAIN-026
 * @isr_safe
 */
AegisErrorCode aegis_domain_sensor_find_by_type(uint8_t sensor_type,
                                     SensorEntity** sensors,
                                     uint8_t max_count,
                                     uint8_t* actual_count);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_SENSOR_H */
