#pragma once 

#include <limits.h>
#include <stdint.h>
#include <sys/socket.h>

#define ARRAY_COUNT(array) sizeof(array) / sizeof(array[0])

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int32_t s32;
typedef int64_t s64;

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

extern const s64 SIZE_U8;

extern const s64 S64_MAX;
