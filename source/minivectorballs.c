//
// Created by nzy on 06.08.2024.
// Port of minivectorballs
// https://github.com/mtuomi/SecondReality
// See end of file for license
// Constantine Zykov <iweb@inbox.ru>
//

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "dos.h"

//#define M_PI 3.14159265358979323846f

void update_dots();

//;x,y,z,oldposshadow,oldpos,-,-,-
struct dot {
    int16_t   x;
    int16_t   y;
    int16_t   z;
    uint16_t     oldposshadow;
    uint16_t     oldpos;
    // int     old3;
    // int     old4;
    uint16_t   yadd;

    uint8_t    index_color;

    uint16_t    posshadow;
    uint16_t    pos;

    bool visible;
    bool shadow_visible;
};

#define MAXDOTS 1024
#define BOTTOM 8000

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

// int timer=30000;
int16_t dropper;//,repeat;
uint16_t frame=0;
int16_t rota=-1*64;
// int fb=0;
int16_t rot=0,rots=0;
int16_t a,b,c,d,i,j;//,mode;
int16_t grav,gravd;
int16_t f=0;
// uint16_t bpmin = 30000;
// uint16_t bpmax = -30000;

int16_t dottaul[1024];

struct dot dots[MAXDOTS] = {0};

float rotsin, rotcos;
int16_t gravitybottom = BOTTOM;
int16_t gravity = 0;
uint16_t dotnum;
int16_t gravityd = 16;

uint16_t rows[SCREEN_HEIGHT];
uint32_t depthtable1[128];
uint32_t depthtable2[128];
uint32_t depthtable3[128];
// int depthtable4[128];

uint8_t *bgpic;
uint8_t* vram;

int16_t sin1024[1024];

uint16_t cols[]={
    0,0,0,
    4,25,30,
    8,40,45,
    16,55,60
};

uint8_t pal[256 * 3];
uint8_t pal2[256 * 3];

int16_t isin(int deg)
{
    // return sin(M_PI * deg / 512) * 255;
    return(sin1024[deg&1023]);
}

int16_t icos(int deg)
{
    // return cos(M_PI * deg / 512) * 255;
    return(sin1024[(deg+256)&1023]);
}


void setup_dots()
{
    dotnum=512;
    for(a=0;a<dotnum;a++)  dottaul[a]=a;
    for(a=0;a<500;a++)
    {
        b=rand()%dotnum;
        c=rand()%dotnum;
        d=dottaul[b];
        dottaul[b]=dottaul[c];
        dottaul[c]=d;
    }
    {
        dropper=22000;
        for(a=0;a<dotnum;a++)
        {
            dots[a].x=0;
            dots[a].y=2560-dropper;
            dots[a].z=0;
            dots[a].yadd=0;
        }
        // mode=7;
        grav=3;
        gravd=13;
        gravitybottom=8105;
        i=-1;
    }
    for(a=0;a<500;a++)
    { // scramble
        b=rand()%dotnum;
        c=rand()%dotnum;
        d=dots[b].x; dots[b].x=dots[c].x; dots[c].x=d;
        d=dots[b].y; dots[b].y=dots[c].y; dots[c].y=d;
        d=dots[b].z; dots[b].z=dots[c].z; dots[c].z=d;
    }
    for(a=0;a<SCREEN_HEIGHT;a++) rows[a] = a * SCREEN_WIDTH;

    for(a=0;a<128;a++)
    {
        c = a - (43 + 20) / 2;
        c = c * 3 / 4;
        c += 8;
        if(c<0) c=0; else if(c>15) c=15;
        c = 15 - c;
        depthtable1[a] = 0x202 + 0x04040404 * c;
        depthtable2[a] = 0x02030302 + 0x04040404 * c;
        depthtable3[a] = 0x202 + 0x04040404 * c;
        //depthtable4[a]=0x02020302+0x04040404*c;
    }
}

