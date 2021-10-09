#!/bin/sh

gcc source/burn.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/julia.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/mandelbrot.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/plasma.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/raycast.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/rotozoom.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/sound.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/stranded.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/tracker.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/tunnel.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
gcc source/voxel.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
