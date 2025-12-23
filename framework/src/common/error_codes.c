/*
 * @file: error_codes.c
 * @brief: 错误码转字符串实现
 * @author: jack liu
 */

#include "error_codes.h"

const char* aegis_error_code_to_string(AegisErrorCode code) {
    switch (code) {
        case ERR_OK:                    return "Success";

        /* 通用错误 */
        case ERR_NULL_PTR:              return "Null pointer";
        case ERR_INVALID_PARAM:         return "Invalid parameter";
        case ERR_NOT_INITIALIZED:       return "Not initialized";
        case ERR_ALREADY_INITIALIZED:   return "Already initialized";
        case ERR_OUT_OF_RANGE:          return "Out of range";
        case ERR_TIMEOUT:               return "Timeout";

        /* 内存池错误 */
        case ERR_MEM_POOL_FULL:         return "Memory pool full";
        case ERR_MEM_POOL_INVALID:      return "Invalid memory block";
        case ERR_MEM_POOL_DOUBLE_FREE:  return "Double free";
        case ERR_MEM_SIZE_TOO_LARGE:    return "Memory size too large";

        /* 命令队列错误 */
        case ERR_CMD_QUEUE_FULL:        return "AegisCommand queue full";
        case ERR_CMD_QUEUE_EMPTY:       return "AegisCommand queue empty";
        case ERR_CMD_INVALID_TYPE:      return "Invalid command type";

        /* HAL 层错误 */
        case ERR_HAL_TIMEOUT:           return "HAL timeout";
        case ERR_HAL_BUSY:              return "HAL busy";
        case ERR_HAL_ERROR:             return "HAL error";

        /* 领域层错误 */
        case ERR_DOMAIN_NOT_FOUND:      return "Domain object not found";
        case ERR_DOMAIN_INVALID_STATE:  return "Invalid domain state";
        case ERR_DOMAIN_FULL:           return "Domain storage full";

        /* 应用层错误 */
        case ERR_APP_CMD_FAILED:        return "AegisCommand failed";
        case ERR_APP_QUERY_FAILED:      return "Query failed";

        default:                        return "Unknown error";
    }
}
