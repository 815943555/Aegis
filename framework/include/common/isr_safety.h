/*
 * @file: isr_safety.h
 * @brief: 中断安全标记宏定义
 * @author: jack liu
 * @req: REQ-COMMON-004
 * @design: DES-COMMON-004
 * @asil: ASIL-B
 */

#ifndef ISR_SAFETY_H
#define ISR_SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 中断安全标记 ==================== */
/*
 * 这些宏用于标记函数是否可在中断中调用
 * 由 check_isr_safety.py 脚本解析验证
 *
 * ISR_SAFE: 可在中断中调用，临界区代码≤10行
 * ISR_UNSAFE: 禁止在中断中调用
 */

#if defined(__GNUC__)
    /* GCC 使用 section 属性辅助检查 */
    #define ISR_SAFE    __attribute__((section(".isr_safe")))
    #define ISR_UNSAFE  __attribute__((section(".isr_unsafe")))
#elif defined(__KEIL__)
    /* Keil 使用注释标记 */
    #define ISR_SAFE    /* @isr_safe */
    #define ISR_UNSAFE  /* @isr_unsafe */
#elif defined(__IAR__)
    /* IAR 使用注释标记 */
    #define ISR_SAFE    /* @isr_safe */
    #define ISR_UNSAFE  /* @isr_unsafe */
#else
    /* 默认使用注释标记 */
    #define ISR_SAFE    /* @isr_safe */
    #define ISR_UNSAFE  /* @isr_unsafe */
#endif

#ifdef __cplusplus
}
#endif

#endif /* ISR_SAFETY_H */
