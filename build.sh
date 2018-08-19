#!/bin/bash
emcc bouncy_balls.c -o bouncy_balls.js -O3 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']"