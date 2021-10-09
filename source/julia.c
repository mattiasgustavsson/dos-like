// Port of julia fractal tutorial code by Lode Vandevenne 
// https://lodev.org/cgtutor/juliamandelbrot.html#Julia_Set
// See end of file for license
//    /Mattias Gustavsson

#include <math.h>
#include <stdlib.h>
#include "dos.h"


int main(int argc, char *argv[])
{
  setvideomode( videomode_320x200 );
  setdoublebuffer(1);
  for( int i = 0; i < 32; ++i ) setpal( i, 0, 0, 31 - i );  
  int w = 320;
  int h = 200;

  //each iteration, it calculates: new = old*old + c, where c is a constant and old starts at current pixel
  double cRe, cIm;           //real and imaginary part of the constant c, determinate shape of the Julia Set
  double newRe, newIm, oldRe, oldIm;   //real and imaginary parts of new and old
  double zoom = 1, moveX = 0.01101, moveY = 0.0101; //you can change these to zoom and change position
  int maxIterations = 255; //after how much iterations the function should stop

  //pick some values for the constant c, this determines the shape of the Julia Set
  cRe = -0.7;
  cIm = 0.27015;

  double zoomSpd = 0.002f;
  for( ; ; ) {
    //loop through every pixel
    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
    {
      //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
      newRe = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
      newIm = (y - h / 2) / (0.5 * zoom * h) + moveY;
      //i will represent the number of iterations
      int i;
      //start the iteration process
      for(i = 0; i < maxIterations; i++)
      {
        //remember value of previous iteration
        oldRe = newRe;
        oldIm = newIm;
        //the actual iteration, the real and imaginary part are calculated
        newRe = oldRe * oldRe - oldIm * oldIm + cRe;
        newIm = 2 * oldRe * oldIm + cIm;
        //if the point is outside the circle with radius 2: stop
        if((newRe * newRe + newIm * newIm) > 4) break;
        if( keystate( KEY_ESCAPE ) || shuttingdown() ) exit(0);
      }
      //draw the pixel
      putpixel(x, y, ( i + 32 ) & 255 );
    }
    zoom += zoomSpd;
    zoomSpd *= 1.02;
    swapbuffers();
  }

  return 0;
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
