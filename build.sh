#!/bin/sh

COMPILER=clang
ROOT_FOLDER=/mnt/data/Projects/http-server-c

if [ -d "/Volumes" ]; then
	ROOT_FOLDER=/Volumes/data/Projects/http-server-c
fi

$COMPILER -std=c11 -g -I$ROOT_FOLDER/include -I/opt/local/include -Wall -Wno-pointer-sign -pedantic -fuse-ld=mold -fsanitize=address -o $ROOT_FOLDER/bin/http_server_c $ROOT_FOLDER/src/*.c -L/opt/local/lib -lmagic

if [ "$1" == "run" ]; then
	$ROOT_FOLDER/bin/http_server_c
fi
