#!/usr/bin/env bash

mkdir -p build
cmake -S . -B build
cmake --build build

if [ "$1" = "test" ]; then
    build/debug/tests
else
    build/debug/chip8
fi
