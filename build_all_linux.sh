#!/bin/sh

gcc -o burn.bin source/burn.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o julia.bin source/julia.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o mandelbrot.bin source/mandelbrot.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o plasma.bin source/plasma.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o raycast.bin source/raycast.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o rotozoom.bin source/rotozoom.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o sound.bin source/sound.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o stranded.bin source/stranded.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o tracker.bin source/tracker.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o tunnel.bin source/tunnel.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc -o voxel.bin source/voxel.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
