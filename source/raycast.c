// Port of raycast tutorial code by Lode Vandevenne 
// https://lodev.org/cgtutor/raycasting.html
// See below for license. Note that the graphics used
// in this sample are the same as in the tutorial, and
// they are from the original Wolfenstein 3D, and are
// copyright id Software.
//    /Mattias Gustavsson

/*
Copyright (c) 2004-2020, Lode Vandevenne

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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "dos.h"

// set to 1 to use the horizontal floor algorithm (contributed by Ádám Tóth in 2019),
// or to 0 to use the slower vertical floor algorithm.
#define FLOOR_HORIZONTAL 1

#define screenWidth 320
#define screenHeight 200
#define texWidth 64 // must be power of two
#define texHeight 64 // must be power of two
#define mapWidth 24
#define mapHeight 24

int worldMap[mapWidth][mapHeight] =
{
  {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
  {8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
  {7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6},
  {7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3},
  {2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
  {2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
  {2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};

typedef struct Sprite
{
  double x;
  double y;
  int texture;
} Sprite;

#define numSprites 19

Sprite sprite[numSprites] =
{
  {20.5, 11.5, 10}, //green light in front of playerstart
  //green lights in every room
  {18.5,4.5, 10},
  {10.0,4.5, 10},
  {10.0,12.5,10},
  {3.5, 6.5, 10},
  {3.5, 20.5,10},
  {3.5, 14.5,10},
  {14.5,20.5,10},

  //row of pillars in front of wall: fisheye test
  {18.5, 10.5, 9},
  {18.5, 11.5, 9},
  {18.5, 12.5, 9},

  //some barrels around the map
  {21.5, 1.5, 8},
  {15.5, 1.5, 8},
  {16.0, 1.8, 8},
  {16.2, 1.2, 8},
  {3.5,  2.5, 8},
  {9.5, 15.5, 8},
  {10.0, 15.1,8},
  {10.5, 15.8,8},
};

//1D Zbuffer
double ZBuffer[screenWidth];

//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteDistance[numSprites];

//function used to sort the sprites
void sortSprites(int* order, double* dist, int amount);

double dmax( double a, double b ) { return a > b ? a : b; }
double dmin( double a, double b ) { return a < b ? a : b; }

int main(int argc, char* argv[])
{
  (void) argc, (void) argv;
  double posX = 22.0, posY = 11.5; //x and y start position
  double dirX = -1.0, dirY = 0.0; //initial direction vector
  double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane
  double pitch = 0; // looking up/down, expressed in screen pixels the horizon shifts
  double posZ = 0; // vertical camera strafing up/down, for jumping/crouching. 0 means standard height. Expressed in screen pixels a wall at distance 1 shifts

  uint8_t* texture[11];
  uint8_t* darktexture[11];

  setvideomode( videomode_320x200 ); 
  int w = 320;
  int h = 200;
  setdoublebuffer(1);

  //load some textures
  int tw, th, palcount;
  uint8_t palette[ 768 ];
  texture[0] = loadgif( "files/raycast/eagle.gif", &tw, &th, &palcount, palette );    
  texture[1] = loadgif( "files/raycast/redbrick.gif", &tw, &th, &palcount, palette );    
  texture[2] = loadgif( "files/raycast/purplestone.gif", &tw, &th, &palcount, palette );    
  texture[3] = loadgif( "files/raycast/greystone.gif", &tw, &th, &palcount, palette );    
  texture[4] = loadgif( "files/raycast/bluestone.gif", &tw, &th, &palcount, palette );    
  texture[5] = loadgif( "files/raycast/mossy.gif", &tw, &th, &palcount, palette );    
  texture[6] = loadgif( "files/raycast/wood.gif", &tw, &th, &palcount, palette );    
  texture[7] = loadgif( "files/raycast/colorstone.gif", &tw, &th, &palcount, palette );    

  darktexture[0] = loadgif( "files/raycast/deagle.gif", &tw, &th, &palcount, palette );    
  darktexture[1] = loadgif( "files/raycast/dredbrick.gif", &tw, &th, &palcount, palette );    
  darktexture[2] = loadgif( "files/raycast/dpurplestone.gif", &tw, &th, &palcount, palette );    
  darktexture[3] = loadgif( "files/raycast/dgreystone.gif", &tw, &th, &palcount, palette );    
  darktexture[4] = loadgif( "files/raycast/dbluestone.gif", &tw, &th, &palcount, palette );    
  darktexture[5] = loadgif( "files/raycast/dmossy.gif", &tw, &th, &palcount, palette );    
  darktexture[6] = loadgif( "files/raycast/dwood.gif", &tw, &th, &palcount, palette );    
  darktexture[7] = loadgif( "files/raycast/dcolorstone.gif", &tw, &th, &palcount, palette );    

  //load some sprite textures
  texture[8] = loadgif( "files/raycast/barrel.gif", &tw, &th, &palcount, palette );    
  texture[9] = loadgif( "files/raycast/pillar.gif", &tw, &th, &palcount, palette );    
  texture[10] = loadgif( "files/raycast/greenlight.gif", &tw, &th, &palcount, palette );    

  for( int i = 0; i < palcount; ++i ) {
      setpal(i, palette[ 3 * i + 0 ],palette[ 3 * i + 1 ], palette[ 3 * i + 2 ] );
  }

   uint8_t* buffer = screenbuffer();

  //start the main loop
  while(!shuttingdown())
  {
    waitvbl();
#if FLOOR_HORIZONTAL
    //FLOOR CASTING
    for(int y = 0; y < screenHeight; ++y)
    {
      // whether this section is floor or ceiling
      bool is_floor = y > screenHeight / 2 + pitch;

      // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
      float rayDirX0 = (float)(dirX - planeX);
      float rayDirY0 = (float)(dirY - planeY);
      float rayDirX1 = (float)(dirX + planeX);
      float rayDirY1 = (float)(dirY + planeY);

      // Current y position compared to the center of the screen (the horizon)
      int p = (int)( is_floor ? (y - screenHeight / 2 - pitch) : (screenHeight / 2 - y + pitch) );

      // Vertical position of the camera.
      // NOTE: with 0.5, it's exactly in the center between floor and ceiling,
      // matching also how the walls are being raycasted. For different values
      // than 0.5, a separate loop must be done for ceiling and floor since
      // they're no longer symmetrical.
      float camZ = (float)( is_floor ? (0.5 * screenHeight + posZ) : (0.5 * screenHeight - posZ) );

      // Horizontal distance from the camera to the floor for the current row.
      // 0.5 is the z position exactly in the middle between floor and ceiling.
      // NOTE: this is affine texture mapping, which is not perspective correct
      // except for perfectly horizontal and vertical surfaces like the floor.
      // NOTE: this formula is explained as follows: The camera ray goes through
      // the following two points: the camera itself, which is at a certain
      // height (posZ), and a point in front of the camera (through an imagined
      // vertical plane containing the screen pixels) with horizontal distance
      // 1 from the camera, and vertical position p lower than posZ (posZ - p). When going
      // through that point, the line has vertically traveled by p units and
      // horizontally by 1 unit. To hit the floor, it instead needs to travel by
      // posZ units. It will travel the same ratio horizontally. The ratio was
      // 1 / p for going through the camera plane, so to go posZ times farther
      // to reach the floor, we get that the total horizontal distance is posZ / p.
      float rowDistance = camZ / p;

      // calculate the real world step vector we have to add for each x (parallel to camera plane)
      // adding step by step avoids multiplications with a weight in the inner loop
      float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
      float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

      // real world coordinates of the leftmost column. This will be updated as we step to the right.
      float floorX = (float)( posX + rowDistance * rayDirX0 );
      float floorY = (float)( posY + rowDistance * rayDirY0 );

      for(int x = 0; x < screenWidth; ++x)
      {
        // the cell coord is simply got from the integer parts of floorX and floorY
        int cellX = (int)(floorX);
        int cellY = (int)(floorY);

        // get the texture coordinate from the fractional part
        int tx = (int)(texWidth * (floorX - cellX)) & (texWidth - 1);
        int ty = (int)(texHeight * (floorY - cellY)) & (texHeight - 1);

        floorX += floorStepX;
        floorY += floorStepY;

        // choose texture and draw the pixel
        int checkerBoardPattern = ((int)(cellX + cellY)) & 1;
        int floorTexture;
        if(checkerBoardPattern == 0) floorTexture = 3;
        else floorTexture = 4;
        int ceilingTexture = 6;
        uint32_t color;


        if(is_floor) {
          // floor
          color = darktexture[floorTexture][texWidth * ty + tx];
          //color = (color >> 1) & 8355711; // make a bit darker
          buffer[ x + w * y ] = (uint8_t)color;
        } else {
          //ceiling
          color = darktexture[ceilingTexture][texWidth * ty + tx];
          //color = (color >> 1) & 8355711; // make a bit darker
          buffer[ x + w * y ] = (uint8_t)color;
        }
      }
    }
#endif // FLOOR_HORIZONTAL

    // WALL CASTING
    for(int x = 0; x < w; x++)
    {
      //calculate ray position and direction
      double cameraX = 2 * x / (double)(w) - 1; //x-coordinate in camera space
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;

      //which box of the map we're in
      int mapX = (int)(posX);
      int mapY = (int)(posY);

      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;

      //length of ray from one x or y-side to next x or y-side
      double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
      double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
      double perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side = 0; //was a NS or a EW wall hit?

      //calculate step and initial sideDist
      if(rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }
      if(rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, either in x-direction, or in y-direction
        if(sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if(worldMap[mapX][mapY] > 0) hit = 1;
      }

      //Calculate distance of perpendicular ray (Euclidean distance would give fisheye effect!)
      if(side == 0) perpWallDist = (sideDistX - deltaDistX);
      else          perpWallDist = (sideDistY - deltaDistY);

      //Calculate height of line to draw on screen
      int lineHeight = (int)(h / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = (int)( -lineHeight / 2 + h / 2 + pitch + (posZ / perpWallDist) );
      if(drawStart < 0) drawStart = 0;
      int drawEnd = (int)( lineHeight / 2 + h / 2 + pitch + (posZ / perpWallDist) );
      if(drawEnd >= h) drawEnd = h;
      //texturing calculations
      int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

      //calculate value of wallX
      double wallX; //where exactly the wall was hit
      if(side == 0) wallX = posY + perpWallDist * rayDirY;
      else          wallX = posX + perpWallDist * rayDirX;
      wallX -= floor((wallX));

      //x coordinate on the texture
      int texX = (int)(wallX * (double)(texWidth));
      if(side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      if(side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

      // TODO: an integer-only bresenham or DDA like algorithm could make the texture coordinate stepping faster
      // How much to increase the texture coordinate per screen pixel
      double step = 1.0 * texHeight / lineHeight;
      // Starting texture coordinate
      double texPos = (drawStart - pitch - (posZ / perpWallDist) - h / 2 + lineHeight / 2) * step;
      for(int y = drawStart; y < drawEnd; y++)
      {
        // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
        int texY = (int)texPos & (texHeight - 1);
        texPos += step;
        uint32_t color = (side == 1 ? darktexture : texture )[texNum][texHeight * texY + texX];
        //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        //if(side == 1) color = (color >> 1) & 8355711;
        buffer[ x + w * y ] = (uint8_t)color;
      }

      //SET THE ZBUFFER FOR THE SPRITE CASTING
      ZBuffer[x] = perpWallDist; //perpendicular distance is used

#if !FLOOR_HORIZONTAL
      //FLOOR CASTING
      double floorXWall, floorYWall; //x, y position of the floor texel at the bottom of the wall

      //4 different wall directions possible
      if(side == 0 && rayDirX > 0)
      {
        floorXWall = mapX;
        floorYWall = mapY + wallX;
      }
      else if(side == 0 && rayDirX < 0)
      {
        floorXWall = mapX + 1.0;
        floorYWall = mapY + wallX;
      }
      else if(side == 1 && rayDirY > 0)
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY;
      }
      else
      {
        floorXWall = mapX + wallX;
        floorYWall = mapY + 1.0;
      }

      double distWall, distPlayer, currentDist;

      distWall = perpWallDist;
      distPlayer = 0.0;

      if(drawEnd < 0) drawEnd = h; //becomes < 0 when the integer overflows

      // draw the ceiling from the top of the screen to drawStart
      for(int y = 0; y < drawStart; y++)
      {
        currentDist = (h - (2.0 * posZ)) / (h - 2.0 * (y - pitch));

        double weight = (currentDist - distPlayer) / (distWall - distPlayer);

        // Some variables here are called floor but apply to the ceiling here
        double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
        double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

        int floorTexX, floorTexY;
        floorTexX = (int)(currentFloorX * texWidth) & (texWidth - 1);
        floorTexY = (int)(currentFloorY * texHeight) & (texHeight - 1);

        buffer[ x + w * y ] = darktexture[6][texWidth * floorTexY + floorTexX];
      }

      //draw the floor from drawEnd to the bottom of the screen
      for(int y = drawEnd + 1; y < h; y++)
      {
        currentDist = (h + (2.0 * posZ)) / (2.0 * (y - pitch) - h);

        double weight = (currentDist - distPlayer) / (distWall - distPlayer);

        double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
        double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

        int floorTexX, floorTexY;
        floorTexX = (int)(currentFloorX * texWidth) & (texWidth - 1);
        floorTexY = (int)(currentFloorY * texHeight) & (texHeight - 1);

        int checkerBoardPattern = ((int)currentFloorX + (int)currentFloorY) & 1;
        int floorTexture;
        if(checkerBoardPattern == 0) floorTexture = 3;
        else floorTexture = 4;

        buffer[ x + w * y ] = (darktexture[floorTexture][texWidth * floorTexY + floorTexX] >> 1) & 8355711;
      }
#endif // !FLOOR_HORIZONTAL
    }

    //SPRITE CASTING
    //sort sprites from far to close
    for(int i = 0; i < numSprites; i++)
    {
      spriteOrder[i] = i;
      spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
    }
    sortSprites(spriteOrder, spriteDistance, numSprites);

    //after sorting the sprites, do the projection and draw them
    for(int i = 0; i < numSprites; i++)
    {
      //translate sprite position to relative to camera
      double spriteX = sprite[spriteOrder[i]].x - posX;
      double spriteY = sprite[spriteOrder[i]].y - posY;

      //transform sprite with the inverse camera matrix
      // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
      // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
      // [ planeY   dirY ]                                          [ -planeY  planeX ]

      double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication

      double transformX = invDet * (dirY * spriteX - dirX * spriteY);
      double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D, the distance of sprite to player, matching sqrt(spriteDistance[i])

      int spriteScreenX = (int)((w / 2) * (1 + transformX / transformY));

      //parameters for scaling and moving the sprites
      #define uDiv 1
      #define vDiv 1
      #define vMove 0.0
      int vMoveScreen = (int)( (int)(vMove / transformY) + pitch + posZ / transformY );

      //calculate height of the sprite on screen
      int spriteHeight = abs((int)(h / (transformY))) / vDiv; //using "transformY" instead of the real distance prevents fisheye
      //calculate lowest and highest pixel to fill in current stripe
      int drawStartY = -spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawStartY < 0) drawStartY = 0;
      int drawEndY = spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawEndY >= h) drawEndY = h - 1;

      //calculate width of the sprite
      int spriteWidth = abs( (int)(h / (transformY))) / uDiv;
      int drawStartX = -spriteWidth / 2 + spriteScreenX;
      if(drawStartX < 0) drawStartX = 0;
      int drawEndX = spriteWidth / 2 + spriteScreenX;
      if(drawEndX >= w) drawEndX = w - 1;

      //loop through every vertical stripe of the sprite on screen
      for(int stripe = drawStartX; stripe < drawEndX; stripe++)
      {
        int texX = (int)(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
        //the conditions in the if are:
        //1) it's in front of camera plane so you don't see things behind you
        //2) it's on the screen (left)
        //3) it's on the screen (right)
        //4) ZBuffer, with perpendicular distance
        if(transformY > 0 && stripe > 0 && stripe < w && transformY < ZBuffer[stripe])
        for(int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
        {
          int d = (y-vMoveScreen) * 256 - h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
          int texY = ((d * texHeight) / spriteHeight) / 256;
          uint32_t color = texture[sprite[spriteOrder[i]].texture][texWidth * texY + texX]; //get current color from the texture
          if((color & 0x00FFFFFF) != 0) buffer[ stripe + w * y ] = (uint8_t)color; //paint pixel if it isn't black, black is the invisible color
        }
      }
    }

    buffer = swapbuffers();
    // No need to clear the screen here, since everything is overdrawn with floor and ceiling

    //timing for input and FPS counter
    double frameTime = 1.0 / 60.0; //frametime is the time this frame has taken, in seconds

    //speed modifiers
    double moveSpeed = frameTime * 3.0; //the constant value is in squares/order
    double rotSpeed = frameTime * 2.0; //the constant value is in radians/order

    //move forward if no wall in front of you
    if (keystate(KEY_UP))
    {
      if(worldMap[(int)(posX + dirX * moveSpeed*5)][(int)(posY)] == false) posX += dirX * moveSpeed;
      if(worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed*5)] == false) posY += dirY * moveSpeed;
    }
    //move backwards if no wall behind you
    if(keystate(KEY_DOWN))
    {
      if(worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == false) posX -= dirX * moveSpeed;
      if(worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
    }
    //rotate to the right
    if(keystate(KEY_RIGHT))
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if(keystate(KEY_LEFT))
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }

    // Very simple demonstration jump/pitch controls
    if(keystate(KEY_PRIOR))
    {
      // look up
      pitch += 400 * moveSpeed;
      if(pitch > 200) pitch = 200;
    }
    if(keystate(KEY_NEXT))
    {
      // look down
      pitch -= 400 * moveSpeed;
      if(pitch < -200) pitch = -200;
    }
    if(keystate(KEY_SPACE))
    {
      // jump
      posZ = 50;
    }
    if(keystate(KEY_LSHIFT))
    {
      // crouch
      posZ = -50;
    }
    if(pitch > 0) pitch = dmax(0, pitch - 100 * moveSpeed);
    if(pitch < 0) pitch = dmin(0, pitch + 100 * moveSpeed);
    if(posZ > 0) posZ = dmax(0, posZ - 100 * moveSpeed);
    if(posZ < 0) posZ = dmin(0, posZ + 100 * moveSpeed);

    if(keystate(KEY_ESCAPE)) break;     
  }
  return 0;
}


struct sortspr_t { 
    double dist; 
    int order; 
};


int cmpspr(const void * a, const void * b) {
   struct sortspr_t const* spra = (struct sortspr_t const*)a;
   struct sortspr_t const* sprb = (struct sortspr_t const*)b;
   if( spra->dist < sprb->dist ) return -1;
   else if( spra->dist > sprb->dist ) return 1;
   else return 0;
}


//sort the sprites based on distance
void sortSprites(int* order, double* dist, int amount)
{
  struct sortspr_t sprites[ 256 ];
  for(int i = 0; i < amount; i++) {
    sprites[i].dist = dist[i];
    sprites[i].order = order[i];
  }
  qsort( sprites, amount, sizeof( *sprites ), cmpspr );
  // restore in reverse order to go from farthest to nearest
  for(int i = 0; i < amount; i++) {
    dist[i] = sprites[amount - i - 1].dist;
    order[i] = sprites[amount - i - 1].order;
  }
}
