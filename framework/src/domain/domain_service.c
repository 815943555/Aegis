/*
 * @file: aegis_domain_service.c
 * @brief: 领域服务（Domain Service）实现（静态注册表）
 * @author: jack liu
 * @req: REQ-DOMAIN-080
 * @design: DES-DOMAIN-080
 * @asil: ASIL-B
 */

#include "domain_service.h"
#include "critical.h"

AegisErrorCode aegis_domain_service_init(AegisDomainService* service) {
    uint8_t i;

    if (service == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();
    for (i = 0; i < (uint8_t)DOMAIN_SERVICE_MAX_HANDLERS; i++) {
        service->handlers[i].op = DOMAIN_SERVICE_OP_INVALID;
        service->handlers[i].handler = NULL;
        service->handlers[i].ctx = NULL;
    }
    service->handler_count = 0;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_domain_service_register_handler(AegisDomainService* service,
                                          AegisDomainServiceOpType op,
                                          DomainServiceHandler handler,
                                          void* ctx) {
    uint8_t i;

    if (service == NULL) {
        return ERR_NULL_PTR;
    }

    if (handler == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    for (i = 0; i < service->handler_count; i++) {
        if (service->handlers[i].op == op) {
            service->handlers[i].handler = handler;
            service->handlers[i].ctx = ctx;
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    if (service->handler_count >= (uint8_t)DOMAIN_SERVICE_MAX_HANDLERS) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    service->handlers[service->handler_count].op = op;
    service->handlers[service->handler_count].handler = handler;
    service->handlers[service->handler_count].ctx = ctx;
    service->handler_count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_domain_service_execute(const AegisDomainService* service,
                                 const AegisDomainServiceRequest* req,
                                 AegisDomainServiceResponse* resp) {
    uint8_t i;
    DomainServiceHandler handler;
    void* ctx;

    if (service == NULL || req == NULL || resp == NULL) {
        return ERR_NULL_PTR;
    }

    if (req->op == DOMAIN_SERVICE_OP_INVALID) {
        resp->result = ERR_INVALID_PARAM;
        resp->created_id = ENTITY_ID_INVALID;
        resp->payload_size = 0U;
        return ERR_INVALID_PARAM;
    }

    resp->result = ERR_OK;
    resp->created_id = ENTITY_ID_INVALID;
    resp->payload_size = 0U;

    handler = NULL;
    ctx = NULL;

    ENTER_CRITICAL();
    for (i = 0; i < service->handler_count; i++) {
        if (service->handlers[i].op == req->op) {
            handler = service->handlers[i].handler;
            ctx = service->handlers[i].ctx;
            break;
        }
    }
    EXIT_CRITICAL();

    if (handler == NULL) {
        resp->result = ERR_NOT_FOUND;
        return ERR_NOT_FOUND;
    }

    resp->result = handler(req, resp, ctx);
    return resp->result;
}
