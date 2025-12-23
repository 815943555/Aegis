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
#include "ring_buffer.h"
#include "trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 命令队列配置 ==================== */
#ifndef CMD_QUEUE_SIZE
#define CMD_QUEUE_SIZE  16      /* 命令队列大小 */
#endif

/* 单条命令payload上限（由使用方定义二进制结构） */
#ifndef APP_CMD_PAYLOAD_MAX
#define APP_CMD_PAYLOAD_MAX  32U
#endif

/* 单条命令结果payload上限 */
#ifndef APP_CMD_RESULT_PAYLOAD_MAX
#define APP_CMD_RESULT_PAYLOAD_MAX  16U
#endif

/* ==================== 命令类型 ==================== */
typedef uint16_t AegisCommandType;

#define CMD_TYPE_INVALID ((AegisCommandType)0xFFFFU)

/* ==================== 命令数据结构 ==================== */
typedef struct {
    AegisCommandType type;                   /* 命令类型 */
    uint32_t timestamp;                 /* 时间戳 */
    AegisEntityId entity_id;                 /* 目标实体ID（可选，业务自定义含义） */
    uint16_t payload_size;              /* payload有效长度 */
    uint8_t payload[APP_CMD_PAYLOAD_MAX];
} AegisCommand;

/* ==================== 命令结果 ==================== */
typedef struct {
    AegisErrorCode result;                   /* 执行结果 */
    AegisEntityId created_id;                /* 创建的实体ID（仅用于创建命令） */
    uint16_t payload_size;              /* 结果payload长度 */
    uint8_t payload[APP_CMD_RESULT_PAYLOAD_MAX];
} AegisCommandResult;

/* ==================== 可注入实例（严格依赖注入） ==================== */
typedef struct {
    AegisRingBuffer ring;
    uint8_t buffer[CMD_QUEUE_SIZE * sizeof(AegisCommand)];
    AegisTraceLog* trace;
    bool_t is_initialized;
} AegisAppCmdQueue;

/* ==================== 命令队列接口 ==================== */
/*
 * @brief: 初始化命令系统
 * @return: 错误码
 * @req: REQ-APP-002
 * @design: DES-APP-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_init(AegisAppCmdQueue* queue, AegisTraceLog* trace);

/*
 * @brief: 入队命令（ISR安全）
 * @param cmd: 命令指针
 * @return: 错误码
 * @req: REQ-APP-003
 * @design: DES-APP-003
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_enqueue(AegisAppCmdQueue* queue, const AegisCommand* cmd);

/*
 * @brief: 执行下一个命令（主循环调用）
 * @param result: 输出命令结果
 * @return: 错误码，ERR_EMPTY表示队列为空
 * @req: REQ-APP-004
 * @design: DES-APP-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_dequeue(AegisAppCmdQueue* queue, AegisCommand* cmd);

/*
 * @brief: 获取队列状态
 * @param count: 输出当前队列中的命令数
 * @return: 错误码
 * @req: REQ-APP-005
 * @design: DES-APP-005
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_get_count(const AegisAppCmdQueue* queue, uint8_t* count);

/*
 * @brief: 清空命令队列
 * @return: 错误码
 * @req: REQ-APP-006
 * @design: DES-APP-006
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_clear(AegisAppCmdQueue* queue);

/* ==================== 便捷Payload API（减少样板代码） ==================== */
/*
 * @brief: 写入命令payload（会设置payload_size；超出上限返回ERR_OUT_OF_RANGE）
 * @req: REQ-APP-007
 * @design: DES-APP-007
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_payload_write(AegisCommand* cmd, const void* payload, uint16_t size);

/*
 * @brief: 读取命令payload到结构体（期望size必须精确匹配）
 * @req: REQ-APP-008
 * @design: DES-APP-008
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_payload_read(const AegisCommand* cmd, void* out, uint16_t expected_size);

/*
 * @brief: 写入命令结果payload（会设置payload_size；超出上限返回ERR_OUT_OF_RANGE）
 * @req: REQ-APP-009
 * @design: DES-APP-009
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_result_payload_write(AegisCommandResult* result, const void* payload, uint16_t size);

/*
 * @brief: 读取命令结果payload到结构体（期望size必须精确匹配）
 * @req: REQ-APP-014
 * @design: DES-APP-014
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_cmd_result_payload_read(const AegisCommandResult* result, void* out, uint16_t expected_size);

/* ==================== 便捷初始化宏（C89兼容）==================== */
#define APP_CMD_INIT(cmd_ptr, type_val) do { \
    (cmd_ptr)->type = (type_val); \
    (cmd_ptr)->timestamp = 0U; \
    (cmd_ptr)->entity_id = ENTITY_ID_INVALID; \
    (cmd_ptr)->payload_size = 0U; \
} while (0)

#define APP_CMD_SET_ENTITY_ID(cmd_ptr, id_val) do { \
    (cmd_ptr)->entity_id = (id_val); \
} while (0)

#define APP_CMD_SET_PAYLOAD_BYTES(cmd_ptr, src_ptr, size_val) do { \
    uint16_t _i; \
    (cmd_ptr)->payload_size = (size_val); \
    for (_i = 0; _i < (size_val) && _i < (uint16_t)APP_CMD_PAYLOAD_MAX; _i++) { \
        (cmd_ptr)->payload[_i] = ((const uint8_t*)(src_ptr))[_i]; \
    } \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* APP_COMMAND_H */
