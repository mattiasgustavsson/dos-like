// Port of fire code for Turbo Pascal (DOS) by Frank Jan Sorensen
// I remember being very impressed by this when I first came across it.
// I've done some minor tweaks to the code, but mostly it is just a 
// straight port from the Turbo Pascal version.
//      /Mattias Gustavsson
//---------------------------------------------------------------------

/*

Hi guys, try this, use it in your code, but please credit

Frank Jan Sorensen Alias:Frank Patxi (fjs@lab.jt.dk) for the
fireroutine.

*/

#include "dos.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

int RootRand     =  20;   /* Max/Min decrease of the root of the flames */
int Decay        =   4;   /* How far should the flames go up on the screen? */
int MinY         =   0;   /* Startingline of the flame routine. (should be adjusted along with MinY above) */
int Smooth       =   1;   /* How descrete can the flames be?*/
int MinFire      =  50;   /* limit between the "starting to burn" and the "is burning" routines */
#define XStart      10    /* Startingpos on the screen */
#define XEnd       310    /* Guess! */
int Width        = XEnd-XStart; /*Well- */
int MaxColor     = 110;   /* Constant for the MakePal procedure */
int FireIncrease =   3;   /*3 = Wood, 90 = Gazolin*/

#define PI 3.14159265358979323846f

struct ColorValue {
    int R, G, B;
};
 
void Hsi2Rgb( float H, float S, float I, struct ColorValue* C ) {
/*Convert (Hue, Saturation, Intensity) -> (RGB)*/
  float T;
  float Rv, Gv, Bv;

  T = H;
  Rv = 1.0f + S * (float)sin(T - 2.0f * PI / 3.0f);
  Gv = 1.0f + S * (float)sin(T);
  Bv = 1.0f + S * (float)sin(T + 2.0f * PI / 3.0f);
  T = 63.999f * I / 2.0f;
  C->R = (int)(Rv * T);
  C->G = (int)(Gv * T);
  C->B = (int)(Bv * T);
} /* Hsi2Rgb */


void MakePal( void ) {
  int I;
  struct ColorValue Pal[256];

  memset(Pal,0,sizeof(Pal));
  for(I=1; I <= MaxColor; ++I) {
    Hsi2Rgb(4.6f-1.5f*I/(float)MaxColor,I/(float)MaxColor,I/(float)MaxColor,&Pal[I]);
  }
  for(I=MaxColor; I < 255; ++I) {
    Pal[I]=Pal[I-1];
    if( Pal[I].R<63 ) ++Pal[I].R;
    if( Pal[I].R<63 ) ++Pal[I].R;
    if( ((I % 2)==0) && (Pal[I].G<53) ) ++Pal[I].G;
    if( ((I % 2)==0) && (Pal[I].B<63) ) ++Pal[I].B;
  }

  for(I=0; I<255; ++I ) {
      setpal(I, Pal[I].R, Pal[I].G, Pal[I].B);
  }
}


float RandomF(void) {
    return rand() / (float)( RAND_MAX );
}


int Random(int R) {
    return (int)(RandomF() * R);
}


int Rand(int R) { /* Return a random number between -R And R*/
  return Random(R*2+1)-R;
}


float Sqr( float x ) {
    return x * x;
}

void Help( void ) {
  readkeys();
  setvideomode( videomode_80x25_9x16 ); 
  clrscr();
  gotoxy(0,0);cputs("Burn version 1.00");
  
  gotoxy(0,2);cputs("Light\"n\"play");
  
  gotoxy(0,4);cputs("Keys : ");
  gotoxy(0,5);cputs("<space> : Throw in a match");
  gotoxy(0,6);cputs("<W>     : Water");
  gotoxy(0,7);cputs("<+>     : Increase intensity");
  gotoxy(0,8);cputs("<->     : Decrease intensity");
  gotoxy(0,9);cputs("<C>     : Initialize fire");
  gotoxy(0,10);cputs("<1>..<9>: Burnability (1=Wood, 9=Gaz)");
  gotoxy(0,11);cputs("<?>     : This help");
  
  gotoxy(0,13);cputs("Hit any key kid >");
  while( !*readkeys() ) waitvbl();
  setvideomode( videomode_320x200 ); 
  MakePal();
};

