#!/bin/sh

clang -o burn.out source/burn.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o edit.out source/edit.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o julia.out source/julia.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o mandelbrot.out source/mandelbrot.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o plasma.out source/plasma.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o raycast.out source/raycast.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o rotozoom.out source/rotozoom.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o sound.out source/sound.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o stranded.out source/stranded.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o tracker.out source/tracker.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o tunnel.out source/tunnel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o voxel.out source/voxel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
