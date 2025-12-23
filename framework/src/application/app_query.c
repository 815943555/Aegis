/*
 * @file: aegis_app_query.c
 * @brief: CQRS查询系统实现（通用查询分发）
 * @author: jack liu
 */

#include "app_query.h"
#include "critical.h"
#include <string.h>

AegisErrorCode aegis_app_query_init(AegisAppQueryDispatcher* dispatcher) {
    uint8_t i;

    if (dispatcher == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    for (i = 0; i < (uint8_t)APP_QUERY_MAX_HANDLERS; i++) {
        dispatcher->handlers[i].type = QUERY_TYPE_INVALID;
        dispatcher->handlers[i].handler = NULL;
        dispatcher->handlers[i].ctx = NULL;
    }
    dispatcher->handler_count = 0;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_query_register_handler(AegisAppQueryDispatcher* dispatcher,
                                     AegisQueryType type,
                                     AppQueryHandler handler,
                                     void* ctx) {
    uint8_t i;

    if (dispatcher == NULL) {
        return ERR_NULL_PTR;
    }

    if (handler == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    for (i = 0; i < dispatcher->handler_count; i++) {
        if (dispatcher->handlers[i].type == type) {
            dispatcher->handlers[i].handler = handler;
            dispatcher->handlers[i].ctx = ctx;
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    if (dispatcher->handler_count >= (uint8_t)APP_QUERY_MAX_HANDLERS) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    dispatcher->handlers[dispatcher->handler_count].type = type;
    dispatcher->handlers[dispatcher->handler_count].handler = handler;
    dispatcher->handlers[dispatcher->handler_count].ctx = ctx;
    dispatcher->handler_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_app_query_register_handlers(AegisAppQueryDispatcher* dispatcher,
                                      const AegisAppQueryHandlerDef* defs,
                                      uint8_t count) {
    uint8_t i;
    AegisErrorCode ret;

    if (dispatcher == NULL) {
        return ERR_NULL_PTR;
    }

    if (defs == NULL && count > 0U) {
        return ERR_NULL_PTR;
    }

    for (i = 0; i < count; i++) {
        ret = aegis_app_query_register_handler(dispatcher, defs[i].type, defs[i].handler, defs[i].ctx);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_query_execute(const AegisAppQueryDispatcher* dispatcher,
                            const AegisQueryRequest* req,
                            AegisQueryResponse* resp) {
    uint8_t i;
    AppQueryHandler handler;
    void* ctx;

    if (dispatcher == NULL || req == NULL || resp == NULL) {
        return ERR_NULL_PTR;
    }

    if (req->type == QUERY_TYPE_INVALID) {
        resp->result = ERR_INVALID_PARAM;
        resp->payload_size = 0U;
        return ERR_INVALID_PARAM;
    }

    handler = NULL;
    ctx = NULL;

    ENTER_CRITICAL();
    for (i = 0; i < dispatcher->handler_count; i++) {
        if (dispatcher->handlers[i].type == req->type) {
            handler = dispatcher->handlers[i].handler;
            ctx = dispatcher->handlers[i].ctx;
            break;
        }
    }
    EXIT_CRITICAL();

    if (handler == NULL) {
        resp->result = ERR_NOT_FOUND;
        resp->payload_size = 0U;
        return ERR_NOT_FOUND;
    }

    resp->result = handler(req, resp, ctx);
    return resp->result;
}

AegisErrorCode aegis_app_query_payload_write(AegisQueryRequest* req, const void* payload, uint16_t size) {
    uint16_t i;

    if (req == NULL) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)APP_QUERY_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    if (payload == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    req->payload_size = size;
    for (i = 0; i < size; i++) {
        req->payload[i] = ((const uint8_t*)payload)[i];
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_query_payload_read(const AegisQueryRequest* req, void* out, uint16_t expected_size) {
    if (req == NULL || out == NULL) {
        return ERR_NULL_PTR;
    }

    if (req->payload_size != expected_size) {
        return ERR_OUT_OF_RANGE;
    }

    if (expected_size > (uint16_t)APP_QUERY_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(out, req->payload, expected_size);
    return ERR_OK;
}

AegisErrorCode aegis_app_query_result_payload_write(AegisQueryResponse* resp, const void* payload, uint16_t size) {
    uint16_t i;

    if (resp == NULL) {
        return ERR_NULL_PTR;
    }

    if (size > (uint16_t)APP_QUERY_RESULT_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    if (payload == NULL && size > 0U) {
        return ERR_NULL_PTR;
    }

    resp->payload_size = size;
    for (i = 0; i < size; i++) {
        resp->payload[i] = ((const uint8_t*)payload)[i];
    }

    return ERR_OK;
}

AegisErrorCode aegis_app_query_result_payload_read(const AegisQueryResponse* resp, void* out, uint16_t expected_size) {
    if (resp == NULL || out == NULL) {
        return ERR_NULL_PTR;
    }

    if (resp->payload_size != expected_size) {
        return ERR_OUT_OF_RANGE;
    }

    if (expected_size > (uint16_t)APP_QUERY_RESULT_PAYLOAD_MAX) {
        return ERR_OUT_OF_RANGE;
    }

    memcpy(out, resp->payload, expected_size);
    return ERR_OK;
}
