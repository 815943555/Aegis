#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
模块脚手架生成器（DDD + CQRS + 严格DI）
用于快速生成一套业务模块骨架：domain/application + dto + 注册入口。

示例：
  python3 tools/scaffold_module.py --name charger --out examples/charger

作者: jack liu
"""

import argparse
from pathlib import Path


DOMAIN_H = """/*
 * @file: {name}_domain.h
 * @brief: {Name} 领域层 - 领域模型/规则/领域事件
 * @author: jack liu
 * @req: REQ-{NAME}-DOMAIN
 * @design: DES-{NAME}-DOMAIN
 * @asil: ASIL-B
 */

#ifndef {NAME}_DOMAIN_H
#define {NAME}_DOMAIN_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "domain_event.h"
#include "domain_repository_write.h"

#ifdef __cplusplus
extern "C" {{
#endif

#define {NAME}_ENTITY_TYPE ((EntityType)1U)
#define {NAME}_EVENT_CHANGED ((DomainEventType)(DOMAIN_EVENT_USER_BASE + 1U))

typedef struct {{
    uint16_t model;
    uint8_t value;
}} {Name}State;

typedef struct {{
    uint8_t old_value;
    uint8_t new_value;
}} {Name}ChangedEvent;

ErrorCode {name}_domain_create(const DomainRepositoryWriteInterface* repo,
                              DomainEventBus* bus,
                              uint16_t model,
                              uint8_t initial_value,
                              EntityId* out_id);

ErrorCode {name}_domain_set_value(const DomainRepositoryWriteInterface* repo,
                                 DomainEventBus* bus,
                                 EntityId id,
                                 uint8_t new_value);

ErrorCode {name}_domain_get(const DomainRepositoryReadInterface* repo,
                            EntityId id,
                            {Name}State* out_state);

#ifdef __cplusplus
}}
#endif

#endif /* {NAME}_DOMAIN_H */
"""


DOMAIN_C = """/*
 * @file: {name}_domain.c
 * @brief: {Name} 领域层实现
 * @author: jack liu
 * @req: REQ-{NAME}-DOMAIN
 * @design: DES-{NAME}-DOMAIN
 * @asil: ASIL-B
 */

#include "{name}_domain.h"
#include <string.h>

static ErrorCode publish_created(DomainEventBus* bus, EntityId id) {{
    DomainEvent ev;
    if (bus == NULL) {{
        return ERR_NULL_PTR;
    }}
    memset(&ev, 0, sizeof(ev));
    ev.type = DOMAIN_EVENT_ENTITY_CREATED;
    ev.aggregate_id = id;
    ev.data.entity_created.entity_type = {NAME}_ENTITY_TYPE;
    return domain_event_publish(bus, &ev);
}}

static ErrorCode publish_changed(DomainEventBus* bus, EntityId id, uint8_t old_v, uint8_t new_v) {{
    DomainEvent ev;
    {Name}ChangedEvent payload;
    if (bus == NULL) {{
        return ERR_NULL_PTR;
    }}
    memset(&payload, 0, sizeof(payload));
    payload.old_value = old_v;
    payload.new_value = new_v;
    memset(&ev, 0, sizeof(ev));
    ev.type = {NAME}_EVENT_CHANGED;
    ev.aggregate_id = id;
    memcpy(ev.data.custom_data, &payload, sizeof(payload));
    return domain_event_publish(bus, &ev);
}}

ErrorCode {name}_domain_create(const DomainRepositoryWriteInterface* repo,
                              DomainEventBus* bus,
                              uint16_t model,
                              uint8_t initial_value,
                              EntityId* out_id) {{
    DomainEntity entity;
    {Name}State state;
    ErrorCode ret;

    if (repo == NULL || bus == NULL || out_id == NULL) {{
        return ERR_NULL_PTR;
    }}

    memset(&state, 0, sizeof(state));
    state.model = model;
    state.value = initial_value;

    memset(&entity, 0, sizeof(entity));
    ret = domain_entity_init(&entity.base, ENTITY_ID_INVALID, {NAME}_ENTITY_TYPE);
    if (ret != ERR_OK) {{
        return ret;
    }}
    ret = domain_entity_payload_set(&entity, &state, (uint16_t)sizeof(state));
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = repo->create(repo, &entity);
    if (ret != ERR_OK) {{
        return ret;
    }}

    (void)publish_created(bus, entity.base.id);
    (void)publish_changed(bus, entity.base.id, 0U, initial_value);

    *out_id = entity.base.id;
    return ERR_OK;
}}

