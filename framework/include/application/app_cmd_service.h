/*
 * @file: aegis_app_cmd_service.h
 * @brief: 应用服务（Application Service）- 命令用例编排
 * @author: jack liu
 * @req: REQ-APP-050
 * @design: DES-APP-050
 * @asil: ASIL-B
 *
 * @note:
 * - Application Service 负责“用例流程编排/分发”：把命令路由到注册的处理器
 * - 队列（aegis_app_command.c）只负责排队/调度；本模块负责执行业务命令（由使用方注册处理器）
 */

#ifndef APP_CMD_SERVICE_H
#define APP_CMD_SERVICE_H

#include "types.h"
#include "error_codes.h"
#include "app_command.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef AegisErrorCode (*AppCmdHandler)(const AegisCommand* cmd, AegisCommandResult* result, void* ctx);

#ifndef APP_CMD_SERVICE_MAX_HANDLERS
#define APP_CMD_SERVICE_MAX_HANDLERS 16U
#endif

typedef struct {
    AegisCommandType type;
    AppCmdHandler handler;
    void* ctx;
} AegisAppCmdServiceHandlerEntry;

typedef struct {
    AegisAppCmdServiceHandlerEntry handlers[APP_CMD_SERVICE_MAX_HANDLERS];
    uint8_t handler_count;
} AegisAppCmdService;

/* ==================== 批量注册（提升敏捷开发效率） ==================== */
typedef struct {
    AegisCommandType type;
    AppCmdHandler handler;
    void* ctx;
} AegisAppCmdHandlerDef;

/*
 * @brief: 批量注册命令处理器（按 defs 顺序注册；遇到错误立即返回）
 * @req: REQ-APP-054
 * @design: DES-APP-054
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_service_register_handlers(AegisAppCmdService* service,
                                            const AegisAppCmdHandlerDef* defs,
                                            uint8_t count);

/*
 * @brief: 初始化命令应用服务（清空注册表）
 * @return: 错误码
 * @req: REQ-APP-051
 * @design: DES-APP-051
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_service_init(AegisAppCmdService* service);

/*
 * @brief: 注册命令处理器
 * @param type: 命令类型
 * @param handler: 处理器回调
 * @param ctx: 业务上下文（可为仓储/领域服务指针等，由使用方决定）
 * @return: 错误码
 * @req: REQ-APP-053
 * @design: DES-APP-053
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_service_register_handler(AegisAppCmdService* service,
                                           AegisCommandType type,
                                           AppCmdHandler handler,
                                           void* ctx);

/*
 * @brief: 执行一个命令（同步，主循环调用）
 * @param cmd: 命令
 * @param result: 输出结果
 * @return: 错误码
 * @req: REQ-APP-052
 * @design: DES-APP-052
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_cmd_service_execute(const AegisAppCmdService* service,
                                  const AegisCommand* cmd,
                                  AegisCommandResult* result);

#ifdef __cplusplus
}
#endif

#endif /* APP_CMD_SERVICE_H */
