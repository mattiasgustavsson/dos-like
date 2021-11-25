// Port of tunnel tutorial code by Lode Vandevenne 
// https://lodev.org/cgtutor/tunnel.html
// See end of file for license
//    /Mattias Gustavsson

#include <math.h>
#include "dos.h"

#define texWidth 256
#define texHeight 256
#define screenWidth 320
#define screenHeight 200

unsigned char texture[texWidth][texHeight];

//Make the tables twice as big as the screen. The center of the buffers is now the position (w,h).
int distanceTable[2 * screenWidth][2 * screenHeight];
int angleTable[2 * screenWidth][2 * screenHeight];

int main(int argc, char *argv[])
{
  setvideomode( videomode_320x200 );
  setdoublebuffer(1);
  int w = screenWidth;
  int h = screenHeight;

  unsigned char palette[ 768 ];
  int gifWidth, gifHeight, palcount;
  unsigned char* gif = loadgif( "files/tunnel.gif", &gifWidth, &gifHeight, &palcount, palette );    
  for( int y = 0; y < texHeight; ++y )
  for( int x = 0; x < texWidth; ++x ) 
  {
    texture[x][y] = gif[ x + y * gifWidth];
  }
  
  for( int i = 0; i < palcount; ++i ) {
    setpal(i, palette[ 3 * i + 0 ],palette[ 3 * i + 1 ], palette[ 3 * i + 2 ] );
  }

  //generate non-linear transformation table, now for the bigger buffers (twice as big)
  for(int y = 0; y < h * 2; y++)
  for(int x = 0; x < w * 2; x++)
  {
    int angle, distance;
    float ratio = 32.0;
    //these formulas are changed to work with the new center of the tables
    distance = (int)(ratio * texHeight / sqrt((float)((x - w) * (x - w) + (y - h) * (y - h)))) % texHeight;
    angle = (int)(0.5 * texWidth * atan2((float)(y - h), (float)(x - w)) / 3.1416);
    distanceTable[x][y] = distance;
    angleTable[x][y] = angle;
  }

  float animation = 0.0f;
  unsigned char* buffer = screenbuffer();

  //begin the loop
  while(!shuttingdown())
  {
    waitvbl();     
    animation += 1.0f / 60.0f;

    //calculate the shift values out of the animation value
    int shiftX = (int)(texWidth * 1.0 * animation);
    int shiftY = (int)(texHeight * 0.25 * animation);
    //calculate the look values out of the animation value
    //by using sine functions, it'll alternate between looking left/right and up/down
    //make sure that x + shiftLookX never goes outside the dimensions of the table, same for y
    int shiftLookX = w / 2 + (int)(w / 2 * sin(animation));
    int shiftLookY = h / 2 + (int)(h / 2 * sin(animation * 2.0));

    for(int y = 0; y < h; y++)
    for(int x = 0; x < w; x++)
    {
      //get the texel from the texture by using the tables, shifted with the animation variable
      //now, x and y are shifted as well with the "look" animation values
      int color = texture[(unsigned int)(distanceTable[x + shiftLookX][y + shiftLookY] + shiftX)  % texWidth]
                 [(unsigned int)(angleTable[x + shiftLookX][y + shiftLookY]+ shiftY) % texHeight];
      buffer[x + 320*y] = (unsigned char)color;
    }
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
