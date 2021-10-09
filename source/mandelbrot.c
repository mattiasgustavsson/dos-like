// Port of mandelbrot tutorial code by Lode Vandevenne 
// https://lodev.org/cgtutor/juliamandelbrot.html#Mandelbrot_Set_
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

  //each iteration, it calculates: newz = oldz*oldz + p, where p is the current pixel, and oldz stars at the origin
  double pr, pi;           //real and imaginary part of the pixel p
  double newRe, newIm, oldRe, oldIm;   //real and imaginary parts of new and old z
  double zoom = 1, moveX = -0.5, moveY = 0; //you can change these to zoom and change position
  int maxIterations = 255;//after how much iterations the function should stop
  double zoomSpd = 0.005f;
  for( ; ; ) {
    //loop through every pixel
    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
    {
      //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
      pr = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
      pi = (y - h / 2) / (0.5 * zoom * h) + moveY;
      newRe = newIm = oldRe = oldIm = 0; //these should start at 0,0
      //"i" will represent the number of iterations
      int i;
      //start the iteration process
      for(i = 0; i < maxIterations; i++)
      {
        //remember value of previous iteration
        oldRe = newRe;
        oldIm = newIm;
        //the actual iteration, the real and imaginary part are calculated
        newRe = oldRe * oldRe - oldIm * oldIm + pr;
        newIm = 2 * oldRe * oldIm + pi;
        //if the point is outside the circle with radius 2: stop
        if((newRe * newRe + newIm * newIm) > 4) break;
      }
      //draw the pixel
      putpixel(x, y, ( i + 32 ) & 255 );
      if( keystate( KEY_ESCAPE ) || shuttingdown() ) exit(0);
    }
    swapbuffers();
    zoom += zoomSpd;
    moveX -= 0.0050109f / zoom;
    zoomSpd *= 1.005;
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
