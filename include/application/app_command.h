/*
 * @file: aegis_app_command.h
 * @brief: CQRS命令系统接口（ISR安全的命令队列）
 * @author: jack liu
 * @req: REQ-APP-001
 * @design: DES-APP-001
 * @asil: ASIL-B
 */

#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 命令队列配置 ==================== */
#ifndef CMD_QUEUE_SIZE
#define CMD_QUEUE_SIZE  16      /* 命令队列大小 */
#endif

/* ==================== 命令类型 ==================== */
typedef enum {
    CMD_TYPE_CREATE_SENSOR = 0,         /* 创建传感器 */
    CMD_TYPE_UPDATE_SENSOR,             /* 更新传感器值 */
    CMD_TYPE_CREATE_ACTUATOR,           /* 创建执行器 */
    CMD_TYPE_UPDATE_ACTUATOR,           /* 更新执行器命令 */
    CMD_TYPE_DELETE_ENTITY,             /* 删除实体 */
    CMD_TYPE_GPIO_TOGGLE,               /* GPIO翻转 */
    CMD_TYPE_MAX
} AegisCommandType;

/* ==================== 命令数据结构 ==================== */
typedef struct {
    AegisCommandType type;                   /* 命令类型 */
    uint32_t timestamp;                 /* 时间戳 */
    AegisEntityId entity_id;                 /* 实体ID（用于更新/删除） */

    /* 命令参数（联合体节省空间） */
    union {
        /* CREATE_SENSOR */
        struct {
            uint8_t sensor_type;
        } create_sensor;

        /* UPDATE_SENSOR */
        struct {
            int32_t value;
        } update_sensor;

        /* CREATE_ACTUATOR */
        struct {
            uint8_t actuator_type;
        } create_actuator;

        /* UPDATE_ACTUATOR */
        struct {
            uint8_t command;
            uint8_t power_level;
        } update_actuator;

        /* GPIO_TOGGLE */
        struct {
            uint8_t port;
            uint8_t pin;
        } gpio_toggle;
    } params;
} AegisCommand;

/* ==================== 命令结果 ==================== */
typedef struct {
    AegisErrorCode result;                   /* 执行结果 */
    AegisEntityId created_id;                /* 创建的实体ID（仅用于创建命令） */
} AegisCommandResult;

/* ==================== 命令队列接口 ==================== */
/*
 * @brief: 初始化命令系统
 * @return: 错误码
 * @req: REQ-APP-002
 * @design: DES-APP-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_init(void);

/*
 * @brief: 入队命令（ISR安全）
 * @param cmd: 命令指针
 * @return: 错误码
 * @req: REQ-APP-003
 * @design: DES-APP-003
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_enqueue(const AegisCommand* cmd);

/*
 * @brief: 执行下一个命令（主循环调用）
 * @param result: 输出命令结果
 * @return: 错误码，ERR_EMPTY表示队列为空
 * @req: REQ-APP-004
 * @design: DES-APP-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_execute_next(AegisCommandResult* result);

/*
 * @brief: 获取队列状态
 * @param count: 输出当前队列中的命令数
 * @return: 错误码
 * @req: REQ-APP-005
 * @design: DES-APP-005
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_get_count(uint8_t* count);

/*
 * @brief: 清空命令队列
 * @return: 错误码
 * @req: REQ-APP-006
 * @design: DES-APP-006
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_clear(void);

/* ==================== 便捷命令构造宏 ==================== */
#define CMD_CREATE_SENSOR(sensor_type_val) \
    { .type = CMD_TYPE_CREATE_SENSOR, \
      .timestamp = 0, \
      .entity_id = ENTITY_ID_INVALID, \
      .params = { .create_sensor = { .sensor_type = (sensor_type_val) } } }

#define CMD_UPDATE_SENSOR(id, val) \
    { .type = CMD_TYPE_UPDATE_SENSOR, \
      .timestamp = 0, \
      .entity_id = (id), \
      .params = { .update_sensor = { .value = (val) } } }

#define CMD_CREATE_ACTUATOR(actuator_type_val) \
    { .type = CMD_TYPE_CREATE_ACTUATOR, \
      .timestamp = 0, \
      .entity_id = ENTITY_ID_INVALID, \
      .params = { .create_actuator = { .actuator_type = (actuator_type_val) } } }

#define CMD_UPDATE_ACTUATOR(id, cmd_val, power_val) \
    { .type = CMD_TYPE_UPDATE_ACTUATOR, \
      .timestamp = 0, \
      .entity_id = (id), \
      .params = { .update_actuator = { .command = (cmd_val), .power_level = (power_val) } } }

#define CMD_GPIO_TOGGLE(port_val, pin_val) \
    { .type = CMD_TYPE_GPIO_TOGGLE, \
      .timestamp = 0, \
      .entity_id = ENTITY_ID_INVALID, \
      .params = { .gpio_toggle = { .port = (port_val), .pin = (pin_val) } } }

#ifdef __cplusplus
}
#endif

#endif /* APP_COMMAND_H */
