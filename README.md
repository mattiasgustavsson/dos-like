![build](https://github.com/mattiasgustavsson/dos-like/workflows/build/badge.svg)

# dos-like

![dos-like examples](dos-like.jpg)

dos-like is a programming library/framework, kind of like a tiny game engine, for writing games and programs
with a similar feel to MS-DOS productions from the early 90s. But rather than writing code that would run on
a real DOS machine, dos-like is about making programs which runs on modern platforms like Windows, Mac and
Linux, but which attempts to recreate the look, feel, and sound of old DOS programs. 

It includes a bunch of examples of classic DOS techniques and effects, a few of which can be seen above.

For more information and screenshots, please see the project home at https://mattiasgustavsson.itch.io/dos-like


## Operating

While running, you can use F11 to toggle between fullscreen and windowed mode.

To start in windowed mode, add the flag -w or --window to the commandline when launching.


## Building

dos-like does not make use of any complicated build systems - a simple command line call to the compiler is
all that is needed to build both your program and thee engine itself.

### Windows

Tiny C Compiler for windows is included. To build all samples, run build_all.bat.
To build individual samples, do:
```
  tcc\tcc source\stranded.c source\dos.c
```  
where `stranded.c` should be replaced with the sample you like to build.

Alternatively, from a Visual Studio Developer Command Prompt, do:
```
  cl source\stranded.c source\dos.c
```  
where `stranded.c` should be replaced with the sample you like to build.

### Mac

To build on Mac (which I don't test myself, but might work):
```
  clang source/stranded.c source/dos.c -lSDL2 -lGLEW -framework OpenGL
```
where `stranded.c` should be replaced with the sample you like to build.

SDL2 and GLEW are required - if you don't have them installed you can do so by running
```
  brew install sdl2 glew  
```
### Linux

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
  
  
