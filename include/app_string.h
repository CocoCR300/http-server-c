#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "definition.h"

typedef struct
{
	u8 *	start;
	size_t	length;
} String;

String string_create(u8 * string, size_t length);
bool string_is_null(String string);

