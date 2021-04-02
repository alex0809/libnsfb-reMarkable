#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ringbuf.h"
#include "log.h"

void ring_buf_init(ring_buf_t *buf, size_t capacity, size_t elem_size)
{
    buf->buffer = malloc(capacity * elem_size);
    buf->buffer_end = (char *)buf->buffer + capacity * elem_size;
    buf->capacity = capacity;
    buf->elem_size = elem_size;
    buf->count = 0;
    buf->head = buf->buffer;
    buf->tail = buf->buffer;
}

void ring_buf_free(ring_buf_t *buf)
{
    free(buf->buffer);
}

bool ring_buf_write(ring_buf_t *buf, void* item) {
    if (buf->elem_size == buf->capacity) {
        ERROR_LOG("Buffer is full. Skipping event.");
        return false; 
    }

    memcpy(buf->head, item, buf->elem_size);
    buf->head = (char*)buf->head + buf->elem_size;
    if (buf->head == buf->buffer_end) {
        buf->head = buf->buffer;
    }
    buf->count++;
    return true;
}

bool ring_buf_read(ring_buf_t *buf, void *item) {
    if (buf->count == 0) {
        return false;
    }
    memcpy(item, buf->tail, buf->elem_size);
    buf->tail = (char*)buf->tail + buf->elem_size;
    if (buf->tail == buf->buffer_end) {
        buf->tail = buf->buffer;
    }
    buf->count--;
    return true;
}

