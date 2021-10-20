// Port of a very simple game jam entry I made a few years ago.
// See end of file for license for this code
//    /Mattias Gustavsson

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "dos.h"

enum bitmap_t {
    INVALID,
    BACKGROUND,
    TREELINE,
    FACE,
    LOGO,
    BOAT,
    CHAIR,
    GUY_CHAIR, 
    GUY_IDLE,
    GUY_JUMP,
    GUY_LEFT,
    GUY_RIGHT,
    HUT1,
    HUT2,
    HUT_BOOK,
    HUT_BOOK_NOBULB,
    HUT_TV,
    HUT_TV_NODISH, 
    PALMTREE, 
    PENGUIN,
    PENGUIN_BATTERY, 
    PENGUIN_WALK, 
    RADIO_BATTERY, 
    RADIO_BATTERY_DISH,
    RADIO_BULB, 
    RADIO_BULB_BATTERY,
    RADIO_BULB_DISH,
    RADIO_DISH, 
    RADIO_FULL, 
    RADIO_NONE, 
    RADIO_SPR, 
    TREE2, 
};

struct {
    enum bitmap_t id;
    char const* filename;
    int w;
    int h;
    int cels;
    unsigned char* pixels[ 20 ];
} bitmaps[] = {
    { INVALID, NULL },
    { BACKGROUND, "files/stranded/background.gif" },
    { TREELINE, "files/stranded/treeline.gif" },
    { FACE, "files/stranded/face.gif" },
    { LOGO, "files/stranded/logo.gif" },
    { BOAT, "files/stranded/boat.gif" },
    { CHAIR, "files/stranded/chair.gif" },
    { GUY_CHAIR, "files/stranded/guychair.gif" },
    { GUY_IDLE, "files/stranded/guy_idle.gif" },
    { GUY_JUMP, "files/stranded/guy_jump_%04d.gif" },
    { GUY_LEFT, "files/stranded/guy_walk_left_%04d.gif" },
    { GUY_RIGHT, "files/stranded/guy_walk_right_%04d.gif" },
    { HUT1, "files/stranded/hut1.gif" },
    { HUT2, "files/stranded/hut2.gif" },
    { HUT_BOOK, "files/stranded/hut_book.gif" },
    { HUT_BOOK_NOBULB, "files/stranded/hut_book_nobulb.gif" },
    { HUT_TV, "files/stranded/hut_tv.gif" },
    { HUT_TV_NODISH, "files/stranded/hut_tv_nodish.gif" },
    { PALMTREE, "files/stranded/palmtree.gif" },
    { PENGUIN, "files/stranded/penguin.gif" },
    { PENGUIN_BATTERY, "files/stranded/penguin_battery.gif" },
    { PENGUIN_WALK, "files/stranded/penguin_walk_%04d.gif" },
    { RADIO_BATTERY, "files/stranded/radio_battery.gif" },
    { RADIO_BATTERY_DISH, "files/stranded/radio_battery_dish.gif" },
    { RADIO_BULB, "files/stranded/radio_bulb.gif" },
    { RADIO_BULB_BATTERY, "files/stranded/radio_bulb_battery.gif" },
    { RADIO_BULB_DISH, "files/stranded/radio_bulb_dish.gif" },
    { RADIO_DISH, "files/stranded/radio_dish.gif" },
    { RADIO_FULL, "files/stranded/radio_full.gif" },
    { RADIO_NONE, "files/stranded/radio_none.gif" },
    { RADIO_SPR, "files/stranded/radio_spr.gif" },
    { TREE2, "files/stranded/tree2.gif" },
};


int palcount = 0;
unsigned char palette[ 768 ];


void loadbitmaps( void ) {
    for( int i = 0; i < sizeof( bitmaps ) / sizeof( *bitmaps ); ++i ) {
        char const* filename = bitmaps[ i ].filename;
        if( !filename ) continue;
        char temp[256];
        if( strstr( filename, "%04d" ) ) {
            for( int j = 0; j < 20; ++j ) {
                sprintf( temp, filename, j + 1 );
                int w = 0;
                int h = 0;
                bitmaps[ i ].pixels[ j ] = loadgif( temp, &w, &h, &palcount, palette );            
                if( !bitmaps[ i ].pixels[ j ] ) {
                    bitmaps[ i ].cels = j;
                    break;
                } else {
                    bitmaps[ i ].w = w; 
                    bitmaps[ i ].h = h;
                }
            }
        } else {
            bitmaps[ i ].pixels[ 0 ] = loadgif( filename, &bitmaps[ i ].w, &bitmaps[ i ].h, &palcount, palette );            
            bitmaps[ i ].cels = 1;
        }
    } 
}


