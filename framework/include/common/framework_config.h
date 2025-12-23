/*
 * @file: framework_config.h
 * @brief: 框架编译期配置点（提升灵活度/可裁剪性）
 * @author: jack liu
 * @req: REQ-COMMON-100
 * @design: DES-COMMON-100
 * @asil: ASIL-B
 *
 * @note:
 * - 通过编译器宏（CMake `add_compile_definitions`）覆盖这些默认值，以适配不同资源/需求。
 * - 该文件不包含任何其他头文件，避免循环依赖；建议由 `types.h` 统一引入。
 */

#ifndef FRAMEWORK_CONFIG_H
#define FRAMEWORK_CONFIG_H

/* ==================== Domain 配置 ==================== */
/* AegisDomainEntity 的业务payload上限（字节） */
#ifndef DOMAIN_ENTITY_PAYLOAD_MAX
#define DOMAIN_ENTITY_PAYLOAD_MAX 64U
#endif

/* AegisDomainEvent 自定义数据上限（字节） */
#ifndef DOMAIN_EVENT_CUSTOM_DATA_MAX
#define DOMAIN_EVENT_CUSTOM_DATA_MAX 64U
#endif

/* Aggregate 暂存事件数量上限 */
#ifndef DOMAIN_AGGREGATE_MAX_PENDING_EVENTS
#define DOMAIN_AGGREGATE_MAX_PENDING_EVENTS 8U
#endif

/* ValueObject 序列化字节上限 */
#ifndef DOMAIN_VALUE_OBJECT_MAX_SIZE
#define DOMAIN_VALUE_OBJECT_MAX_SIZE 32U
#endif

/* ==================== Application 配置 ==================== */
/* AegisCommand payload 上限（字节） */
#ifndef APP_CMD_PAYLOAD_MAX
#define APP_CMD_PAYLOAD_MAX 32U
#endif

/* AegisCommand result payload 上限（字节） */
#ifndef APP_CMD_RESULT_PAYLOAD_MAX
#define APP_CMD_RESULT_PAYLOAD_MAX 16U
#endif

/* Query payload 上限（字节） */
#ifndef APP_QUERY_PAYLOAD_MAX
#define APP_QUERY_PAYLOAD_MAX 32U
#endif

/* Query result payload 上限（字节） */
#ifndef APP_QUERY_RESULT_PAYLOAD_MAX
#define APP_QUERY_RESULT_PAYLOAD_MAX 64U
#endif

/* DTO payload 上限（字节） */
#ifndef APP_DTO_PAYLOAD_MAX
#define APP_DTO_PAYLOAD_MAX 64U
#endif

#endif /* FRAMEWORK_CONFIG_H */

