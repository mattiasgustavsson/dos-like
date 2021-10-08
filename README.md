![build](https://github.com/mattiasgustavsson/dos-like/workflows/build/badge.svg)

# dos-like

Engine for making things with a MS-DOS feel, but for modern platforms

Includes a bunch of examples of classic DOS techniques and effects.

Also available from https://mattiasgustavsson.itch.io/dos-like

## Windows

Tiny C Compiler for windows is included. To build all samples, run build_all.bat.
To build individual samples, do:
```
  tcc\tcc source\stranded.c source\dos.c
```  
where `stranded.c` should be replaced with the sample you like to build.

## Mac

To build on Mac (which I don't test myself, but might work):
```
  clang source/stranded.c source/dos.c -lSDL2 -lGLEW -framework OpenGL
```
where `stranded.c` should be replaced with the sample you like to build.

SDL2 and GLEW are required - if you don't have then installed you can do so by running
```
  brew install sdl2 glew  
```
## Linux

To build on Linux (which I don't test myself, but might work):
```
  gcc source/stranded.c source/dos.c -lSDL2 -lGLEW -lGL -lm -lpthread
```
where `stranded.c` should be replaced with the sample you like to build.

SDL2 and GLEW are required - if you don't have them installed you can do so by running
```
  sudo apt-get install libsdl2-dev
  sudo apt-get install libglew-dev
```
  
  