int main( int argc, char* argv[] ) {

  int FlameArray[ 320 ];
  int I,J;
  int X,P;
  int MoreFire;
  int V;
  char Ch;

  Help();
  srand((unsigned)time(NULL));
  setvideomode( videomode_320x200 ); 
  MoreFire=1;
  MakePal();
  unsigned char* Scr = screenbuffer();

  /* Use this if you want to view the palette */
/*  for( I=0; I<255; ++I) {
    for( J=0; J<20;++J ) {
      putpixel(I,J,I);
    }
  }
  while( !*readkeys() ) waitvbl();
*/

  /* Initialize FlameArray */
  for( I=XStart; I<=XEnd; ++I ) {
    FlameArray[I]=0;
  }

  int init = 40;

  memset(Scr,0,screenwidth()*screenheight()); /* Clear Screen */

  while( !shuttingdown() && !keystate( KEY_ESCAPE ) ) {
    waitvbl();
    Ch = *readchars();
 
    /* Put the values from FlameArray on the bottom line of the screen */
    for( I=XStart; I<=XEnd; ++I ) {
        putpixel(I,199,FlameArray[I]);
    }

    /* This loop makes the actual flames */

    for( I=0; I<=319; ++I ) {
      for( J=MinY; J<=199; ++J ) {
        V=getpixel(I,J);
        if( (V==0) ||
           (V<Decay) ||
           (I<0) ||
           (I>319) )
          putpixel(I,J-1,0);
        else
          putpixel(I-(Random(3)-1),(J-1),V-Random(Decay));
      }
    }

    /*Match?*/
    if( (Random(150)==0) || (Ch==' ') || init > 0 ) {
      --init;
      memset(&FlameArray[XStart+Random(Width-5)],255, 5);
    }
    /*In-/Decrease?*/
    if (Ch=='-') if( MoreFire >-2 ) --MoreFire;
    if (Ch=='+') if( MoreFire < 4 ) ++MoreFire;

    /*!!*/
    if( toupper(Ch) == 'C' ) memset(FlameArray,0, sizeof(FlameArray));
    if( toupper(Ch) == 'W' ) {
      for( I=1; I<=10; ++I ) FlameArray[XStart+Random(Width)]=0;
    }

    if( Ch == '?' ) Help();

    if( Ch >= '1' && Ch <= '9' ) FireIncrease=3+(int)Sqr((float)(Ch-'1'));

    /*This loop controls the "root" of the flames ie. the values in FlameArray.*/
    for( I=XStart; I<=XEnd; ++I ) {
      X=FlameArray[I];

      if( X<MinFire ) { /* Increase by the "burnability"*/
        /*Starting to burn:*/
        if( X>10 )  X += Random(FireIncrease);
      } else {
        /* Otherwise randomize and increase by intensity (is burning)*/
        X += Rand(RootRand)+MoreFire;
      }
      if( X>255 ) X=255; /* X Too large ?*/
      FlameArray[I]=X;
    }

    /* Pour a little water on both sides of the fire to make it look nice on the sides*/
    for( I=1; I<=Width/8; ++I) {
      X=(int)(Sqr(RandomF())*Width/8);
      FlameArray[XStart+X]=0;
      FlameArray[XEnd-X]=0;
    }

    /*Smoothen the values of FrameArray to avoid "descrete" flames*/
    P=0;
    for( I=XStart+Smooth; I<=XEnd-Smooth; ++I ) {
      X=0;
      for( J=-Smooth; J<=Smooth; ++J ) X += FlameArray[I+J];
      FlameArray[I]=X / (2*Smooth+1);
    }

  }
  /*Restore video mode*/
  setvideomode( videomode_80x25_9x16 ); 
  /*Good bye*/
  return 0;
}
