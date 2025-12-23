/*
 * @file: error_codes.h
 * @brief: 全局统一错误码定义
 * @author: jack liu
 * @req: REQ-COMMON-002
 * @design: DES-COMMON-002
 * @asil: ASIL-B
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 错误码枚举 ==================== */
typedef enum {
    ERR_OK                  = 0,    /* 成功 */

    /* 通用错误 (1-99) */
    ERR_NULL_PTR            = 1,    /* 空指针 */
    ERR_INVALID_PARAM       = 2,    /* 非法参数 */
    ERR_NOT_INITIALIZED     = 3,    /* 未初始化 */
    ERR_ALREADY_INITIALIZED = 4,    /* 已初始化 */
    ERR_OUT_OF_RANGE        = 5,    /* 超出范围 */
    ERR_TIMEOUT             = 6,    /* 超时 */
    ERR_BUSY                = 7,    /* 忙 */
    ERR_NOT_FOUND           = 8,    /* 未找到 */
    ERR_EMPTY               = 9,    /* 空（队列/缓冲区） */
    ERR_INVALID_STATE       = 10,   /* 非法状态 */

    /* 内存池错误 (100-199) */
    ERR_MEM_POOL_FULL       = 100,  /* 内存池已满 */
    ERR_MEM_POOL_INVALID    = 101,  /* 非法内存块 */
    ERR_MEM_POOL_DOUBLE_FREE= 102,  /* 重复释放 */
    ERR_MEM_SIZE_TOO_LARGE  = 103,  /* 请求大小超出池容量 */

    /* 命令队列错误 (200-299) */
    ERR_CMD_QUEUE_FULL      = 200,  /* 命令队列已满 */
    ERR_CMD_QUEUE_EMPTY     = 201,  /* 命令队列为空 */
    ERR_CMD_INVALID_TYPE    = 202,  /* 非法命令类型 */

    /* HAL 层错误 (300-399) */
    ERR_HAL_TIMEOUT         = 300,  /* HAL 超时 */
    ERR_HAL_BUSY            = 301,  /* HAL 忙 */
    ERR_HAL_ERROR           = 302,  /* HAL 错误 */

    /* 领域层错误 (400-499) */
    ERR_DOMAIN_NOT_FOUND    = 400,  /* 领域对象未找到 */
    ERR_DOMAIN_INVALID_STATE= 401,  /* 非法状态 */
    ERR_DOMAIN_FULL         = 402,  /* 领域对象存储已满 */

    /* 应用层错误 (500-599) */
    ERR_APP_CMD_FAILED      = 500,  /* 命令执行失败 */
    ERR_APP_QUERY_FAILED    = 501,  /* 查询执行失败 */

    ERR_MAX                 = 999   /* 最大错误码 */
} AegisErrorCode;

/* ==================== 错误码转字符串 ==================== */
/*
 * @brief: 将错误码转换为字符串描述
 * @param code: 错误码
 * @return: 错误码对应的字符串
 * @req: REQ-COMMON-003
 * @design: DES-COMMON-003
 * @asil: ASIL-B
 * @isr_safe
 */
const char* aegis_error_code_to_string(AegisErrorCode code);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_CODES_H */
