#include <stdlib.h>
#include <stdbool.h>

typedef struct ring_buf_s {
    void *head;
    void *tail;
    size_t capacity;
    size_t elem_size;
    size_t count;
    void *buffer;
    void *buffer_end;
} ring_buf_t;

void ring_buf_init(ring_buf_t *buf, size_t capacity, size_t elem_size);
void ring_buf_free(ring_buf_t *buf);
bool ring_buf_read(ring_buf_t *buf, void* item);
bool ring_buf_write(ring_buf_t *buf, void* item);
