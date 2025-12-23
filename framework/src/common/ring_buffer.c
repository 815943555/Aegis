/*
 * @file: ring_buffer.c
 * @brief: DMA友善的环形缓冲区实现
 * @author: jack liu
 */

#include "ring_buffer.h"
#include "critical.h"
#include <string.h>

/* ==================== 内部辅助宏 ==================== */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_ring_buffer_init(AegisRingBuffer* rb, uint8_t* buffer, uint16_t size) {
    if (rb == NULL || buffer == NULL || size == 0) {
        return ERR_INVALID_PARAM;
    }

    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;

    return ERR_OK;
}

AegisErrorCode aegis_ring_buffer_put(AegisRingBuffer* rb, uint8_t data) {
    if (rb == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    /* 检查是否已满 */
    if (rb->count >= rb->size) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    /* 写入数据 */
    rb->buffer[rb->head] = data;
    rb->head = (uint16_t)((rb->head + 1) % rb->size);
    rb->count++;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_ring_buffer_get(AegisRingBuffer* rb, uint8_t* data) {
    if (rb == NULL || data == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    /* 检查是否为空 */
    if (rb->count == 0) {
        EXIT_CRITICAL();
        return ERR_OUT_OF_RANGE;
    }

    /* 读取数据 */
    *data = rb->buffer[rb->tail];
    rb->tail = (uint16_t)((rb->tail + 1) % rb->size);
    rb->count--;

    EXIT_CRITICAL();

    return ERR_OK;
}

uint16_t aegis_ring_buffer_write(AegisRingBuffer* rb, const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    uint16_t free_space;
    uint16_t to_end;
    uint16_t chunk1;
    uint16_t chunk2;

    if (rb == NULL || data == NULL || len == 0) {
        return 0;
    }

    ENTER_CRITICAL();

    /* 计算可写空间 */
    free_space = rb->size - rb->count;
    written = MIN(len, free_space);

    if (written > 0) {
        /* 计算到缓冲区末尾的距离 */
        to_end = rb->size - rb->head;

        if (written <= to_end) {
            /* 一次拷贝完成 */
            memcpy(&rb->buffer[rb->head], data, written);
        } else {
            /* 分两次拷贝：先到末尾，再从头开始 */
            chunk1 = to_end;
            chunk2 = written - chunk1;
            memcpy(&rb->buffer[rb->head], data, chunk1);
            memcpy(&rb->buffer[0], &data[chunk1], chunk2);
        }

        rb->head = (uint16_t)((rb->head + written) % rb->size);
        rb->count += written;
    }

    EXIT_CRITICAL();

    return written;
}

uint16_t aegis_ring_buffer_read(AegisRingBuffer* rb, uint8_t* data, uint16_t len) {
    uint16_t read_len = 0;
    uint16_t to_end;
    uint16_t chunk1;
    uint16_t chunk2;

    if (rb == NULL || data == NULL || len == 0) {
        return 0;
    }

    ENTER_CRITICAL();

    /* 计算可读数据量 */
    read_len = MIN(len, rb->count);

    if (read_len > 0) {
        /* 计算到缓冲区末尾的距离 */
        to_end = rb->size - rb->tail;

        if (read_len <= to_end) {
            /* 一次拷贝完成 */
            memcpy(data, &rb->buffer[rb->tail], read_len);
        } else {
            /* 分两次拷贝：先到末尾，再从头开始 */
            chunk1 = to_end;
            chunk2 = read_len - chunk1;
            memcpy(data, &rb->buffer[rb->tail], chunk1);
            memcpy(&data[chunk1], &rb->buffer[0], chunk2);
        }

        rb->tail = (uint16_t)((rb->tail + read_len) % rb->size);
        rb->count -= read_len;
    }

    EXIT_CRITICAL();

    return read_len;
}

uint16_t aegis_ring_buffer_get_write_ptr(AegisRingBuffer* rb, uint8_t** ptr) {
    uint16_t free_space;
    uint16_t to_end;
    uint16_t continuous;

    if (rb == NULL || ptr == NULL) {
        return 0;
    }

    ENTER_CRITICAL();

    /* 计算可写空间 */
    free_space = rb->size - rb->count;
    to_end = rb->size - rb->head;

    /* 连续可写空间 = MIN(剩余空间, 到末尾距离) */
    continuous = MIN(free_space, to_end);

    if (continuous > 0) {
        *ptr = &rb->buffer[rb->head];
    } else {
        *ptr = NULL;
    }

    EXIT_CRITICAL();

    return continuous;
}

AegisErrorCode aegis_ring_buffer_commit_write(AegisRingBuffer* rb, uint16_t len) {
    if (rb == NULL) {
        return ERR_NULL_PTR;
    }

    if (len > (rb->size - rb->count)) {
        return ERR_OUT_OF_RANGE;
    }

    ENTER_CRITICAL();

    rb->head = (uint16_t)((rb->head + len) % rb->size);
    rb->count += len;

    EXIT_CRITICAL();

    return ERR_OK;
}

uint16_t aegis_ring_buffer_get_read_ptr(AegisRingBuffer* rb, uint8_t** ptr) {
    uint16_t to_end;
    uint16_t continuous;

    if (rb == NULL || ptr == NULL) {
        return 0;
    }

    ENTER_CRITICAL();

    /* 计算到末尾的距离 */
    to_end = rb->size - rb->tail;

    /* 连续可读空间 = MIN(当前数据量, 到末尾距离) */
    continuous = MIN(rb->count, to_end);

    if (continuous > 0) {
        *ptr = &rb->buffer[rb->tail];
    } else {
        *ptr = NULL;
    }

    EXIT_CRITICAL();

    return continuous;
}

AegisErrorCode aegis_ring_buffer_commit_read(AegisRingBuffer* rb, uint16_t len) {
    if (rb == NULL) {
        return ERR_NULL_PTR;
    }

    if (len > rb->count) {
        return ERR_OUT_OF_RANGE;
    }

    ENTER_CRITICAL();

    rb->tail = (uint16_t)((rb->tail + len) % rb->size);
    rb->count -= len;

    EXIT_CRITICAL();

    return ERR_OK;
}

uint16_t aegis_ring_buffer_get_count(const AegisRingBuffer* rb) {
    uint16_t count;

    if (rb == NULL) {
        return 0;
    }

    ENTER_CRITICAL();
    count = rb->count;
    EXIT_CRITICAL();

    return count;
}

uint16_t aegis_ring_buffer_get_free(const AegisRingBuffer* rb) {
    uint16_t free_space;

    if (rb == NULL) {
        return 0;
    }

    ENTER_CRITICAL();
    free_space = rb->size - rb->count;
    EXIT_CRITICAL();

    return free_space;
}

void aegis_ring_buffer_clear(AegisRingBuffer* rb) {
    if (rb == NULL) {
        return;
    }

    ENTER_CRITICAL();

    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;

    EXIT_CRITICAL();
}
