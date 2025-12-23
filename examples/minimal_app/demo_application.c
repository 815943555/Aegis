/*
 * @file: demo_application.c
 * @brief: 示例应用层（CQRS）实现
 * @author: jack liu
 */

#include "demo_application.h"
#include <string.h>
#include "app_macros.h"

static AegisErrorCode handle_create_charger(const AegisCommand* cmd, AegisCommandResult* result, void* ctx) {
    DemoUseCaseDeps* deps;
    DemoCreateChargerCmd payload;
    AegisEntityId created_id;
    AegisErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    deps = (DemoUseCaseDeps*)ctx;

    APP_CMD_PAYLOAD_GET(ret, cmd, &payload);
    if (ret != ERR_OK) {
        result->result = ERR_INVALID_PARAM;
        return ERR_INVALID_PARAM;
    }

    created_id = ENTITY_ID_INVALID;
    ret = demo_domain_charger_create(deps->repo, deps->bus,
                                     payload.charger_model,
                                     payload.initial_power_level,
                                     &created_id);
    result->created_id = created_id;
    result->result = ret;

    return ret;
}

static AegisErrorCode handle_set_power(const AegisCommand* cmd, AegisCommandResult* result, void* ctx) {
    DemoUseCaseDeps* deps;
    DemoSetPowerCmd payload;
    AegisErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    deps = (DemoUseCaseDeps*)ctx;

    if (cmd->entity_id == ENTITY_ID_INVALID) {
        result->result = ERR_INVALID_PARAM;
        return ERR_INVALID_PARAM;
    }

    APP_CMD_PAYLOAD_GET(ret, cmd, &payload);
    if (ret != ERR_OK) {
        result->result = ERR_INVALID_PARAM;
        return ERR_INVALID_PARAM;
    }

    ret = demo_domain_charger_set_power_level(deps->repo, deps->bus,
                                              cmd->entity_id,
                                              payload.new_power_level);
    result->result = ret;
    return ret;
}

static AegisErrorCode handle_get_charger(const AegisQueryRequest* req, AegisQueryResponse* resp, void* ctx) {
    DemoUseCaseDeps* deps;
    DemoChargerState state;
    DemoChargerDto dto;
    AegisErrorCode ret;

    if (req == NULL || resp == NULL || ctx == NULL) {
        return ERR_NULL_PTR;
    }

    deps = (DemoUseCaseDeps*)ctx;

    if (req->entity_id == ENTITY_ID_INVALID) {
        resp->result = ERR_INVALID_PARAM;
        resp->payload_size = 0U;
        return ERR_INVALID_PARAM;
    }

    memset(&state, 0, sizeof(DemoChargerState));
    ret = demo_domain_charger_get(&deps->repo->read, req->entity_id, &state);
    if (ret != ERR_OK) {
        resp->result = ret;
        resp->payload_size = 0U;
        return ret;
    }

    memset(&dto, 0, sizeof(DemoChargerDto));
    dto.id = req->entity_id;
    dto.charger_model = state.charger_model;
    dto.power_level = state.power_level;

    APP_QUERY_RESULT_PAYLOAD_SET(ret, resp, &dto);
    if (ret != ERR_OK) {
        resp->result = ret;
        resp->payload_size = 0U;
        return ret;
    }
    resp->result = ERR_OK;
    return ERR_OK;
}

AegisErrorCode demo_application_register(AegisAppRuntime* app, void* ctx) {
    AegisErrorCode ret;
    DemoApplicationModule* module;
    AegisAppCmdHandlerDef cmd_defs[2];
    AegisAppQueryHandlerDef query_defs[1];

    if (app == NULL || !app->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (ctx == NULL) {
        return ERR_NULL_PTR;
    }

    module = (DemoApplicationModule*)ctx;

    module->deps.repo = app->write_repo;
    module->deps.bus = &app->event_bus;

    APP_CMD_HANDLER_DEF_SET(&cmd_defs[0], DEMO_CMD_CREATE_CHARGER, handle_create_charger, &module->deps);
    APP_CMD_HANDLER_DEF_SET(&cmd_defs[1], DEMO_CMD_SET_POWER_LEVEL, handle_set_power, &module->deps);
    APP_REGISTER_CMD_HANDLERS(ret, &app->cmd_service, cmd_defs);
    if (ret != ERR_OK) {
        return ret;
    }

    APP_QUERY_HANDLER_DEF_SET(&query_defs[0], DEMO_QUERY_GET_CHARGER, handle_get_charger, &module->deps);
    APP_REGISTER_QUERY_HANDLERS(ret, &app->query, query_defs);
    if (ret != ERR_OK) {
        return ret;
    }

    return ERR_OK;
}
