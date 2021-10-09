// Port of example voxel code by Sebastian Macke
// https://github.com/s-macke/VoxelSpace
// See end of the file for original license - I place my changes under the same
//      /Mattias Gustavsson

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include "dos.h"

int main( int argc, char* argv[] ) {
    setvideomode( videomode_320x200 ); 

    uint8_t palette[ 768 ];
    int mapwidth, mapheight, palcount;
    uint8_t* mapcol = loadgif( "files/C1W.gif", &mapwidth, &mapheight, &palcount, palette );    
    uint8_t* mapalt = loadgif( "files/D1.gif", &mapwidth, &mapheight, NULL, NULL );    

    for( int i = 0; i < palcount; ++i ) {
        setpal(i, palette[ 3 * i + 0 ],palette[ 3 * i + 1 ], palette[ 3 * i + 2 ] );
    }
    setpal( 0, 36, 36, 56 );

    struct {
        float x;       // x position on the map
        float y;       // y position on the map
        float height;  // height of the camera
        float angle;   // direction of the camera
        float horizon; // horizon position (look up and down)
        float distance; // distance of map
    } camera = { 512, 800, 78, 0, 100, 800 };

    setdoublebuffer( 1 );
    uint8_t* screen = screenbuffer();

    while( !shuttingdown() ) {
        waitvbl();
        clearscreen();        

        if( keystate( KEY_LEFT ) ) camera.angle += 0.02f;
        if( keystate( KEY_RIGHT ) ) camera.angle -= 0.02f;
        if( keystate( KEY_UP ) ) {
            camera.x -= (float)sin( camera.angle ) * 1.1f;
            camera.y -= (float)cos( camera.angle ) * 1.1f;
        }
        if( keystate( KEY_DOWN ) ) {
            camera.x += (float)sin( camera.angle ) * 0.75f;
            camera.y += (float)cos( camera.angle ) * 0.75f;
        }
        if( keystate( KEY_R ) ) camera.height += 0.5f;
        if( keystate( KEY_F ) ) camera.height -= 0.5f;
        if( keystate( KEY_Q ) ) camera.horizon += 1.5f;
        if( keystate( KEY_W ) ) camera.horizon -= 1.5f;

        int mapwidthperiod = mapwidth - 1;
        int mapheightperiod = mapheight - 1;
        int mapshift = 10;

        // Collision detection. Don't fly below the surface.
        int cameraoffs = ( ( ((int)camera.y) & mapwidthperiod ) << mapshift ) + ( ((int)camera.x) & mapheightperiod );
        if( ( mapalt[ cameraoffs ] + 10.0f ) > camera.height ) camera.height = mapalt[ cameraoffs ] + 10.0f;

        int screenwidth = 320;
        int screenheight = 200;
        float sinang = (float)sin( camera.angle );
        float cosang = (float)cos( camera.angle );

        int hiddeny[ 320 ];
        for( int i = 0; i < screenwidth; ++i )  hiddeny[ i ] = screenheight;
        float deltaz = 1.0f;

        // Draw from front to back
        for( float z = 1.0f; z < camera.distance; z += deltaz ) {
            // 90 degree field of view
            float plx =  -cosang * z - sinang * z;
            float ply =   sinang * z - cosang * z;
            float prx =   cosang * z - sinang * z;
            float pry =  -sinang * z - cosang * z;

            float dx = ( prx - plx ) / screenwidth;
            float dy = ( pry - ply ) / screenwidth;
            plx += camera.x;
            ply += camera.y;
            float invz = 1.0f / z * 100.0f;
            for( int i = 0; i < screenwidth; ++i ) {
                int mapoffset = ( ( ((int)ply) & mapwidthperiod ) << mapshift ) + ( ((int)plx) & mapheightperiod );
                int heightonscreen = (int)( ( camera.height - mapalt[ mapoffset ] ) * invz + camera.horizon );
                if( heightonscreen < 0 ) heightonscreen = 0;
                int col = mapcol[ mapoffset ];
                for( int y = heightonscreen; y < hiddeny[ i ]; ++y ) {
                    screen[ i + y * 320 ] = (uint8_t)col;
                }
                if( heightonscreen < hiddeny[ i ] )  hiddeny[ i ] = heightonscreen; 
                plx += dx;
                ply += dy;
            }
            deltaz += 0.005f;
        }
        
        setcolor( 255 );
        outtextxy( 10, 10, "UP/DOWN/LEFT/RIGHT - move/turn" );
        outtextxy( 10, 18, "R/F - change altitude" );
        outtextxy( 10, 26, "Q/W - change pitch" );

        screen = swapbuffers();

        if( keystate( KEY_ESCAPE ) )  break;
    }

    return 0;
}

/*
MIT License

Copyright (c) 2017 Sebastian Macke

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
