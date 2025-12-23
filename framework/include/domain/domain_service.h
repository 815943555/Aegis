/*
 * @file: aegis_domain_service.h
 * @brief: 领域服务（Domain Service）通用抽象（用例无关的领域能力/策略封装）
 * @author: jack liu
 * @req: REQ-DOMAIN-080
 * @design: DES-DOMAIN-080
 * @asil: ASIL-B
 */

#ifndef DOMAIN_SERVICE_H
#define DOMAIN_SERVICE_H

#include "types.h"
#include "error_codes.h"
#include "domain_entity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t AegisDomainServiceOpType;
#define DOMAIN_SERVICE_OP_INVALID ((AegisDomainServiceOpType)0xFFFFU)

#ifndef DOMAIN_SERVICE_PAYLOAD_MAX
#define DOMAIN_SERVICE_PAYLOAD_MAX 32U
#endif

#ifndef DOMAIN_SERVICE_RESULT_PAYLOAD_MAX
#define DOMAIN_SERVICE_RESULT_PAYLOAD_MAX 16U
#endif

typedef struct {
    AegisDomainServiceOpType op;
    AegisEntityId aggregate_id;
    uint16_t payload_size;
    uint8_t payload[DOMAIN_SERVICE_PAYLOAD_MAX];
} AegisDomainServiceRequest;

typedef struct {
    AegisErrorCode result;
    AegisEntityId created_id;
    uint16_t payload_size;
    uint8_t payload[DOMAIN_SERVICE_RESULT_PAYLOAD_MAX];
} AegisDomainServiceResponse;

typedef AegisErrorCode (*DomainServiceHandler)(const AegisDomainServiceRequest* req,
                                          AegisDomainServiceResponse* resp,
                                          void* ctx);

#ifndef DOMAIN_SERVICE_MAX_HANDLERS
#define DOMAIN_SERVICE_MAX_HANDLERS 16U
#endif

typedef struct {
    AegisDomainServiceOpType op;
    DomainServiceHandler handler;
    void* ctx;
} AegisDomainServiceHandlerEntry;

typedef struct {
    AegisDomainServiceHandlerEntry handlers[DOMAIN_SERVICE_MAX_HANDLERS];
    uint8_t handler_count;
} AegisDomainService;

/*
 * @brief: 初始化领域服务注册表
 * @return: 错误码
 * @req: REQ-DOMAIN-081
 * @design: DES-DOMAIN-081
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_service_init(AegisDomainService* service);

/*
 * @brief: 注册领域服务处理器
 * @param op: 操作类型
 * @param handler: 处理器
 * @param ctx: 业务上下文
 * @return: 错误码
 * @req: REQ-DOMAIN-082
 * @design: DES-DOMAIN-082
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_domain_service_register_handler(AegisDomainService* service,
                                          AegisDomainServiceOpType op,
                                          DomainServiceHandler handler,
                                          void* ctx);

/*
 * @brief: 执行一个领域服务请求（同步分发）
 * @param req: 请求
 * @param resp: 输出响应
 * @return: 错误码
 * @req: REQ-DOMAIN-083
 * @design: DES-DOMAIN-083
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_domain_service_execute(const AegisDomainService* service,
                                 const AegisDomainServiceRequest* req,
                                 AegisDomainServiceResponse* resp);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_SERVICE_H */
