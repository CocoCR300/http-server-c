#include "buffer.h"

#include <stdlib.h>
#include <string.h>

u8 buffer_resize(Buffer * buffer, size_t capacity)
{
	u8 * new_start = realloc(buffer->start, capacity);
	if (new_start == NULL) {
		return 1;
	}

	buffer->start = new_start;
	buffer->capacity = capacity;
	return 0;
}

size_t buffer_write(Buffer * destination, const u8 * data, size_t length)
{
	size_t * buffer_used = &destination->used;
	size_t buffer_left = destination->capacity - *buffer_used;

	size_t write = buffer_left < length ? buffer_left : length;
	if (buffer_left < length) {
		write = buffer_left;
	}
	else {
		write = length;
	}

	memcpy(destination->start, data, write);
	*buffer_used += write;

	return length - write;
}

