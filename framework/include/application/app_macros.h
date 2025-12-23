/*
 * @file: aegis_app_macros.h
 * @brief: Application 层便捷宏（提升开发效率并做编译期约束）
 * @author: jack liu
 * @req: REQ-APP-070
 * @design: DES-APP-070
 * @asil: ASIL-B
 *
 * @note:
 * - 这些宏不引入任何 infrastructure 依赖；用于应用开发的“强制约束 + 少样板”。
 * - 宏内部使用编译期断言限制 payload 结构体尺寸，防止超出框架上限。
 */

#ifndef APP_MACROS_H
#define APP_MACROS_H

#include "compile_time.h"
#include "app_command.h"
#include "app_cmd_service.h"
#include "app_query.h"
#include "app_dto.h"
#include "app_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Module 组合辅助（严格DI） ==================== */
#define APP_MODULE_SET(mod_ptr, register_fn_val, ctx_val) do { \
    (mod_ptr)->register_fn = (register_fn_val); \
    (mod_ptr)->ctx = (ctx_val); \
} while (0)

#define APP_REGISTER_MODULES(ret_var, aegis_app_ptr, modules_arr) do { \
    (ret_var) = aegis_app_init_register_modules((aegis_app_ptr), (modules_arr), (uint8_t)FW_ARRAY_SIZE(modules_arr)); \
} while (0)

/* ==================== Handler 表批量注册（声明式） ==================== */
#define APP_CMD_HANDLER_DEF_INIT(type_val, handler_fn, ctx_val) { (type_val), (handler_fn), (ctx_val) }
#define APP_CMD_HANDLER_DEF_SET(def_ptr, type_val, handler_fn, ctx_val) do { \
    (def_ptr)->type = (type_val); \
    (def_ptr)->handler = (handler_fn); \
    (def_ptr)->ctx = (ctx_val); \
} while (0)

#define APP_REGISTER_CMD_HANDLERS(ret_var, service_ptr, defs_arr) do { \
    FW_STATIC_ASSERT(FW_ARRAY_SIZE(defs_arr) <= (unsigned long)APP_CMD_SERVICE_MAX_HANDLERS, aegis_app_cmd_handlers_too_many); \
    (ret_var) = aegis_app_cmd_service_register_handlers((service_ptr), (defs_arr), (uint8_t)FW_ARRAY_SIZE(defs_arr)); \
} while (0)

#define APP_QUERY_HANDLER_DEF_INIT(type_val, handler_fn, ctx_val) { (type_val), (handler_fn), (ctx_val) }
#define APP_QUERY_HANDLER_DEF_SET(def_ptr, type_val, handler_fn, ctx_val) do { \
    (def_ptr)->type = (type_val); \
    (def_ptr)->handler = (handler_fn); \
    (def_ptr)->ctx = (ctx_val); \
} while (0)

#define APP_REGISTER_QUERY_HANDLERS(ret_var, dispatcher_ptr, defs_arr) do { \
    FW_STATIC_ASSERT(FW_ARRAY_SIZE(defs_arr) <= (unsigned long)APP_QUERY_MAX_HANDLERS, aegis_app_query_handlers_too_many); \
    (ret_var) = aegis_app_query_register_handlers((dispatcher_ptr), (defs_arr), (uint8_t)FW_ARRAY_SIZE(defs_arr)); \
} while (0)

/* ==================== AegisCommand payload 宏（编译期尺寸约束） ==================== */
#define APP_CMD_PAYLOAD_SET(ret_var, cmd_ptr, payload_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(payload_ptr)) <= (unsigned long)APP_CMD_PAYLOAD_MAX, aegis_app_cmd_payload_too_big); \
    (ret_var) = aegis_app_cmd_payload_write((cmd_ptr), (payload_ptr), (uint16_t)sizeof(*(payload_ptr))); \
} while (0)

