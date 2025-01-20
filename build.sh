#!/bin/sh

cc -std=c11 -g -I./include -Wall -Wno-pointer-sign -pedantic -fuse-ld=mold -fsanitize=address -static-libsan -o ./bin/program ./src/*.c
