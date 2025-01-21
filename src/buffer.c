#include "buffer.h"

#include <string.h>
#include "error_number.h"

void buffer_allocate(Buffer * buffer, s64 capacity)
{
	buffer->capacity = 0;
	buffer->used = 0;

	buffer->start = malloc(capacity);
	if (buffer->start != NULL) {
		buffer->capacity = capacity;
	}
}

u8 buffer_read_file(Buffer * buffer, FILE * file, s64 to_read)
{
	s64 buffer_left = buffer->capacity - buffer->used;
	if (buffer_left < to_read) {
		s64 new_capacity = buffer->capacity + (to_read - buffer_left);
		u8 result = buffer_resize(buffer, new_capacity);
		if (result != 0) {
			return 1;
		}
	}

	s64 read = fread(buffer->start, SIZE_U8, to_read, file);
	buffer->used += read;
	if (read < to_read) {
		return FILE_READ_ERROR;
	}

	return 0;
}

u8 buffer_resize(Buffer * buffer, s64 capacity)
{
	u8 * new_start = realloc(buffer->start, capacity);
	if (new_start == NULL) {
		return 1;
	}

	buffer->start = new_start;
	buffer->capacity = capacity;
	return 0;
}

s64 buffer_write(Buffer * destination, const u8 * data, s64 length)
{
	s64 * buffer_used = &destination->used;
	s64 buffer_left = destination->capacity - *buffer_used;

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

extern inline void buffer_clear(Buffer * buffer);
extern inline void buffer_free(Buffer * buffer);
extern inline bool buffer_full(Buffer * buffer);

