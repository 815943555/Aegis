/*
 * @file: compile_time.h
 * @brief: 编译期工具宏（断言/数组大小/unused）
 * @author: jack liu
 * @req: REQ-COMMON-110
 * @design: DES-COMMON-110
 * @asil: ASIL-B
 *
 * @note: C89 无 _Static_assert；这里使用 enum + 除零技巧实现编译期断言，
 *        以避免 GCC 的 -Werror=unused-local-typedefs。
 */

#ifndef COMPILE_TIME_H
#define COMPILE_TIME_H

#define FW_UNUSED(x) do { (void)(x); } while (0)

/* 返回元素个数（类型为 sizeof 表达式的整数类型；使用方可按需再转换） */
#define FW_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FW_CONCAT_INNER(a, b) a##b
#define FW_CONCAT(a, b) FW_CONCAT_INNER(a, b)

/* cond 为真 -> 1/1；cond 为假 -> 1/0（编译期错误） */
#define FW_STATIC_ASSERT(cond, msg) \
    enum { FW_CONCAT(static_assert_, FW_CONCAT(msg, __LINE__)) = 1 / (int)(!!(cond)) }

#endif /* COMPILE_TIME_H */
