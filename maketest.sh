#!/bin/bash

clang++ -std=c++11 -fsanitize=address -fno-omit-frame-pointer -g src/main.cpp example/example.cpp -o test-out