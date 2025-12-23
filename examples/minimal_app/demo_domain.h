/*
 * @file: demo_domain.h
 * @brief: 示例领域层（充电桩功率控制）- 领域模型与规则
 * @author: jack liu
 */

#ifndef DEMO_DOMAIN_H
#define DEMO_DOMAIN_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"
#include "domain_event.h"
#include "domain_repository_write.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 示例：领域概念 ==================== */
/* 聚合：Charger（充电桩） */
#define DEMO_ENTITY_TYPE_CHARGER ((AegisEntityType)1U)

/* 自定义领域事件：功率变化 */
#define DEMO_EVENT_POWER_LEVEL_CHANGED ((AegisDomainEventType)(DOMAIN_EVENT_USER_BASE + 1U))

typedef struct {
    uint16_t charger_model;   /* 业务字段：型号 */
    uint8_t power_level;      /* 业务字段：功率档位(0-100) */
} DemoChargerState;

typedef struct {
    uint8_t old_power;
    uint8_t new_power;
} DemoPowerChangedEventData;

/* ==================== 领域行为（Domain Service/Factory） ==================== */
AegisErrorCode demo_domain_charger_create(const AegisDomainRepositoryWriteInterface* repo,
                                     AegisDomainEventBus* bus,
                                     uint16_t charger_model,
                                     uint8_t initial_power_level,
                                     AegisEntityId* out_id);

AegisErrorCode demo_domain_charger_set_power_level(const AegisDomainRepositoryWriteInterface* repo,
                                              AegisDomainEventBus* bus,
                                              AegisEntityId charger_id,
                                              uint8_t new_power_level);

AegisErrorCode demo_domain_charger_get(const AegisDomainRepositoryReadInterface* repo,
                                  AegisEntityId charger_id,
                                  DemoChargerState* out_state);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_DOMAIN_H */

