![build](https://github.com/mattiasgustavsson/dos-like/workflows/build/badge.svg)

# MS-DOS Like

![MS-DOS Like examples](dos-like.jpg)

MS-DOS Like is a programming library/framework, kind of like a tiny game engine, for writing games and programs with a similar feel to MS-DOS productions from the early 90s. But rather than writing code that would run on a real MS-DOS machine, MS-DOS Like is about making programs which run on modern platforms like Windows, Mac and Linux, but which attempts to recreate the look, feel, and sound of old MS-DOS programs. 

It includes a bunch of examples of classic MS-DOS techniques and effects, a few of which can be seen above.

For more information and screenshots, please see the project home at https://mattiasgustavsson.itch.io/dos-like


## Operating

While running, you can use F11 to toggle between fullscreen and windowed mode.

To start in windowed mode, add the parameter `-w` or `--window` to the command line when launching.


## Building

MS-DOS Like does not make use of any complicated build systems - a simple command line call to the compiler is
all that is needed to build both your program and the engine itself.


### Windows

Tiny C Compiler for Windows is included. To build all samples, run `build_all.bat`.
To build individual samples, do:
```
  tcc\tcc source\stranded.c source\dos.c
```  
where `stranded.c` should be replaced with the sample you would like to build.

Alternatively, from a Visual Studio Developer Command Prompt, do:
```
  cl source\stranded.c source\dos.c
```  
where `stranded.c` should be replaced with the sample you would like to build.


### Mac

To build all samples on Mac, run `build_all_macos.sh`.
To build individual samples, do:
```
  clang source/stranded.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -framework OpenGL -lpthread
```
where `stranded.c` should be replaced with the sample you would like to build.

SDL2 and GLEW are required - if you don't have them installed you can do so with Homebrew by running:
```
  brew install sdl2 glew  
```
where SDL2 and GLEW gets installed using Homebrew, or you can use `install_sdl2glew_macos.sh` to install GLEW and SDL2.


### Linux

To build all samples on Linux, run `build_all_linux.sh`.
To build individual samples, do:
```
  gcc source/stranded.c source/dos.c `sdl2-config --libs --cflags` -lGLEW -lGL -lm -lpthread
```
where `stranded.c` should be replaced with the sample you would like to build.

SDL2 and GLEW are required - if you don't have them installed you can do so on Ubuntu (or wherever `apt-get` or `apt` is available) by running:
```
  sudo apt-get install libsdl2-dev
  sudo apt-get install libglew-dev
```
or:
```
  sudo apt install libsdl2-dev
  sudo apt install libglew-dev
```
where SDL2 and GLEW gets installed using `apt-get` or `apt`.