void setup_pal()
{

    for (int i = 0; i < 256; i++) {
        getpal(i, (int*)&pal[3 * i + 0], (int*)&pal[3 * i + 1], (int*)&pal[3 * i + 2]);
    }

    // outp(0x3c8,0);
    uint8_t i = 0;
    for(a = 0; a < 16; a++)
    {
        for(b = 0; b < 4; b++)
        {
            c = 100 + a * 9;

            pal[3 * i + 0] = cols[b * 3 + 0];
            pal[3 * i + 1] = cols[b * 3 + 1] * c / 256;
            pal[3 * i + 2] = cols[b * 3 + 2] * c / 256;
            i++;
        }
    }

    pal[3 * 255 + 0] = 31;
    pal[3 * 255 + 1] = 0;
    pal[3 * 255 + 2] = 15;

    // outp(0x3c8,64);
    for(a = 0; a < 100; a++)
    {
        c = 64 - 256 / (a + 4);
        c = c * c / 64;

        pal[3 * (64 + a) + 0] = c / 4;
        pal[3 * (64 + a) + 1] = c / 4;
        pal[3 * (64 + a) + 2] = c / 4;
    }
}

void draw_dot(struct dot *dot)
{
    //shadow
    if (dot->shadow_visible)
    {
        uint16_t c = 87 + 87 * 256;
        memcpy(vram + dot->posshadow, &c, 2);
    }

    if (dot->visible)
    {
        memcpy(vram + dot->pos + 1, (uint8_t*)depthtable1 + dot->index_color, 2);
        memcpy(vram + dot->pos + 320, (uint8_t*)depthtable2 + dot->index_color, 4);
        memcpy(vram + dot->pos + 641, (uint8_t*)depthtable3 + dot->index_color, 2);
    }
}

void draw_dots()
{
    for (int i = 0; i < dotnum; i++) {
        draw_dot(&dots[i]);
    }
}

void draw_floor()
{
    for(uint8_t a = 0; a < 100; a++)
    {
        // memset(vram+(100+a)*320,a+64,320);
        hline(0, 100 + a, SCREEN_WIDTH, a + 64);
    }
}

void fadein()
{
    for(b=64;b>=0;b--)
	{
		for(c=0;c<768;c++)
		{
			a=pal[c]-b;
			if(a<0) a=0;
			pal2[c]=a;
		}

        waitvbl();
        waitvbl();
        for (int i = 0; i < 256; i++) {
            setpal(i, pal2[3 * i + 0], pal2[3 * i + 1], pal2[3 * i + 2]);
        }
	}
}

void update_gravity(struct dot *dot)
{
    dot->visible = false;
    dot->shadow_visible = false;
    //рассчитываем z
    float bp =  ((dot->z * rotcos - dot->x * rotsin) / 0x10000) + 9000;
    //рассчитываем x
    float t = (dot->z * rotsin + dot->x * rotcos) / 0x100;

    float ax = (t + t / 8) / bp + 160;
    if (ax >= 0 && ax < SCREEN_WIDTH) {
        //shadow
        uint8_t shadow = (0x80000 / bp) + 100;
        if (shadow >= 0 && shadow < SCREEN_HEIGHT) {
            dot->posshadow = rows[shadow] + ax;
            dot->shadow_visible = true;

            //ball
            dot->yadd += gravity;
            int16_t b = dot->y + dot->yadd;
            if (b >= gravitybottom) {
                dot->yadd = (-dot->yadd * gravityd) / 0x10;
                b += dot->yadd;
            }

            dot->y = b;

            int16_t b_ = (b << 6) / bp + 100;
            if (b_ >= 0 && b_ < SCREEN_HEIGHT) {
                c = rows[b_] + ax;
                int8_t ic = ((int16_t)bp >> 6) & ~3;

                dot->pos = c;
                dot->index_color = ic;

                dot->visible = true;
            }
        }
    }
}

