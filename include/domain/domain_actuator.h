/*
 * @file: aegis_domain_actuator.h
 * @brief: 执行器领域服务层
 * @author: jack liu
 * @req: REQ-DOMAIN-030
 * @design: DES-DOMAIN-030
 * @asil: ASIL-B
 * @note: 负责执行器的业务逻辑，不包含存储细节
 */

#ifndef DOMAIN_ACTUATOR_H
#define DOMAIN_ACTUATOR_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 执行器类型定义 ==================== */
#define ACTUATOR_TYPE_MOTOR      0  /* 电机 */
#define ACTUATOR_TYPE_VALVE      1  /* 阀门 */
#define ACTUATOR_TYPE_HEATER     2  /* 加热器 */

/* ==================== 执行器命令定义 ==================== */
#define ACTUATOR_CMD_STOP        0  /* 停止 */
#define ACTUATOR_CMD_START       1  /* 启动 */
#define ACTUATOR_CMD_ADJUST      2  /* 调速/调节 */

/* ==================== 执行器领域服务接口 ==================== */

/*
 * @brief: 创建执行器
 * @param actuator_type: 执行器类型
 * @param actuator_id: 输出创建的执行器ID
 * @return: 错误码
 * @req: REQ-DOMAIN-031
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_actuator_create(uint8_t actuator_type, AegisEntityId* actuator_id);

/*
 * @brief: 获取执行器实体
 * @param actuator_id: 执行器ID
 * @param actuator: 输出执行器实体指针
 * @return: 错误码
 * @req: REQ-DOMAIN-032
 * @isr_safe
 */
AegisErrorCode aegis_domain_actuator_get(AegisEntityId actuator_id, ActuatorEntity** actuator);

/*
 * @brief: 设置执行器命令
 * @param actuator_id: 执行器ID
 * @param command: 命令（ACTUATOR_CMD_XXX）
 * @param power_level: 功率等级（0-100）
 * @return: 错误码
 * @note: 包含命令和功率等级验证逻辑
 * @req: REQ-DOMAIN-033
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_actuator_set_command(AegisEntityId actuator_id,
                                      uint8_t command,
                                      uint8_t power_level);

/*
 * @brief: 验证执行器命令是否有效
 * @param command: 命令
 * @param power_level: 功率等级
 * @return: TRUE=有效, FALSE=无效
 * @req: REQ-DOMAIN-034
 * @isr_safe
 */
bool_t aegis_domain_actuator_validate_command(uint8_t command, uint8_t power_level);

/*
 * @brief: 删除执行器
 * @param actuator_id: 执行器ID
 * @return: 错误码
 * @req: REQ-DOMAIN-035
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_actuator_delete(AegisEntityId actuator_id);

/*
 * @brief: 获取执行器运行状态
 * @param actuator_id: 执行器ID
 * @param is_running: 输出运行状态
 * @return: 错误码
 * @req: REQ-DOMAIN-036
 * @isr_safe
 */
AegisErrorCode aegis_domain_actuator_get_status(AegisEntityId actuator_id, bool_t* is_running);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_ACTUATOR_H */