ErrorCode {name}_domain_set_value(const DomainRepositoryWriteInterface* repo,
                                 DomainEventBus* bus,
                                 EntityId id,
                                 uint8_t new_value) {{
    DomainEntity* stored;
    DomainEntity updated;
    const void* payload_ptr;
    uint16_t payload_size;
    {Name}State old_state;
    {Name}State new_state;
    ErrorCode ret;

    if (repo == NULL || bus == NULL) {{
        return ERR_NULL_PTR;
    }}

    ret = repo->read.get(&repo->read, id, &stored);
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    if (ret != ERR_OK || payload_ptr == NULL || payload_size != (uint16_t)sizeof({Name}State)) {{
        return ERR_OUT_OF_RANGE;
    }}

    memcpy(&old_state, payload_ptr, sizeof(old_state));
    new_state = old_state;
    new_state.value = new_value;

    updated = *stored;
    ret = domain_entity_payload_set(&updated, &new_state, (uint16_t)sizeof(new_state));
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = repo->update(repo, &updated);
    if (ret != ERR_OK) {{
        return ret;
    }}

    (void)publish_changed(bus, id, old_state.value, new_value);
    return ERR_OK;
}}

ErrorCode {name}_domain_get(const DomainRepositoryReadInterface* repo,
                            EntityId id,
                            {Name}State* out_state) {{
    DomainEntity* stored;
    const void* payload_ptr;
    uint16_t payload_size;
    ErrorCode ret;

    if (repo == NULL || out_state == NULL) {{
        return ERR_NULL_PTR;
    }}

    ret = repo->get(repo, id, &stored);
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = domain_entity_payload_get(stored, &payload_ptr, &payload_size);
    if (ret != ERR_OK || payload_ptr == NULL || payload_size != (uint16_t)sizeof({Name}State)) {{
        return ERR_OUT_OF_RANGE;
    }}

    memcpy(out_state, payload_ptr, sizeof(*out_state));
    return ERR_OK;
}}
"""


APP_H = """/*
 * @file: {name}_application.h
 * @brief: {Name} 应用层 - CQRS 用例编排与注册
 * @author: jack liu
 * @req: REQ-{NAME}-APP
 * @design: DES-{NAME}-APP
 * @asil: ASIL-B
 */

#ifndef {NAME}_APPLICATION_H
#define {NAME}_APPLICATION_H

#include "types.h"
#include "error_codes.h"
#include "app_init.h"
#include "app_command.h"
#include "app_cmd_service.h"
#include "app_query.h"
#include "{name}_domain.h"

#ifdef __cplusplus
extern "C" {{
#endif

#define {NAME}_CMD_CREATE ((CommandType)1U)
#define {NAME}_CMD_SET_VALUE ((CommandType)2U)
#define {NAME}_QUERY_GET ((QueryType)1U)

typedef struct {{
    uint16_t model;
    uint8_t initial_value;
}} {Name}CreateCmd;

typedef struct {{
    uint8_t new_value;
}} {Name}SetValueCmd;

typedef struct {{
    EntityId id;
    uint16_t model;
    uint8_t value;
}} {Name}Dto;

typedef struct {{
    const DomainRepositoryWriteInterface* write_repo;
    DomainEventBus* bus;
    const DomainRepositoryReadInterface* read_repo;
}} {Name}ApplicationCtx;

typedef struct {{
    {Name}ApplicationCtx ctx;
}} {Name}ApplicationModule;

/* 模块注册函数：签名匹配 AppModuleRegisterFn（便于按需拼装多个模块） */
ErrorCode {name}_application_register(AppRuntime* app, void* ctx);

#ifdef __cplusplus
}}
#endif

#endif /* {NAME}_APPLICATION_H */
"""


APP_C = """/*
 * @file: {name}_application.c
 * @brief: {Name} 应用层实现
 * @author: jack liu
 * @req: REQ-{NAME}-APP
 * @design: DES-{NAME}-APP
 * @asil: ASIL-B
 */

#include "{name}_application.h"
#include <string.h>
#include "app_macros.h"

static ErrorCode handle_create(const Command* cmd, CommandResult* result, void* ctx) {{
    {Name}ApplicationCtx* c;
    {Name}CreateCmd p;
    EntityId id;
    ErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {{
        return ERR_NULL_PTR;
    }}
    c = ({Name}ApplicationCtx*)ctx;

    APP_CMD_PAYLOAD_GET(ret, cmd, &p);
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = {name}_domain_create(c->write_repo, c->bus, p.model, p.initial_value, &id);
    if (ret != ERR_OK) {{
        return ret;
    }}

    memset(result, 0, sizeof(*result));
    result->result = ERR_OK;
    result->created_id = id;
    return ERR_OK;
}}