void update_dots()
{
    if(frame==500) f=0;
    i=dottaul[j];
    j++; j%=dotnum;
    if(frame<500)
    {
        dots[i].x=isin(f*11)*40;
        dots[i].y=icos(f*13)*10-dropper;
        dots[i].z=isin(f*17)*40;
        dots[i].yadd=0;
    }
    else if(frame<900)
    {
        dots[i].x=icos(f*15)*55;
        dots[i].y=dropper;
        dots[i].z=isin(f*15)*55;
        dots[i].yadd=-260;
    }
    else if(frame<1700)
    {
        a=sin1024[frame&1023]/8;
        dots[i].x=icos(f*66)*a;
        dots[i].y=BOTTOM;
        dots[i].z=isin(f*66)*a;
        dots[i].yadd=-300;
    }
    else if(frame<2360)
    {
        /*
        a=rand()/128+32;
        dot[i].y=8000-a*80;
        b=rand()&1023;
        a+=rand()&31;
        dot[i].x=sin1024[b]*a/3+(a-50)*7;
        dot[i].z=sin1024[(b+256)&1023]*a/3+(a-40)*7;
        dot[i].yadd=300;
        if(frame>1640 && !(frame&31) && grav>-2) grav--;
        */
        dots[i].x=rand()-16384;
        dots[i].y=BOTTOM-rand()/2;
        dots[i].z=rand()-16384;
        dots[i].yadd=0;
        if(frame>1900 && !(frame&31) && grav>0) grav--;
    }
    else if(frame<2400)
    {
        a=frame-2360;
        for(b=0;b<768;b+=3)
        {
            c=pal[b+0]+a*3;
            if(c>63) c=63;
            pal2[b+0]=c;
            c=pal[b+1]+a*3;
            if(c>63) c=63;
            pal2[b+1]=c;
            c=pal[b+2]+a*4;
            if(c>63) c=63;
            pal2[b+2]=c;
        }
    }
    else if(frame<2440)
    {
        a=frame-2400;
        for(b=0;b<768;b+=3)
        {
            c=63-a*2;
            if(c<0) c=0;
            pal2[b+0]=c;
            pal2[b+1]=c;
            pal2[b+2]=c;
        }
    }
    if(dropper>4000) dropper-=100;
    rotcos=icos(rot)*64; rotsin=isin(rot)*64;
    rots+=2;
    if(frame>1900)
    {
        rot+=rota/64;
        rota--;
    }
    else rot=isin(rots);
    f++;
    gravity=grav;
    gravityd=gravd;

    for (uint16_t i = 0; i < dotnum; i++) {
        update_gravity(&dots[i]);
    }
}

int adjust_framerate()
{
    return 1; // у fc сделано так (на временной основе)
}

void setpalette(uint8_t *pal)
{
    for (int i = 0; i < 256; i++) {
        setpal(i, pal[3 * i + 0], pal[3 * i + 1], pal[3 * i + 2]);
    }
}

int main( int argc, char *argv[] )
{
    srand(0xfcfc);
    setvideomode( videomode_320x200 );

    vram = screenbuffer();

    setup_dots();
    setup_pal();

    for (int i = 0; i < 256; i++) {
        setpal(i, 0, 0, 0);
    }
    draw_floor();
    fadein();

    bgpic = (uint8_t*)(malloc(SCREEN_WIDTH * SCREEN_HEIGHT));
    memcpy(bgpic, vram,SCREEN_WIDTH * SCREEN_HEIGHT);

    while( frame < 2450 ) {

        if(frame>2300) setpalette(pal2);

        waitvbl();

        if( keystate( KEY_SPACE ) )
            continue;

        frame ++;

        clearscreen();
        draw_floor();

        for (int num = adjust_framerate(); num; num--) {
            update_dots();
        }

        draw_dots();

        vram = swapbuffers();

        if( keystate( KEY_ESCAPE ) )
            break;

    }

    return 0;
}

