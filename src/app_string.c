#include "app_string.h"

const String STRING_NULL = { .start = NULL, .length = 0 };

extern inline u8 string_char_at(String * string, s64 index);

extern inline bool string_is_null(String string);

extern inline String string_create(u8 * string, s64 length);

extern inline String string_create_from_static(const char * string);

extern inline String string_ignore_leading_chars(const String * string, s64 count);

extern inline void string_to_c_string(const String * string, char * c_string);