static ErrorCode handle_set_value(const Command* cmd, CommandResult* result, void* ctx) {{
    {Name}ApplicationCtx* c;
    {Name}SetValueCmd p;
    ErrorCode ret;

    if (cmd == NULL || result == NULL || ctx == NULL) {{
        return ERR_NULL_PTR;
    }}
    c = ({Name}ApplicationCtx*)ctx;

    APP_CMD_PAYLOAD_GET(ret, cmd, &p);
    if (ret != ERR_OK) {{
        return ret;
    }}

    ret = {name}_domain_set_value(c->write_repo, c->bus, cmd->entity_id, p.new_value);
    if (ret != ERR_OK) {{
        return ret;
    }}

    memset(result, 0, sizeof(*result));
    result->result = ERR_OK;
    return ERR_OK;
}}

static ErrorCode handle_get(const QueryRequest* req, QueryResponse* resp, void* ctx) {{
    {Name}ApplicationCtx* q;
    {Name}State state;
    {Name}Dto dto;
    ErrorCode ret;

    if (req == NULL || resp == NULL || ctx == NULL) {{
        return ERR_NULL_PTR;
    }}
    q = ({Name}ApplicationCtx*)ctx;

    memset(&state, 0, sizeof(state));
    ret = {name}_domain_get(q->read_repo, req->entity_id, &state);
    if (ret != ERR_OK) {{
        resp->result = ret;
        resp->payload_size = 0U;
        return ERR_OK;
    }}

    memset(&dto, 0, sizeof(dto));
    dto.id = req->entity_id;
    dto.model = state.model;
    dto.value = state.value;

    memset(resp, 0, sizeof(*resp));
    resp->result = ERR_OK;
    APP_QUERY_RESULT_PAYLOAD_SET(ret, resp, &dto);
    if (ret != ERR_OK) {{
        resp->result = ret;
        resp->payload_size = 0U;
        return ret;
    }}
    return ERR_OK;
}}

ErrorCode {name}_application_register(AppRuntime* app, void* ctx) {{
    ErrorCode ret;
    {Name}ApplicationModule* module;
    AppCmdHandlerDef cmd_defs[2];
    AppQueryHandlerDef query_defs[1];

    if (app == NULL || app->write_repo == NULL || ctx == NULL) {{
        return ERR_NULL_PTR;
    }}

    module = ({Name}ApplicationModule*)ctx;
    module->ctx.write_repo = app->write_repo;
    module->ctx.bus = &app->event_bus;
    module->ctx.read_repo = &app->write_repo->read;

    APP_CMD_HANDLER_DEF_SET(&cmd_defs[0], {NAME}_CMD_CREATE, handle_create, &module->ctx);
    APP_CMD_HANDLER_DEF_SET(&cmd_defs[1], {NAME}_CMD_SET_VALUE, handle_set_value, &module->ctx);
    APP_REGISTER_CMD_HANDLERS(ret, &app->cmd_service, cmd_defs);
    if (ret != ERR_OK) {{
        return ret;
    }}

    APP_QUERY_HANDLER_DEF_SET(&query_defs[0], {NAME}_QUERY_GET, handle_get, &module->ctx);
    APP_REGISTER_QUERY_HANDLERS(ret, &app->query, query_defs);
    if (ret != ERR_OK) {{
        return ret;
    }}

    return ERR_OK;
}}
"""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", required=True, help="模块名（lower_snake_case）")
    parser.add_argument("--out", required=True, help="输出目录（例如 examples/charger ）")
    args = parser.parse_args()

    name = args.name.strip()
    if not name or any(c in name for c in " /\\\\."):
        raise SystemExit("name 不合法：只能使用无路径的 lower_snake_case 名称")

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    Name = "".join([p[:1].upper() + p[1:] for p in name.split("_")])
    NAME = name.upper()

    (out_dir / f"{name}_domain.h").write_text(DOMAIN_H.format(name=name, Name=Name, NAME=NAME), encoding="utf-8")
    (out_dir / f"{name}_domain.c").write_text(DOMAIN_C.format(name=name, Name=Name, NAME=NAME), encoding="utf-8")
    (out_dir / f"{name}_application.h").write_text(APP_H.format(name=name, Name=Name, NAME=NAME), encoding="utf-8")
    (out_dir / f"{name}_application.c").write_text(APP_C.format(name=name, Name=Name, NAME=NAME), encoding="utf-8")

    print(f"✅ 已生成模块骨架: {out_dir}")
    print(f"- {name}_domain.[ch]")
    print(f"- {name}_application.[ch]")
    print("下一步：把这些文件加入你的目标工程/示例的 CMakeLists.txt 并注册模块入口。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
