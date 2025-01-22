#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "definition.h"

typedef struct
{
	u8	*	start;
	s64		capacity;
	s64		used;
} Buffer;

void buffer_allocate(Buffer * buffer, s64 capacity);
u8 buffer_read_file(Buffer * buffer, FILE * file, s64 to_read);
u8 buffer_resize(Buffer * buffer, s64 capacity);
s64 buffer_write(Buffer * destination, const u8 * data, s64 length);
s64 buffer_write_count(Buffer * destination, u8 data, s64 count);

inline void buffer_clear(Buffer * buffer)
{
	buffer->used = 0;
}

inline void buffer_free(Buffer * buffer)
{
	free(buffer->start);
	buffer->capacity = buffer->used = 0;
}

inline bool buffer_full(Buffer * buffer)
{
	return buffer->capacity == buffer->used;
}

