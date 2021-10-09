#!/bin/sh

clang -o burn.bin source/burn.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o julia.bin source/julia.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o mandelbrot.bin source/mandelbrot.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o plasma.bin source/plasma.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o raycast.bin source/raycast.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o rotozoom.bin source/rotozoom.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o sound.bin source/sound.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o stranded.bin source/stranded.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o tracker.bin source/tracker.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o tunnel.bin source/tunnel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
clang -o voxel.bin source/voxel.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