struct {
    int x;
    int y;
    int origin_x;
    int origin_y;
    int visible;
    int cel;
    enum bitmap_t bitmap;
    char const* label_text;
    int label_color;
    int label_wrap;
} sprites[ 64 ];

int spr_index = 0;

int sprite( int spr, int x, int y, enum bitmap_t bitmap ) {
    sprites[ spr ].x = x;
    sprites[ spr ].y = y;
    sprites[ spr ].visible = 1;
    sprites[ spr ].bitmap = bitmap;
    sprites[ spr ].label_text = NULL;
    sprites[ spr ].label_color = 0;
    sprites[ spr ].label_wrap = 0;
    return spr;
};

int label( int spr, int x, int y, char const* text, int color ) {
    sprites[ spr ].x = x;
    sprites[ spr ].y = y;
    sprites[ spr ].visible = 1;
    sprites[ spr ].bitmap = INVALID;
    sprites[ spr ].label_text = text;
    sprites[ spr ].label_color = color;
    sprites[ spr ].label_wrap = 0;
    return spr;
};

void sprite_hide( int spr ) {
    sprites[ spr ].visible = 0;
}

void sprite_show( int spr ) {
    sprites[ spr ].visible = 1;
}

int sprite_x( int spr ) {
    return sprites[ spr ].x;
}

int sprite_y( int spr ) {
    return sprites[ spr ].y;
}

void sprite_pos( int spr, int x, int y ) {
    sprites[ spr ].x = x;
    sprites[ spr ].y = y;
}

void sprite_cel( int spr, int cel ) {
    sprites[ spr ].cel = cel;
}

void sprite_origin( int spr, int x, int y ) {
    sprites[ spr ].origin_x = x;
    sprites[ spr ].origin_y = y;
}

void sprite_bitmap( int spr, enum bitmap_t bitmap ) {
    sprites[ spr ].bitmap = bitmap;
}

void label_color( int spr, int color ) {
    sprites[ spr ].label_color = color;
}

void label_wrap( int spr, int wrap ) {
    sprites[ spr ].label_wrap = wrap;
}

void label_text( int spr, char const* text ) {
    sprites[ spr ].label_text = text;
}


void drawsprites( void ) {
    for( int i = 0; i < spr_index; ++i ) {
        if( !sprites[ i ].visible ) continue;
        enum bitmap_t bitmap = sprites[ i ].bitmap;
        char const* label = sprites[ i ].label_text;
        int xp = sprites[ i ].x - sprites[ i ].origin_x;
        int yp = sprites[ i ].y - sprites[ i ].origin_y;
        if( bitmap != INVALID ) {
            int width = bitmaps[ bitmap ].w;
            int height = bitmaps[ bitmap ].h;
            unsigned char* pixels = bitmaps[ bitmap ].pixels[ sprites[ i ].cel % bitmaps[ bitmap ].cels ];
            maskblit( xp, yp, pixels, width, height, 0, 0, width, height, 255 );
        } else if( label ) {
            setcolor( 254 );
            yp += 30;
			for( int y = -1; y <= 1; ++y ) {
                for( int x = -1; x <= 1; ++x ) {
				    if( x == 0 && y == 0 ) continue;
                    centertextxy( xp + x, yp + y, label, sprites[ i ].label_wrap );
                }
            }
            centertextxy( xp + 2, yp, label, sprites[ i ].label_wrap );
            centertextxy( xp + 2, yp + 1, label, sprites[ i ].label_wrap );
            centertextxy( xp - 1, yp + 2, label, sprites[ i ].label_wrap );
            centertextxy( xp    , yp + 2, label, sprites[ i ].label_wrap );
            centertextxy( xp + 1, yp + 2, label, sprites[ i ].label_wrap );
            setcolor( sprites[ i ].label_color );
            centertextxy( xp, yp, label, sprites[ i ].label_wrap );
        }
    }
}


struct dialog_t {
    char const* actor;
    char const* line;
};