#define APP_CMD_PAYLOAD_GET(ret_var, cmd_ptr, out_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(out_ptr)) <= (unsigned long)APP_CMD_PAYLOAD_MAX, aegis_app_cmd_payload_out_too_big); \
    (ret_var) = aegis_app_cmd_payload_read((cmd_ptr), (out_ptr), (uint16_t)sizeof(*(out_ptr))); \
} while (0)

#define APP_CMD_RESULT_PAYLOAD_SET(ret_var, result_ptr, payload_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(payload_ptr)) <= (unsigned long)APP_CMD_RESULT_PAYLOAD_MAX, aegis_app_cmd_result_payload_too_big); \
    (ret_var) = aegis_app_cmd_result_payload_write((result_ptr), (payload_ptr), (uint16_t)sizeof(*(payload_ptr))); \
} while (0)

#define APP_CMD_RESULT_PAYLOAD_GET(ret_var, result_ptr, out_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(out_ptr)) <= (unsigned long)APP_CMD_RESULT_PAYLOAD_MAX, aegis_app_cmd_result_payload_out_too_big); \
    (ret_var) = aegis_app_cmd_result_payload_read((result_ptr), (out_ptr), (uint16_t)sizeof(*(out_ptr))); \
} while (0)

/* ==================== Query payload 宏（编译期尺寸约束） ==================== */
#define APP_QUERY_PAYLOAD_SET(ret_var, req_ptr, payload_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(payload_ptr)) <= (unsigned long)APP_QUERY_PAYLOAD_MAX, aegis_app_query_payload_too_big); \
    (ret_var) = aegis_app_query_payload_write((req_ptr), (payload_ptr), (uint16_t)sizeof(*(payload_ptr))); \
} while (0)

#define APP_QUERY_PAYLOAD_GET(ret_var, req_ptr, out_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(out_ptr)) <= (unsigned long)APP_QUERY_PAYLOAD_MAX, aegis_app_query_payload_out_too_big); \
    (ret_var) = aegis_app_query_payload_read((req_ptr), (out_ptr), (uint16_t)sizeof(*(out_ptr))); \
} while (0)

#define APP_QUERY_RESULT_PAYLOAD_SET(ret_var, resp_ptr, payload_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(payload_ptr)) <= (unsigned long)APP_QUERY_RESULT_PAYLOAD_MAX, aegis_app_query_result_payload_too_big); \
    (ret_var) = aegis_app_query_result_payload_write((resp_ptr), (payload_ptr), (uint16_t)sizeof(*(payload_ptr))); \
} while (0)

#define APP_QUERY_RESULT_PAYLOAD_GET(ret_var, resp_ptr, out_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(out_ptr)) <= (unsigned long)APP_QUERY_RESULT_PAYLOAD_MAX, aegis_app_query_result_payload_out_too_big); \
    (ret_var) = aegis_app_query_result_payload_read((resp_ptr), (out_ptr), (uint16_t)sizeof(*(out_ptr))); \
} while (0)

/* ==================== DTO payload 宏（编译期尺寸约束） ==================== */
#define APP_DTO_PAYLOAD_SET(ret_var, dto_ptr, payload_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(payload_ptr)) <= (unsigned long)APP_DTO_PAYLOAD_MAX, aegis_app_dto_payload_too_big); \
    (ret_var) = aegis_app_dto_payload_write((dto_ptr), (payload_ptr), (uint16_t)sizeof(*(payload_ptr))); \
} while (0)

#define APP_DTO_PAYLOAD_GET(ret_var, dto_ptr, out_ptr) do { \
    FW_STATIC_ASSERT(sizeof(*(out_ptr)) <= (unsigned long)APP_DTO_PAYLOAD_MAX, aegis_app_dto_payload_out_too_big); \
    (ret_var) = aegis_app_dto_payload_read((dto_ptr), (out_ptr), (uint16_t)sizeof(*(out_ptr))); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* APP_MACROS_H */
