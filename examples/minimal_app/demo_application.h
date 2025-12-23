/*
 * @file: demo_application.h
 * @brief: 示例应用层（CQRS）- 用例编排与handler注册
 * @author: jack liu
 */

#ifndef DEMO_APPLICATION_H
#define DEMO_APPLICATION_H

#include "types.h"
#include "error_codes.h"
#include "app_command.h"
#include "app_cmd_service.h"
#include "app_query.h"
#include "app_init.h"
#include "demo_domain.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== CQRS 类型定义 ==================== */
/* 命令 */
#define DEMO_CMD_CREATE_CHARGER   ((AegisCommandType)1U)
#define DEMO_CMD_SET_POWER_LEVEL  ((AegisCommandType)2U)

typedef struct {
    uint16_t charger_model;
    uint8_t initial_power_level;
} DemoCreateChargerCmd;

typedef struct {
    uint8_t new_power_level;
} DemoSetPowerCmd;

/* 查询 */
#define DEMO_QUERY_GET_CHARGER ((AegisQueryType)1U)

typedef struct {
    AegisEntityId id;
    uint16_t charger_model;
    uint8_t power_level;
} DemoChargerDto;

typedef struct {
    const AegisDomainRepositoryWriteInterface* repo;
    AegisDomainEventBus* bus;
} DemoUseCaseDeps;

typedef struct {
    DemoUseCaseDeps deps;
} DemoApplicationModule;

/* 注册示例用例处理器（由组合根持有 module 生命周期，避免隐藏全局状态） */
AegisErrorCode demo_application_register(AegisAppRuntime* app, void* ctx);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_APPLICATION_H */
