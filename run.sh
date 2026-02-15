#!/usr/bin/env bash

mkdir -p build
cmake -S . -B build
cmake --build build
build/debug/chip8
