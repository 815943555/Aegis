/*
 * @file: aegis_domain_repository.h
 * @brief: 领域仓储抽象接口（依赖倒置，具体实现由 port 层提供）
 * @author: jack liu
 * @req: REQ-DOMAIN-010
 * @design: DES-DOMAIN-010
 * @asil: ASIL-B
 */

#ifndef DOMAIN_REPOSITORY_H
#define DOMAIN_REPOSITORY_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 仓储抽象接口（函数指针表）==================== */

/*
 * @brief: 仓储接口 - 定义通用的实体存储抽象操作
 * @note: 遵循依赖倒置原则，Domain 层只依赖抽象接口
 * @note: 仓储只关心"如何存储实体"，不关心"实体的具体业务类型"
 * @note: 具体实现由 port 层提供（如内存存储、Flash 存储等）
 */
typedef struct {
    /*
     * @brief: 初始化仓储
     * @return: 错误码
     * @req: REQ-DOMAIN-011
     * @isr_unsafe
     */
    AegisErrorCode (*init)(void);

    /*
     * @brief: 创建实体（通用接口）
     * @param entity: 实体指针（已分配内存并初始化）
     * @return: 错误码
     * @note: 调用者需要先创建实体对象，仓储只负责存储
     * @req: REQ-DOMAIN-012
     * @isr_unsafe
     */
    AegisErrorCode (*create)(AegisEntityBase* entity);

    /*
     * @brief: 获取实体（通用接口）
     * @param entity_id: 实体ID
     * @param entity: 输出实体指针
     * @return: 错误码
     * @note: 返回的是 AegisEntityBase 指针，调用者需要根据 type 字段转换
     * @req: REQ-DOMAIN-013
     * @isr_safe
     */
    AegisErrorCode (*get)(AegisEntityId entity_id, AegisEntityBase** entity);

    /*
     * @brief: 更新实体（通用接口）
     * @param entity: 实体指针（包含更新后的数据）
     * @return: 错误码
     * @note: 调用者负责修改实体内容，仓储只负责持久化
     * @req: REQ-DOMAIN-014
     * @isr_unsafe
     */
    AegisErrorCode (*update)(AegisEntityBase* entity);

    /*
     * @brief: 删除实体（通用接口）
     * @param entity_id: 实体ID
     * @return: 错误码
     * @req: REQ-DOMAIN-015
     * @isr_unsafe
     */
    AegisErrorCode (*delete_entity)(AegisEntityId entity_id);

    /*
     * @brief: 按类型查找实体
     * @param entity_type: 实体类型
     * @param entities: 输出实体指针数组（由调用者分配）
     * @param max_count: 数组最大容量
     * @param actual_count: 输出实际找到的实体数量
     * @return: 错误码
     * @req: REQ-DOMAIN-016
     * @isr_safe
     */
    AegisErrorCode (*find_by_type)(AegisEntityType entity_type,
                              AegisEntityBase** entities,
                              uint8_t max_count,
                              uint8_t* actual_count);

    /*
     * @brief: 统计指定类型的实体数量
     * @param entity_type: 实体类型
     * @param count: 输出实体数量
     * @return: 错误码
     * @req: REQ-DOMAIN-017
     * @isr_safe
     */
    AegisErrorCode (*count_by_type)(AegisEntityType entity_type, uint8_t* count);

} DomainRepositoryInterface;

/* ==================== 仓储依赖注入接口 ==================== */

/*
 * @brief: 设置仓储实现（依赖注入）
 * @param repo_impl: 仓储具体实现的函数指针表
 * @return: 错误码
 * @note: 必须在使用仓储前调用，由 port 层在初始化时注入
 * @req: REQ-DOMAIN-020
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_repository_set_impl(const DomainRepositoryInterface* repo_impl);

/* ==================== 仓储外观接口（Facade）==================== */
/* 提供统一的调用接口，内部转发到具体实现 */

/*
 * @brief: 初始化仓储
 * @return: 错误码
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_repository_init(void);

/*
 * @brief: 创建实体（通用接口）
 * @param entity: 实体指针
 * @return: 错误码
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_repository_create(AegisEntityBase* entity);

/*
 * @brief: 获取实体（通用接口）
 * @param entity_id: 实体ID
 * @param entity: 输出实体指针
 * @return: 错误码
 * @isr_safe
 */
AegisErrorCode aegis_domain_repository_get(AegisEntityId entity_id, AegisEntityBase** entity);

/*
 * @brief: 更新实体（通用接口）
 * @param entity: 实体指针
 * @return: 错误码
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_repository_update(AegisEntityBase* entity);

/*
 * @brief: 删除实体
 * @param entity_id: 实体ID
 * @return: 错误码
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_repository_delete(AegisEntityId entity_id);

/*
 * @brief: 按类型查找实体
 * @param entity_type: 实体类型
 * @param entities: 输出实体指针数组
 * @param max_count: 数组最大容量
 * @param actual_count: 输出实际找到的实体数量
 * @return: 错误码
 * @isr_safe
 */
AegisErrorCode aegis_domain_repository_find_by_type(AegisEntityType entity_type,
                                         AegisEntityBase** entities,
                                         uint8_t max_count,
                                         uint8_t* actual_count);

/*
 * @brief: 统计指定类型的实体数量
 * @param entity_type: 实体类型
 * @param count: 输出实体数量
 * @return: 错误码
 * @isr_safe
 */
AegisErrorCode aegis_domain_repository_count_by_type(AegisEntityType entity_type, uint8_t* count);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_REPOSITORY_H */
