// Port of rotozoom code by seancode
// https://seancode.com/demofx/
// See end of file for license
//    /Mattias Gustavsson

#include <math.h>
#include "dos.h"

#define PI 3.14159265358979323846f

int main( int argc, char *argv[] ) {
    setvideomode( videomode_320x200 );
    setdoublebuffer(1);

    unsigned char palette[ 768 ];
    int gifWidth, gifHeight, palcount;
    unsigned char* gif = loadgif( "files/rotozoom.gif", &gifWidth, &gifHeight, &palcount, palette );    

    for( int i = 0; i < palcount; ++i ) {
        setpal(i, palette[ 3 * i + 0 ],palette[ 3 * i + 1 ], palette[ 3 * i + 2 ] );
    }

    unsigned char* buffer = screenbuffer();
    int angle = 0.0f;
    while( !shuttingdown() ) {
        waitvbl();
        float s = (float)sin( angle * PI / 180.0f );
        float c = (float)cos( angle * PI / 180.0f );
        angle = ( angle + 1 ) % 360;
        int destOfs = 0;
        for( int y = 0; y < 200; ++y ) {
            for( int x = 0; x < 320; ++x ) {
                int u = (int)( ( x * c - y * s ) * ( s + 1 ) + 64 ) % gifWidth;
                int v = (int)( ( x * s + y * c ) * ( s + 1 ) + 64 ) % gifHeight;
                if( u < 0 ) u += gifWidth;
                if( v < 0 ) v += gifHeight;                
                int srcOfs = ( u + v * gifWidth );
                buffer[destOfs++] = gif[ srcOfs ];
            }
        }
        buffer = swapbuffers();

        if( keystate( KEY_ESCAPE ) ) break;     
    }

    return 0;
}


/*
Copyright (c) 2021, seancode
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/
