#include "app_string.h"

String string_create(u8 * string, size_t length)
{
	String new_string = {
		.start = string,
		.length = length
	};
	
	return new_string;
}

bool string_is_null(String string)
{
	return string.start == NULL && string.length == 0;
}

