#pragma once

#include <stdbool.h>
#include "definition.h"

typedef struct
{
	u8 *	start;
	size_t	capacity;
	size_t	used;
} Buffer;

static inline bool buffer_full(Buffer * buffer)
{
	return buffer->capacity == buffer->used;
}

u8 buffer_resize(Buffer * buffer, size_t capacity);
size_t buffer_write(Buffer * destination, const u8 * data, size_t length);

