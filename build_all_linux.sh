#!/bin/sh

gcc -o burn.out source/burn.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o edit.out source/edit.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o julia.out source/julia.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o mandelbrot.out source/mandelbrot.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o plasma.out source/plasma.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o raycast.out source/raycast.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o rotozoom.out source/rotozoom.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o sound.out source/sound.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o stranded.out source/stranded.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o tracker.out source/tracker.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o tunnel.out source/tunnel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
gcc -o voxel.out source/voxel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread