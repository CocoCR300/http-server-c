#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "definition.h"

typedef struct
{
	u8 *	start;
	s64		length;
} String;

extern const String STRING_NULL;

// NOTE: Put inline *definitions* in header files, and extern inline *declarations* in c files: https://stackoverflow.com/q/6312597
// Yes, there are no mistakes in the line above.
inline u8 string_char_at(String * string, s64 index) {
	assert(string != NULL && index < string->length);

	// Assume ASCII for now
	return string->start[index];
}

inline bool string_is_null(String string)
{
	return string.start == NULL && string.length == 0;
}

inline String string_create(u8 * string, s64 length)
{
	String new_string = {
		.start = string,
		.length = length
	};
	
	return new_string;
}

inline String string_create_from_static(const char * c_string)
{
	s64 length = strlen(c_string);
	assert(length <= S64_MAX);

	String new_string = {
		.start = c_string,
		.length = length
	};

	return new_string;
}

