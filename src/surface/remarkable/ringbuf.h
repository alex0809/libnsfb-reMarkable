#ifndef RM_RING_BUF_H
#define RM_RING_BUF_H

#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>

typedef struct ring_buf_s {
	void *head;
	void *tail;
	size_t capacity;
	size_t elem_size;
	sem_t count;
	void *buffer;
	void *buffer_end;
} ring_buf_t;

void ring_buf_init(ring_buf_t *buf, size_t capacity, size_t elem_size);
void ring_buf_free(ring_buf_t *buf);
bool ring_buf_wait(ring_buf_t *buf, void *item, struct timespec *timeout);
bool ring_buf_write(ring_buf_t *buf, void *item);

#endif
