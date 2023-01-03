#!/bin/bash

set -ex;

SOURCES=( flags.c xpride.c )
CC=${CC:-clang}
LIBS="-lxcb"

CFLAGS="$CFLAGS -Wall -Wextra -pedantic -std=c2x"
BIN=xpride

CFLAGS_DBG="-fsanitize=address,undefined -glldb"
BIN_DBG=xpride.dbg

# Full path sources
FP_SOURCES=()

for file in "${SOURCES[@]}"; do
	FP_SOURCES+=("src/$file");
done

if [[ "$1" = "debug" ]]; then
	$CC $LIBS $CFLAGS $CFLAGS_DBG -o $BIN_DBG ${FP_SOURCES[@]};
else
	$CC $LIBS $CFLAGS -o $BIN ${FP_SOURCES[@]};
fi
