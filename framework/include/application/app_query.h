/*
 * @file: aegis_app_query.h
 * @brief: CQRS查询系统接口（通用查询分发）
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

/* ==================== 查询分发配置 ==================== */
#ifndef APP_QUERY_PAYLOAD_MAX
#define APP_QUERY_PAYLOAD_MAX 32U
#endif

#ifndef APP_QUERY_RESULT_PAYLOAD_MAX
#define APP_QUERY_RESULT_PAYLOAD_MAX 64U
#endif

typedef uint16_t AegisQueryType;
#define QUERY_TYPE_INVALID ((AegisQueryType)0xFFFFU)

typedef struct {
    AegisQueryType type;
    AegisEntityId entity_id;                 /* 可选：目标实体ID */
    uint16_t payload_size;
    uint8_t payload[APP_QUERY_PAYLOAD_MAX];
} AegisQueryRequest;

typedef struct {
    AegisErrorCode result;
    uint16_t payload_size;
    uint8_t payload[APP_QUERY_RESULT_PAYLOAD_MAX];
} AegisQueryResponse;

typedef AegisErrorCode (*AppQueryHandler)(const AegisQueryRequest* req, AegisQueryResponse* resp, void* ctx);

#ifndef APP_QUERY_MAX_HANDLERS
#define APP_QUERY_MAX_HANDLERS 16U
#endif

typedef struct {
    AegisQueryType type;
    AppQueryHandler handler;
    void* ctx;
} AegisAppQueryHandlerEntry;

typedef struct {
    AegisAppQueryHandlerEntry handlers[APP_QUERY_MAX_HANDLERS];
    uint8_t handler_count;
} AegisAppQueryDispatcher;

/* ==================== 批量注册（提升敏捷开发效率） ==================== */
typedef struct {
    AegisQueryType type;
    AppQueryHandler handler;
    void* ctx;
} AegisAppQueryHandlerDef;

/*
 * @brief: 批量注册查询处理器（按 defs 顺序注册；遇到错误立即返回）
 * @req: REQ-APP-019
 * @design: DES-APP-019
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_query_register_handlers(AegisAppQueryDispatcher* dispatcher,
                                      const AegisAppQueryHandlerDef* defs,
                                      uint8_t count);

/*
 * @brief: 初始化查询系统（清空注册表）
 * @return: 错误码
 * @req: REQ-APP-011
 * @design: DES-APP-011
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_query_init(AegisAppQueryDispatcher* dispatcher);

/*
 * @brief: 注册查询处理器
 * @param type: 查询类型
 * @param handler: 处理器
 * @param ctx: 业务上下文
 * @return: 错误码
 * @req: REQ-APP-012
 * @design: DES-APP-012
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_query_register_handler(AegisAppQueryDispatcher* dispatcher,
                                     AegisQueryType type,
                                     AppQueryHandler handler,
                                     void* ctx);

/*
 * @brief: 执行查询（同步）
 * @param req: 请求
 * @param resp: 输出响应
 * @return: 错误码
 * @req: REQ-APP-013
 * @design: DES-APP-013
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_execute(const AegisAppQueryDispatcher* dispatcher,
                            const AegisQueryRequest* req,
                            AegisQueryResponse* resp);

/* ==================== 便捷Payload API（减少样板代码） ==================== */
/*
 * @brief: 写入查询payload（会设置payload_size；超出上限返回ERR_OUT_OF_RANGE）
 * @req: REQ-APP-015
 * @design: DES-APP-015
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_payload_write(AegisQueryRequest* req, const void* payload, uint16_t size);

/*
 * @brief: 读取查询payload到结构体（期望size必须精确匹配）
 * @req: REQ-APP-016
 * @design: DES-APP-016
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_payload_read(const AegisQueryRequest* req, void* out, uint16_t expected_size);

/*
 * @brief: 写入查询结果payload（会设置payload_size；超出上限返回ERR_OUT_OF_RANGE）
 * @req: REQ-APP-017
 * @design: DES-APP-017
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_result_payload_write(AegisQueryResponse* resp, const void* payload, uint16_t size);

/*
 * @brief: 读取查询结果payload到结构体（期望size必须精确匹配）
 * @req: REQ-APP-018
 * @design: DES-APP-018
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_app_query_result_payload_read(const AegisQueryResponse* resp, void* out, uint16_t expected_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_QUERY_H */
