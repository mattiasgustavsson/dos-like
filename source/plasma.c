// Port of plasma tutorial code by Lode Vandevenne 
// https://lodev.org/cgtutor/plasma.html
// See end of file for license
//    /Mattias Gustavsson

#include <math.h>
#include "dos.h"

// Y-coordinate first because we use horizontal scanlines
unsigned int plasma[200][320];

int main(int argc, char *argv[])
{
  setvideomode( videomode_320x200 );
  setdoublebuffer(1);
  int w = 320;
  int h = 200;

  //generate the palette
  for(int x = 0; x < 256; x++)
  {
    int r = (int)(128.0 + 128 * sin(3.1415 * x / 32.0));
    int g = (int)(128.0 + 128 * sin(3.1415 * x / 64.0));
    int b = (int)(128.0 + 128 * sin(3.1415 * x / 128.0));
    setpal( x, r >> 2, g >> 2, b >> 2 );
  } 
  
  //generate the plasma once
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++)
  {
    //the plasma buffer is a sum of sines
    int color = (int)
    (
        128.0 + (128.0 * sin(x / 32.0))
      + 128.0 + (128.0 * sin(y / 16.0))
      + 128.0 + (128.0 * sin((x + y) / 32.0))
      + 128.0 + (128.0 * sin(sqrt((double)(x * x + y * y)) / 16.0))
    ) / 4;
    plasma[y][x] = color;
  }
  
  int paletteShift = 0;

  unsigned char* buffer = screenbuffer();

  //start the animation loop, it rotates the palette
  while(!shuttingdown())
  {
    waitvbl();

    //the parameter to shift the palette varies with time
    paletteShift++;

    //draw every pixel again, with the shifted palette color
    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
    {
      buffer[x + y * 320 ] = (plasma[y][x] + paletteShift) % 256;
    }

    //make everything visible
    buffer = swapbuffers();

    if( keystate( KEY_ESCAPE ) ) {
        break;
    }
  }

  return(0);
}

/*
Lode's Computer Graphics Tutorial
Legal Stuff
This tutorial (including all the separate articles) is Copyright (c) 2004-2007 by Lode Vandevenne. All rights reserved. Do not copy/translate any of the content of this tutorial to a site/book/whatever without my permission.

Some photos are taken from the free photo archive at morguefile.com. These are of course copyright by the authors and contributers of Morguefile. A thank you goes to the photographers!

The source code of QuickCG and all the source code of the examples given in this tutorial and all its articles is released under the following license:

Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