struct dialog_t dlg_lets_go_home[] = {
    { "player_sitting", "Hmmm... maybe three and a half years on this island is enough..." },
    { "player_sitting", "I've enjoyed being stranded here, but now I think I'd like to go home." },
    { "player_sitting", "Last time I checked, the radio was working fine. I think I'll call for help now." },
    { "#SetFlag","may_stand" },
    { NULL, NULL }
};

struct dialog_t dlg_radio_first[] = {
    { "player_radio", "Oh no! There are parts missing!" },
    { "player_radio", "Someone have grabbed the ANTENNA, the BATTERY, and the BULB." },
    { "player_radio", "I *have* to find them!" },
    { "#SetFlag", "looked_at_radio" },
    { NULL, NULL }
};

struct dialog_t dlg_radio_none[] = {
    { "player_radio", "It is missing the ANTENNA, the BATTERY, and the BULB." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_bulb[] = {
    { "player_radio", "It is missing the ANTENNA and the BATTERY." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_dish[] = {
    { "player_radio", "It is missing the BULB and the BATTERY." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_battery[] = {
    { "player_radio", "It is missing the ANTENNA and the BULB." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_bulb_dish[] = {
    { "player_radio", "It is missing the BATTERY." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_battery_dish[] = {
    { "player_radio", "It is missing the BULB." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_bulb_battery[] = {
    { "player_radio", "It is missing the ANTENNA." },
    { NULL, NULL }
};

struct dialog_t dlg_radio_all[] = {
    { "player_radio", "The radio is complete! Now I can go home." },
    { "player_radio", "MAYDAY! MAYDAY! I AM STRANDED ON AN ISLAND. SEND HELP!" },
    { "#SetFlag", "rescued" },
    { NULL, NULL }
};

struct dialog_t dlg_hut_tv[] = {
    { "player_hut", "Hey, that's my antenna!" },
    { "tv_guy", "Huh, what? No, it's my antenna now. I found it." },
    { "tv_guy", "I need it for my TV. To watch my shows." },
    { "player_hut", "But *I* need it for my radio. So I can call for help and get home." },
    { "tv_guy", "Uh... I *am* very tired, I need to sleep. But I can't stop watching." },
    { "player_hut", "You can have the antenna back later, I only need it to make the transmission." },
    { "tv_guy", "Ok, go ahead then." },
    { "#ClearFlag", "at_hut_tv" },
    { NULL, NULL }
};

struct dialog_t dlg_hut_tv_sleep[] = {
    { "player_hut", "Looks like he's sleeping. I better not disturb him." },
    { NULL, NULL }
};

struct dialog_t dlg_hut_book[] = {
    { "player_hut", "That's my bulb! For my radio." },
    { "book_guy", "Oh, yes yes, quite. I need it so I can read my books." },
    { "player_hut", "But *I* need it for my radio. So I can call for help and get home." },
    { "book_guy", "I see, I see. Well I am really rather sleepy. But I can not put this book down. Quite the page-turner it is." },
    { "player_hut", "You can have the bulb back later, I only need it to make the transmission." },
    { "book_guy", "Ah, very well, very well." },
    { "#ClearFlag", "at_hut_bulb" },
    { NULL, NULL }
};

struct dialog_t dlg_hut_book_sleep[] = {
    { "player_hut", "Looks like he's sleeping. I better not disturb him." },
    { NULL, NULL }
};

struct dialog_t dlg_penguin[] = {
    { "penguin", "Waddya want!?" },
    { "player", "Uh, is that my battery you have there?" },
    { "penguin", "Don't know! Don't care!" },
    { "player", "But you're not even using it!" },
    { "penguin", "Sure I do! I lick it." },
    { "player", "You *lick* it?" },
    { "penguin", "Makes my tongue tingle." },
    { "player", "But *I* need it for my radio. So I can call for help and get home." },
    { "player", "What are you even doing here? Aren't you supposed to be on the south pole or something?" },
    { "penguin", "Vacation." },
    { "penguin", "It's about time I got back anyway." },
    { "penguin", "Don't touch my battery while I'm gone!" },
    { "#SetFlag", "penguin_running" },
    { "player", "Yeah, right." },
    { "#SetFlag", "penguin_gone" },
    { NULL, NULL }
};

struct dialog_t dlg_grab_battery[] = {
    { "player", "I'm having this!" },
    { NULL, NULL }
};



enum dlg_state_t {
    DIALOG_STATE_READY,
    DIALOG_STATE_RUN_DIALOG,
    DIALOG_STATE_DISPLAYING_SPEECH,
    DIALOG_STATE_SENDING_EVENT,
    DIALOG_STATE_UNDEFINED,
};

struct {
    struct dialog_t* dlg;
    enum dlg_state_t current_state;
    int current_index;
} dialog;


enum dlg_state_t dialogstate( void ) {
    return dialog.current_state;
}


void startdialog( struct dialog_t* dlg ) {
    dialog.dlg = dlg;
    dialog.current_index = 0;
    dialog.current_state = DIALOG_STATE_RUN_DIALOG;
}


void rundialogline( void) {
	if( dialog.dlg[ dialog.current_index ].actor == NULL ) {
		dialog.current_state = DIALOG_STATE_READY;
        dialog.dlg = NULL;
		return;
	}

    char const* actor = dialog.dlg[ dialog.current_index ].actor;
	if( *actor != '#' ) {
		dialog.current_state = DIALOG_STATE_DISPLAYING_SPEECH;
	} else  {
		dialog.current_state = DIALOG_STATE_SENDING_EVENT;
	}
}


void execdialog( void ) {
	switch( dialog.current_state ) {
		case DIALOG_STATE_UNDEFINED: break;

		case DIALOG_STATE_READY: break;

		case DIALOG_STATE_RUN_DIALOG: {
			rundialogline();
		} break;
		
		case DIALOG_STATE_DISPLAYING_SPEECH: {
            ++dialog.current_index;
			rundialogline();
		} break;

		case DIALOG_STATE_SENDING_EVENT: {
            ++dialog.current_index;
			dialog.current_state = DIALOG_STATE_RUN_DIALOG;
		} break;
	}
}


char const* dialogactor( void ) {
    return dialog.dlg[ dialog.current_index ].actor;
}


char const* dialogline( void ) {
    return dialog.dlg[ dialog.current_index ].line;
}


void delay( int jiffys ) {
    for( int i = 0; i < jiffys; ++i ) {
        waitvbl();
    	  if( shuttingdown() || *readkeys() == KEY_ESCAPE ) break;
        drawsprites();
        swapbuffers();
    }
}


int is_near( int spr, int xpos ) {
	return ( xpos > sprite_x( spr ) - 25 ) && ( xpos < sprite_x( spr ) + 25 );
}


void set_interact( int spr, char const* str, int interact, int offset ) {
    label_text( interact, str );
	sprite_pos( interact, sprite_x( spr ) + 160 + offset, sprite_y( interact ) );
}

#define COLOR_WHITE 203
#define COLOR_GREEN 77
#define COLOR_YELLOW 174
#define COLOR_CYAN 144
#define COLOR_ORANGE 214

void setup_speech( int speech, char const* character )
	{
    struct {
        char const* name;
        int x;
        int y;
        int color;
        int wrap;
    } speech_settings[] = {
		{ "player_sitting", 50, 85, COLOR_GREEN, 220 },
		{ "player", 20, 60, COLOR_GREEN, 220 }, 
		{ "player_hut", 100, 60, COLOR_GREEN, 220 }, 
		{ "player_radio", 50, 20, COLOR_GREEN, 220 }, 
		{ "tv_guy", 20, 20, COLOR_YELLOW, 220 }, 
		{ "book_guy", 20, 20, COLOR_CYAN, 220 }, 
		{ "penguin", 185, 75, COLOR_ORANGE, 120 }, 
	};

    for( int i = 0; i < sizeof( speech_settings ) / sizeof( *speech_settings ) ; ++i ) {
        if( strcmp( speech_settings[ i ].name, character ) == 0 ) {
            sprite_pos( speech, speech_settings[ i ].x, speech_settings[ i ].y );
            label_color( speech, speech_settings[ i ].color );
            label_wrap( speech, speech_settings[ i ].wrap );
            return;
        }
    }
}


float ease_in_out_quad( float t ) { 
	return ( ( t *= 2.0f ) < 1.0f ) 
		? ( 0.5f * t * t ) 
		: ( -0.5f * ( ( t - 1.0f ) * ( t - 3.0f ) - 1.0f ) ); 
}


float ease_out_quad( float t ) { 
	return -t * ( t - 2.0f ); 
}


float ease_out_elastic( float t ) { 
	if( t == 0.0f ) {
        return 0.0f; 	
    }
	if( t == 1.0f ) {
        return 1.0f; 
    }
	
	float p = 0.4f; 
	float s = p / 4.0f; 
	return ( (float)pow( 2.0f, -10.0f * t ) * (float)sin( ( t - s ) * ( 2.0f * (float)acos( -1.0f ) ) / p ) + 1.0f ); 
}


float ease_out_bounce( float t ) { 
	if( t < ( 1.0f / 2.75f ) ) {
		return 7.5625f * t * t; 
    } else if( t < ( 2.0f / 2.75f ) ) {
        t -= ( 1.50f / 2.75f );
		return 7.5625f * t * t + 0.75f; 
    } else if( t < ( 2.5f / 2.75f ) ) {
        t -= ( 2.25f / 2.75f );
		return 7.5625f * t * t + 0.9375f; 
    } else {
        t -= ( 2.625f / 2.75f );
		return 7.5625f * t * t + 0.984375f; 
    }
}


void title_screen( void ) {
    spr_index = 0;
    sprite( spr_index++, -1050, 0, BACKGROUND );
    sprite( spr_index++, -1050, 0, TREELINE ) ;
    int face = sprite( spr_index++, 0, 240, FACE ) ;
    int logo = sprite( spr_index++, 0, -200, LOGO ) ;
    int credits = label( spr_index++, -160, 192, "Code, art, music and design by Mattias Gustavsson", COLOR_WHITE );

    int face_delay = 6;
    int face_move = 0;
    int logo_delay = 30;
    int logo_move = 0;
    int credits_delay = 30;
    int credits_move = 0;

	while( !shuttingdown() ) {
        waitvbl();
		if( keystate( KEY_ESCAPE ) ) break; 
		if( keystate( KEY_SPACE ) ) return;

        if( face_delay >= 0 ) {
            --face_delay;
        } else if( face_move <= 120 ) {
            float d = ease_out_elastic( face_move / 120.0f );
            float d2 = ease_in_out_quad( face_move / 120.0f );
            int y = (int)( 240.0f - 155.0f * d - 15.0f * d2 );
            sprite_pos( face, sprite_x( face ), y );
            ++face_move;
        }

        if( logo_delay >= 0 ) {
            --logo_delay;
        } else if( logo_move <= 60 ) {
            float d = ease_out_bounce( logo_move / 60.0f );
            int y = (int)( -200.0f + 210.0f * d );
            sprite_pos( logo, sprite_x( logo ), y );
            ++logo_move;
        }

        if( credits_delay >= 0 ) {
            --credits_delay;
        } else if( credits_move <= 60 ){
            float d = ease_out_quad( credits_move / 60.0f );
            int x = (int)( -160.0f + 320.0f * d );
            sprite_pos( credits, x, sprite_y( credits ) );
            ++credits_move;
        }
        drawsprites();
        swapbuffers();
	}
    exit( 0 );

}


int main( int argc, char* argv[] ) {
    setvideomode( videomode_320x240 );
    //setsoundbank( DEFAULT_SOUNDBANK_SB16 );
    setdoublebuffer( 1 );
    loadbitmaps();
    for( int i = 0; i < palcount; ++i ) {
        setpal(i, palette[ 3 * i + 0 ],palette[ 3 * i + 1 ], palette[ 3 * i + 2 ] );
    }
    int font = installuserfont( "files/stranded/volter.fnt" );
    settextstyle( font, 0, 0, 0 );

    playmusic( loadmid( "files/stranded/stranded.mid" ), 1, 255 );

	title_screen();
  
    int objects[ 32 ] = { 0 };
    int objects_count = 0;

    spr_index = 0;
    int o = 30;
    int background = sprite( spr_index++, -65, 0, BACKGROUND );
    int treeline = sprite( spr_index++, -65, 0, TREELINE );
    int radio = sprite( spr_index++, 240, 10 + o, RADIO_SPR ); objects[ objects_count++ ] = radio;
    int hut1 = sprite( spr_index++, 540, 10 + o, HUT1 ); objects[ objects_count++ ] = hut1;
	int hut2 = sprite( spr_index++, 850, 20 + o, HUT2 ); objects[ objects_count++ ] = hut2;
	objects[ objects_count++ ] =  sprite( spr_index++, 180, -30 + o, TREE2 );
	objects[ objects_count++ ] = sprite( spr_index++, 120, -10 + o, PALMTREE );
	int chair = sprite( spr_index++, 65, 25 + o, CHAIR ); objects[ objects_count++ ] = chair;
	int penguin_battery = sprite( spr_index++, 970 + 62, -10 + o, PENGUIN_BATTERY ); objects[ objects_count++ ] = penguin_battery;
	int penguin = sprite( spr_index++, 970, -10 + o, PENGUIN ); objects[ objects_count++ ] = penguin;

	int xpos = 65;
	int guy = sprite( spr_index++, xpos, 25 + o, GUY_CHAIR );
    
	int interact = label( spr_index++, 160, 70, "", COLOR_WHITE );
    
    int radio_back = sprite( spr_index++, -250 + o, 0, BACKGROUND );
    sprite_hide( radio_back );

    int radio_pic = sprite( spr_index++, 0, 0, RADIO_FULL );
    sprite_hide( radio_pic );

	int hut_tv_pic = sprite( spr_index++, 0, 0, HUT_TV );
    sprite_hide( hut_tv_pic );

	int hut_bulb = sprite( spr_index++, 0, 0, HUT_BOOK );
    sprite_hide( hut_bulb );
        	
    int guy_inside = sprite( spr_index++, 0, 0 + o, GUY_IDLE );
    sprite_hide( guy_inside );

    int boat = sprite( spr_index++, -320, 30 + o, BOAT );

	int speech = label( spr_index++, 50, 70, "", COLOR_GREEN );
    label_wrap( speech, 220 );


	int fadeout = 0;
  int fadeout_delay = 120;
  int speech_visible = 1;
	int rescued = 0;
	int may_stand = 0;
	int looked_at_radio = 0;
	int sitting = 1;
	int penguin_gone = 0;
	int at_radio = 0;
	int at_hut_tv = 0;
	int at_hut_bulb = 0;
	int has_dish = 0;
	int has_bulb = 0;
	int has_battery = 0;	  
  int shown_intro = 0;
  int boat_delay = 0;
  int boat_move = 0;

    float anim = 0.0f;

	while( !shuttingdown() && !keystate( KEY_ESCAPE ) ) {
		for( int i = 0; i < objects_count; ++i )
            sprite_origin( objects[ i ], xpos, 0 );

		sprite_origin( interact, xpos, 0 );

		sprite_origin( guy, xpos, 0 );
		sprite_pos( guy, xpos, sprite_y( guy ) );
		sprite_pos( background, -xpos, sprite_y( background ) );
		sprite_pos( treeline, -xpos, sprite_y( treeline ) );			

        if( !shown_intro ) {
            delay( 60 );
            startdialog( dlg_lets_go_home );
            shown_intro = 1;
        }

        anim += 20.0f / 60.0f;
        sprite_cel( guy, (int) anim );

        waitvbl();
    
		if( fadeout ) {
            if( --fadeout_delay <= 0 ) {
			    int all_black = 1;
			    for( int i = 0; i < 256; ++i ) {
                    int r, g, b;
                    getpal( i, &r, &g, &b );
				    if( r > 0 || g > 0 || b > 0 ) all_black = 0;
                    if( b > 0 ) --b;
                    else if( g > 0 ) --g;
                    else if( r > 0 ) --r;
				    setpal( i, r, g, b );
			    }
			    if( all_black ) break;;
            }
		}
		
		if( keystate( KEY_ESCAPE ) ) break;


        if( dialogstate() != DIALOG_STATE_READY ) {
			if( dialogstate() == DIALOG_STATE_SENDING_EVENT ) {
   				char const* event = dialogactor();
				if( strcmp( event, "#SetFlag" ) == 0 ) {
					char const* flag = dialogline();
					if( strcmp( flag, "may_stand" ) == 0 ) {
						may_stand = 1;
					}
					if( strcmp( flag, "looked_at_radio" ) == 0 ) {
						looked_at_radio = 1;
					}
					if( strcmp( flag, "penguin_running" ) == 0 ) {
                        sprite_pos( penguin, 970 + 62, sprite_y( penguin ) );
                        sprite_bitmap( penguin, PENGUIN_WALK );
						delay( 30 );
                        float penganim = 0.0f;
                        for( int i = 0; i < 90; ++i ) {
                            waitvbl();
                            penganim += 25.0f / 60.0f;
                            sprite_cel( penguin, (int) penganim );
                            sprite_pos( penguin, sprite_x( penguin ) + 3, sprite_y( penguin ) );
                            drawsprites();
                            swapbuffers();
                        }
                        sprite_hide( penguin );
					}
					if( strcmp( flag, "penguin_gone" ) == 0 ) {
						penguin_gone = 1;
						execdialog();
					}
					if( strcmp( flag, "rescued" ) == 0 ) {
						rescued = 1;
					}
				}
				if( strcmp( event, "#ClearFlag" ) == 0 ) {
					char const* flag = dialogline();
					if( strcmp( flag, "at_hut_tv" ) == 0 ) {
						sprite_hide( hut_tv_pic );
						sprite_hide( guy_inside );
						sprite_bitmap( hut_tv_pic, HUT_TV_NODISH );
						has_dish = 1;
						at_hut_tv = 0;
					}
					if( strcmp( flag, "at_hut_bulb" ) == 0 ) {
						sprite_hide( hut_bulb );
						sprite_hide( guy_inside );
						sprite_bitmap( hut_bulb, HUT_BOOK_NOBULB );
						has_bulb = 1;
						at_hut_bulb = 0;
					}
				}
 				execdialog();
			} else if( dialogstate() == DIALOG_STATE_DISPLAYING_SPEECH ) {
                sprite_bitmap( guy, sitting ? GUY_CHAIR : GUY_IDLE );
				setup_speech( speech, dialogactor() );
				label_text( speech, dialogline() );
				if( speech_visible && *readkeys() == KEY_SPACE )  {					
					speech_visible = 0;
                    label_text( speech, "" ); 
                    delay( 30 );
					execdialog(); 
                    speech_visible = 1;
				}
			} else {
                execdialog();
			}
            drawsprites();
            swapbuffers();
            continue;
		} else {
		    label_text( speech, "" ); 
		}
		   
        label_text( interact, "" ); 
		if( !sitting && !at_radio && !at_hut_tv && !at_hut_bulb && !rescued ) {
			if( is_near( radio, xpos ) ) set_interact( radio, "Radio", interact, 0 );
			if( is_near( hut1, xpos ) ) set_interact( hut1, "Hut", interact, 0 );
			if( is_near( hut2, xpos ) ) set_interact( hut2, "Hut", interact, 0 );
			if( !penguin_gone && is_near( penguin, xpos ) ) set_interact( penguin, "Penguin", interact, 60 );
			if( !has_battery && penguin_gone && is_near( penguin_battery, xpos ) ) set_interact( penguin_battery, "Battery", interact, 0 );
				
			if( keystate( KEY_LEFT ) )  {
				xpos -= 2;
				if( xpos < 10 ) xpos = 10;
                sprite( guy, sprite_x( guy ), sprite_y( guy ), GUY_LEFT );
			} else if( keystate( KEY_RIGHT ) ) {
				xpos += 2;
				if( xpos > 1120 ) xpos = 1120;
                sprite( guy, sprite_x( guy ), sprite_y( guy ), GUY_RIGHT );
			} else {
                sprite( guy, sprite_x( guy ), sprite_y( guy ), GUY_IDLE );
			}

			if( *readkeys() == KEY_SPACE ) {
				if( is_near( radio, xpos ) ) {
					sprite_show( radio_pic );
					if( has_dish && has_bulb && has_battery ) {
                        sprite_bitmap( radio_pic, RADIO_FULL );
						startdialog( dlg_radio_all );
					} else if( has_dish && has_bulb ) {
                        sprite_bitmap( radio_pic, RADIO_BULB_DISH );
						startdialog( dlg_radio_bulb_dish );
					} else if( has_battery && has_bulb ) {
                        sprite_bitmap( radio_pic, RADIO_BULB_BATTERY );
						startdialog( dlg_radio_bulb_battery );
					} else if( has_dish && has_battery ) {
                        sprite_bitmap( radio_pic, RADIO_BATTERY_DISH );
						startdialog( dlg_radio_battery_dish );
					} else if( has_dish ) {
                        sprite_bitmap( radio_pic, RADIO_DISH );
						startdialog( dlg_radio_dish );
					} else if( has_battery ) {
                        sprite_bitmap( radio_pic, RADIO_BATTERY );
						startdialog( dlg_radio_battery );
					} else if( has_bulb ) {
                        sprite_bitmap( radio_pic, RADIO_BULB );
						startdialog( dlg_radio_bulb );
					} else {
                        sprite_bitmap( radio_pic, RADIO_NONE );
						if( !looked_at_radio )
							startdialog( dlg_radio_first );
						else
							startdialog( dlg_radio_none );
					}

					sprite_show( radio_back );
					at_radio = 1;
				}
				
                if( is_near( hut1, xpos ) ) {
					sprite_show( hut_tv_pic );
					sprite_show( guy_inside );
                    sprite_pos( guy_inside, 100, 60 );
					if( !has_dish )
						startdialog( dlg_hut_tv );
					else
						startdialog( dlg_hut_tv_sleep );
					at_hut_tv = 1;
				}

				if( is_near( hut2, xpos ) ) {
					sprite_show( hut_bulb );
					sprite_show( guy_inside );
                    sprite_pos( guy_inside, 100, 60 );
					if( !has_bulb )
						startdialog( dlg_hut_book );
					else
						startdialog( dlg_hut_book_sleep );
					at_hut_bulb = 1;
				}

				if( !penguin_gone && is_near( penguin, xpos ) ) {
                    sprite_bitmap( penguin, PENGUIN );
					startdialog( dlg_penguin );
					label_text( interact, "" );
                    sprite_bitmap( guy, GUY_IDLE );
				}

				if( !has_battery && penguin_gone && is_near( penguin_battery, xpos ) ) {
					label_text( interact, "" );
					sprite_hide( penguin );
					startdialog( dlg_grab_battery );
					sprite_hide( penguin_battery );
					has_battery = 1;
				}

                if( is_near( chair, xpos ) ) {
					sitting = 1;
					xpos = 65;
					sprite_bitmap( guy, GUY_CHAIR );
                    sprite_pos( guy, 0, 25+o );
		            sprite_origin( guy, xpos, 0 );
		            sprite_pos( guy, xpos, sprite_y( guy ) );
				}
			}

		} else if( sitting && may_stand ) {
			if( *readkeys() == KEY_SPACE ) {
				sitting = 0;
                sprite( guy, 0, 30+o, GUY_IDLE ); 
			}
		} else if( at_radio ) {
			if( *readkeys() == KEY_SPACE ) {
				sprite_hide( radio_pic );
				sprite_hide( radio_back );
				at_radio = 0;

                if( rescued ) {
                    sprite_bitmap( guy, GUY_JUMP );
					xpos = 20;
		            sprite_origin( guy, xpos, 0 );
		            sprite_pos( guy, xpos, sprite_y( guy ) );
                    boat_delay = 90;
                    boat_move = 0;
					fadeout = 1; 
				}
			}
		} else if( at_hut_tv ) {
			if( *readkeys() == KEY_SPACE ) {
			    sprite_hide( hut_tv_pic );
			    sprite_hide( guy_inside );
				sprite_bitmap( hut_tv_pic, HUT_TV_NODISH );
				has_dish = 1;
				at_hut_tv = 0;
			}
		} else if( at_hut_bulb ) {
			if( *readkeys() == KEY_SPACE ) {
				sprite_hide( hut_bulb );
				sprite_hide( guy_inside );
				sprite_bitmap( hut_bulb, HUT_BOOK_NOBULB );
				has_bulb = 1;
				at_hut_bulb = 0;
            }
        }

        execdialog();

        if( rescued && fadeout ) {
            if( boat_delay >= 0 ) {
                --boat_delay;
            } else {
                float d = ease_out_quad( boat_move / 120.0f );
                int x = (int)( -320.0f + 130.0f * d );
                sprite_pos( boat, x, sprite_y( boat ) );
                ++boat_move;
            }
        }

		sprite_origin( guy, xpos, 0 );
		sprite_pos( guy, xpos, sprite_y( guy ) );
        drawsprites();
        swapbuffers();
	}

    return 0;
}

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2021 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/