int16_t sin1024[1024] = {
	0,1,3,4,6,7,9,10,12,14,15,17,18,20,21,23,25,26,28,29,31,32,34,36,37,39,40,42,43,45,46,48,
	49,51,53,54,56,57,59,60,62,63,65,66,68,69,71,72,74,75,77,78,80,81,83,84,86,87,89,90,92,93,95,96,
	97,99,100,102,103,105,106,108,109,110,112,113,115,116,117,119,120,122,123,124,126,127,128,130,131,132,134,135,136,138,139,140,
	142,143,144,146,147,148,149,151,152,153,155,156,157,158,159,161,162,163,164,166,167,168,169,170,171,173,174,175,176,177,178,179,
	181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,211,
	212,213,214,215,216,217,217,218,219,220,221,221,222,223,224,225,225,226,227,227,228,229,230,230,231,232,232,233,234,234,235,235,
	236,237,237,238,238,239,239,240,241,241,242,242,243,243,244,244,244,245,245,246,246,247,247,247,248,248,249,249,249,250,250,250,
	251,251,251,251,252,252,252,252,253,253,253,253,254,254,254,254,254,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	256,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,253,253,253,253,252,252,252,252,251,251,251,
	251,250,250,250,249,249,249,248,248,247,247,247,246,246,245,245,244,244,244,243,243,242,242,241,241,240,239,239,238,238,237,237,
	236,235,235,234,234,233,232,232,231,230,230,229,228,227,227,226,225,225,224,223,222,221,221,220,219,218,217,217,216,215,214,213,
	212,211,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,
	181,179,178,177,176,175,174,173,171,170,169,168,167,166,164,163,162,161,159,158,157,156,155,153,152,151,149,148,147,146,144,143,
	142,140,139,138,136,135,134,132,131,130,128,127,126,124,123,122,120,119,117,116,115,113,112,110,109,108,106,105,103,102,100,99,
	97,96,95,93,92,90,89,87,86,84,83,81,80,78,77,75,74,72,71,69,68,66,65,63,62,60,59,57,56,54,53,51,
	49,48,46,45,43,42,40,39,37,36,34,32,31,29,28,26,25,23,21,20,18,17,15,14,12,10,9,7,6,4,3,1,
	0,-1,-3,-4,-6,-7,-9,-10,-12,-14,-15,-17,-18,-20,-21,-23,-25,-26,-28,-29,-31,-32,-34,-36,-37,-39,-40,-42,-43,-45,-46,-48,
	-49,-51,-53,-54,-56,-57,-59,-60,-62,-63,-65,-66,-68,-69,-71,-72,-74,-75,-77,-78,-80,-81,-83,-84,-86,-87,-89,-90,-92,-93,-95,-96,
	-97,-99,-100,-102,-103,-105,-106,-108,-109,-110,-112,-113,-115,-116,-117,-119,-120,-122,-123,-124,-126,-127,-128,-130,-131,-132,-134,-135,-136,-138,-139,-140,
	-142,-143,-144,-146,-147,-148,-149,-151,-152,-153,-155,-156,-157,-158,-159,-161,-162,-163,-164,-166,-167,-168,-169,-170,-171,-173,-174,-175,-176,-177,-178,-179,
	-181,-182,-183,-184,-185,-186,-187,-188,-189,-190,-191,-192,-193,-194,-195,-196,-197,-198,-199,-200,-201,-202,-203,-204,-205,-206,-207,-208,-209,-210,-211,-211,
	-212,-213,-214,-215,-216,-217,-217,-218,-219,-220,-221,-221,-222,-223,-224,-225,-225,-226,-227,-227,-228,-229,-230,-230,-231,-232,-232,-233,-234,-234,-235,-235,
	-236,-237,-237,-238,-238,-239,-239,-240,-241,-241,-242,-242,-243,-243,-244,-244,-244,-245,-245,-246,-246,-247,-247,-247,-248,-248,-249,-249,-249,-250,-250,-250,
	-251,-251,-251,-251,-252,-252,-252,-252,-253,-253,-253,-253,-254,-254,-254,-254,-254,-254,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,
	-256,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-255,-254,-254,-254,-254,-254,-254,-253,-253,-253,-253,-252,-252,-252,-252,-251,-251,-251,
	-251,-250,-250,-250,-249,-249,-249,-248,-248,-247,-247,-247,-246,-246,-245,-245,-244,-244,-244,-243,-243,-242,-242,-241,-241,-240,-239,-239,-238,-238,-237,-237,
	-236,-235,-235,-234,-234,-233,-232,-232,-231,-230,-230,-229,-228,-227,-227,-226,-225,-225,-224,-223,-222,-221,-221,-220,-219,-218,-217,-217,-216,-215,-214,-213,
	-212,-211,-211,-210,-209,-208,-207,-206,-205,-204,-203,-202,-201,-200,-199,-198,-197,-196,-195,-194,-193,-192,-191,-190,-189,-188,-187,-186,-185,-184,-183,-182,
	-181,-179,-178,-177,-176,-175,-174,-173,-171,-170,-169,-168,-167,-166,-164,-163,-162,-161,-159,-158,-157,-156,-155,-153,-152,-151,-149,-148,-147,-146,-144,-143,
	-142,-140,-139,-138,-136,-135,-134,-132,-131,-130,-128,-127,-126,-124,-123,-122,-120,-119,-117,-116,-115,-113,-112,-110,-109,-108,-106,-105,-103,-102,-100,-99,
	-97,-96,-95,-93,-92,-90,-89,-87,-86,-84,-83,-81,-80,-78,-77,-75,-74,-72,-71,-69,-68,-66,-65,-63,-62,-60,-59,-57,-56,-54,-53,-51,
	-49,-48,-46,-45,-43,-42,-40,-39,-37,-36,-34,-32,-31,-29,-28,-26,-25,-23,-21,-20,-18,-17,-15,-14,-12,-10,-9,-7,-6,-4,-3,-1
};

/*
Copyright (c) 2024, Constantine Zykov
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
