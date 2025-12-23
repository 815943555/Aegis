/*
 * @file: ring_buffer.h
 * @brief: DMA友善的环形缓冲区接口
 * @author: jack liu
 * @req: REQ-COMMON-006
 * @design: DES-COMMON-006
 * @asil: ASIL-B
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 环形缓冲区配置 ==================== */
/* DMA友善设计：缓冲区地址对齐，适合DMA直接访问 */

#ifndef RING_BUFFER_SIZE
#define RING_BUFFER_SIZE  256  /* 缓冲区大小（字节），建议2的幂次 */
#endif

/* ==================== 环形缓冲区结构 ==================== */
typedef struct {
    uint8_t* buffer;        /* 缓冲区指针（DMA可直接访问） */
    uint16_t size;          /* 缓冲区总大小 */
    uint16_t head;          /* 写入位置（生产者） */
    uint16_t tail;          /* 读取位置（消费者） */
    uint16_t count;         /* 当前数据量 */
} AegisRingBuffer;

/* ==================== 环形缓冲区接口 ==================== */
/*
 * @brief: 初始化环形缓冲区
 * @param rb: 环形缓冲区指针
 * @param buffer: 静态缓冲区指针（必须对齐）
 * @param size: 缓冲区大小
 * @return: 错误码
 * @req: REQ-RING-001
 * @design: DES-RING-001
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_ring_buffer_init(AegisRingBuffer* rb, uint8_t* buffer, uint16_t size);

/*
 * @brief: 写入单字节（ISR安全）
 * @param rb: 环形缓冲区指针
 * @param data: 待写入字节
 * @return: 错误码
 * @req: REQ-RING-002
 * @design: DES-RING-002
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_ring_buffer_put(AegisRingBuffer* rb, uint8_t data);

/*
 * @brief: 读取单字节（ISR安全）
 * @param rb: 环形缓冲区指针
 * @param data: 输出字节
 * @return: 错误码
 * @req: REQ-RING-003
 * @design: DES-RING-003
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_ring_buffer_get(AegisRingBuffer* rb, uint8_t* data);

/*
 * @brief: 批量写入（DMA友善）
 * @param rb: 环形缓冲区指针
 * @param data: 待写入数据
 * @param len: 数据长度
 * @return: 实际写入长度
 * @req: REQ-RING-004
 * @design: DES-RING-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
uint16_t aegis_ring_buffer_write(AegisRingBuffer* rb, const uint8_t* data, uint16_t len);

/*
 * @brief: 批量读取（DMA友善）
 * @param rb: 环形缓冲区指针
 * @param data: 输出缓冲区
 * @param len: 期望读取长度
 * @return: 实际读取长度
 * @req: REQ-RING-005
 * @design: DES-RING-005
 * @asil: ASIL-B
 * @isr_unsafe
 */
uint16_t aegis_ring_buffer_read(AegisRingBuffer* rb, uint8_t* data, uint16_t len);

/*
 * @brief: 获取连续可写空间（DMA使用）
 * @param rb: 环形缓冲区指针
 * @param ptr: 输出缓冲区指针
 * @return: 连续可写长度
 * @req: REQ-RING-006
 * @design: DES-RING-006
 * @asil: ASIL-B
 * @isr_safe
 */
uint16_t aegis_ring_buffer_get_write_ptr(AegisRingBuffer* rb, uint8_t** ptr);

/*
 * @brief: 提交写入（DMA完成后调用）
 * @param rb: 环形缓冲区指针
 * @param len: 已写入长度
 * @return: 错误码
 * @req: REQ-RING-007
 * @design: DES-RING-007
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_ring_buffer_commit_write(AegisRingBuffer* rb, uint16_t len);

/*
 * @brief: 获取连续可读空间（DMA使用）
 * @param rb: 环形缓冲区指针
 * @param ptr: 输出缓冲区指针
 * @return: 连续可读长度
 * @req: REQ-RING-008
 * @design: DES-RING-008
 * @asil: ASIL-B
 * @isr_safe
 */
uint16_t aegis_ring_buffer_get_read_ptr(AegisRingBuffer* rb, uint8_t** ptr);

/*
 * @brief: 提交读取（DMA完成后调用）
 * @param rb: 环形缓冲区指针
 * @param len: 已读取长度
 * @return: 错误码
 * @req: REQ-RING-009
 * @design: DES-RING-009
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_ring_buffer_commit_read(AegisRingBuffer* rb, uint16_t len);

/*
 * @brief: 获取当前数据量
 * @param rb: 环形缓冲区指针
 * @return: 当前数据量（字节）
 * @req: REQ-RING-010
 * @design: DES-RING-010
 * @asil: ASIL-B
 * @isr_safe
 */
uint16_t aegis_ring_buffer_get_count(const AegisRingBuffer* rb);

/*
 * @brief: 获取剩余空间
 * @param rb: 环形缓冲区指针
 * @return: 剩余空间（字节）
 * @req: REQ-RING-011
 * @design: DES-RING-011
 * @asil: ASIL-B
 * @isr_safe
 */
uint16_t aegis_ring_buffer_get_free(const AegisRingBuffer* rb);

/*
 * @brief: 清空缓冲区
 * @param rb: 环形缓冲区指针
 * @req: REQ-RING-012
 * @design: DES-RING-012
 * @asil: ASIL-B
 * @isr_unsafe
 */
void aegis_ring_buffer_clear(AegisRingBuffer* rb);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H */
