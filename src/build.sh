#!/bin/sh
SRC=test_01.cpp
TARGET=test_01
CXXFLAGS="-I../sunvox_lib/ -Wall -m64"
LDFLAGS=" -lm -ldl"

cp ../sunvox_lib/lib_x86_64/sunvox.so ./
rm -rf $TARGET
g++ $CXXFLAGS $SRC -o $TARGET $LDFLAGS