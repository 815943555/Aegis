/*
 * @file: types.h
 * @brief: C89 基础类型定义，替代 stdint.h
 * @author: jack liu
 * @req: REQ-COMMON-001
 * @design: DES-COMMON-001
 * @asil: ASIL-B
 */

#ifndef TYPES_H
#define TYPES_H

/* 框架统一配置点（可由编译宏覆盖默认值） */
#include "framework_config.h"

/* 防止重复包含 */
#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 基础整数类型 ==================== */
/* 假设目标平台 char=8bit, short=16bit, int=32bit, long=32bit */
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef unsigned int        uint32_t;
typedef signed int          int32_t;
typedef unsigned long       ulong_t;
typedef signed long         long_t;

/* 布尔类型 (C89 无 bool) */
typedef unsigned char       bool_t;
#define TRUE                ((bool_t)1)
#define FALSE               ((bool_t)0)

/* NULL 定义 */
#ifndef NULL
#define NULL                ((void*)0)
#endif

/* ==================== 通用常量 ==================== */
#define MAX_UINT8           ((uint8_t)0xFF)
#define MAX_UINT16          ((uint16_t)0xFFFF)
#define MAX_UINT32          ((uint32_t)0xFFFFFFFF)

#define MIN_INT8            ((int8_t)(-128))
#define MAX_INT8            ((int8_t)127)
#define MIN_INT16           ((int16_t)(-32768))
#define MAX_INT16           ((int16_t)32767)

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
