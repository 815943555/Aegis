/*
 * @file: aegis_app_cmd_service.c
 * @brief: 应用服务（Application Service）- 命令用例编排实现
 * @author: jack liu
 * @req: REQ-APP-050
 * @design: DES-APP-050
 * @asil: ASIL-B
 */

#include "app_cmd_service.h"
#include "critical.h"

AegisErrorCode aegis_app_cmd_service_init(AegisAppCmdService* service) {
    uint8_t i;

    if (service == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    for (i = 0; i < (uint8_t)APP_CMD_SERVICE_MAX_HANDLERS; i++) {
        service->handlers[i].type = CMD_TYPE_INVALID;
        service->handlers[i].handler = NULL;
        service->handlers[i].ctx = NULL;
    }
    service->handler_count = 0;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_service_register_handler(AegisAppCmdService* service,
                                           AegisCommandType type,
                                           AppCmdHandler handler,
                                           void* ctx) {
    uint8_t i;

    if (service == NULL) {
        return ERR_NULL_PTR;
    }

    if (handler == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    /* 更新已存在的注册 */
    for (i = 0; i < service->handler_count; i++) {
        if (service->handlers[i].type == type) {
            service->handlers[i].handler = handler;
            service->handlers[i].ctx = ctx;
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    if (service->handler_count >= (uint8_t)APP_CMD_SERVICE_MAX_HANDLERS) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    service->handlers[service->handler_count].type = type;
    service->handlers[service->handler_count].handler = handler;
    service->handlers[service->handler_count].ctx = ctx;
    service->handler_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_service_register_handlers(AegisAppCmdService* service,
                                            const AegisAppCmdHandlerDef* defs,
                                            uint8_t count) {
    uint8_t i;
    AegisErrorCode ret;

    if (service == NULL) {
        return ERR_NULL_PTR;
    }

    if (defs == NULL && count > 0U) {
        return ERR_NULL_PTR;
    }

    for (i = 0; i < count; i++) {
        ret = aegis_app_cmd_service_register_handler(service, defs[i].type, defs[i].handler, defs[i].ctx);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_cmd_service_execute(const AegisAppCmdService* service,
                                  const AegisCommand* cmd,
                                  AegisCommandResult* result) {
    uint8_t i;
    AppCmdHandler handler;
    void* ctx;

    if (service == NULL || cmd == NULL || result == NULL) {
        return ERR_NULL_PTR;
    }

    result->result = ERR_OK;
    result->created_id = ENTITY_ID_INVALID;
    result->payload_size = 0U;

    handler = NULL;
    ctx = NULL;

    ENTER_CRITICAL();
    for (i = 0; i < service->handler_count; i++) {
        if (service->handlers[i].type == cmd->type) {
            handler = service->handlers[i].handler;
            ctx = service->handlers[i].ctx;
            break;
        }
    }
    EXIT_CRITICAL();

    if (handler == NULL) {
        result->result = ERR_NOT_FOUND;
        return ERR_NOT_FOUND;
    }

    result->result = handler(cmd, result, ctx);
    return result->result;
}
