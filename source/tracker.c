// Work-in-progress program for composing music. It is quite incomplete,
// but shows how to use note commands and draw things in text mode.
// See end of file for license
//    /Mattias Gustavsson

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dos.h"

enum frame_t {
    FRAME_SINGLE,
    FRAME_HDOUBLE,
    FRAME_VDOUBLE, 
    FRAME_DOUBLE, 
    FRAME_SOLID, 
};

void clear( int x, int y, int w, int h );
void frame( enum frame_t type, int x, int y, int w, int h );
void vdivider( enum frame_t type, int x, int y, int h );
void hdivider( enum frame_t type, int x, int y, int w );
void divcross( enum frame_t type, int x, int y );



/*

CC OP
 0 No operation
 1 volume
 2 pan
 3 pitch bend up
 4 pitch bend down
 5 chord
 6 pattern break
 7 step time ms

*/


struct internal_tracker_note_t {
    int note;
    int program;
    int velocity;
    int cc_command;
    int cc_data;
};


struct internal_tracker_channel_t {
    struct internal_tracker_note_t notes[ 100 ];
};


#define INTERNAL_TRACKER_MAX_CHANNEL_COUNT 4


struct internal_tracker_pattern_t {
    struct internal_tracker_channel_t channels[ INTERNAL_TRACKER_MAX_CHANNEL_COUNT ];
};


struct internal_tracker_midi_chan_info_t {
    int drums;
    int age;
    int channel;
    int program;
};


struct internal_tracker_spinner_t {
    int x;
    int y;
    int min;
    int max;
    int *value;
    void (*on_changed)( int, void* );
    void* user_data;
};


#define INTERNAL_TRACKER_ACTION_LOAD 1
#define INTERNAL_TRACKER_ACTION_SAVE 2
#define INTERNAL_TRACKER_ACTION_PLAY_SONG 3
#define INTERNAL_TRACKER_ACTION_PLAY_PATTERN 4


struct internal_tracker_button_t {
    int x;
    int y;
    int width;
    char label[ 32 ];
    int action;
};


struct internal_tracker_label_t {
    int x;
    int y;
    char label[ 32 ];
};


struct internal_tracker_numedit_t {
    int x;
    int y;
    int width;
    int* value;
};


enum internal_tracker_play_state_t {
   INTERNAL_TRACKER_PLAY_STATE_STOPPED,
   INTERNAL_TRACKER_PLAY_STATE_PLAYING_PATTERN,
   INTERNAL_TRACKER_PLAY_STATE_PLAYING_SONG,
   /*INTERNAL_TRACKER_PLAY_STATE_RECORDING_PATTERN,*/
};


struct tracker_t {
    enum internal_tracker_play_state_t play_state;
    int play_time_us;
    int step_time_us;

    int base_octave;
    int beats_per_minute;
    int current_velocity;
    int edit_mode;
    int current_program;
    int current_channel;
    int current_pattern;
    int pattern_pos;
    int channel_count;

    int curs_x;
    int curs_y;

    int mouse_x;
    int mouse_y;

    int pattern_length;
    struct internal_tracker_pattern_t patterns[ 100 ];

    struct internal_tracker_midi_chan_info_t midi_chan_info[ 16 ];

    int song_pos;
    int song_length;
    int song[ 1000 ];

    int keyboard_state[ KEYCOUNT ];
    int prev_keyboard_state[ KEYCOUNT ];

    struct internal_tracker_spinner_t spinners[ 32 ];
    int spinner_count;
    int spin_cooldown;

    struct internal_tracker_button_t buttons[ 32 ];
    int button_count;

    struct internal_tracker_label_t labels[ 32 ];
    int label_count;

    struct internal_tracker_numedit_t numedits[ 32 ];
    int numedit_count;
};


int internal_tracker_key_pressed( struct tracker_t* tracker, enum keycode_t key ) {
    return tracker->keyboard_state[ key ] && !tracker->prev_keyboard_state[ key ];
}


int internal_tracker_key_released( struct tracker_t* tracker, enum keycode_t key ) {
    return !tracker->keyboard_state[ key ] && tracker->prev_keyboard_state[ key ];
}


int internal_tracker_key_down( struct tracker_t* tracker, enum keycode_t key ) {
    return tracker->keyboard_state[ key ];
}


int internal_tracker_add_spinner( struct tracker_t* tracker, int x, int y, int min, int max, int* value, void (*on_changed)( int, void* ), void* user_data ) {
    if( tracker->spinner_count >= sizeof( tracker->spinners ) / sizeof( *tracker->spinners ) ) {
        return -1;
    }

    int index = tracker->spinner_count++;
    struct internal_tracker_spinner_t* spinner = &tracker->spinners[ index ];
    spinner->x = x;
    spinner->y = y;
    spinner->value = value;
    spinner->min = min;
    spinner->max = max;
    spinner->on_changed = on_changed;
    spinner->user_data =  user_data;
    return index;
}


void internal_tracker_draw_spinners( struct tracker_t* tracker ) {
    textbackground( 2 );
    for( int i = 0; i < tracker->spinner_count; ++i ) {
        struct internal_tracker_spinner_t* spinner = &tracker->spinners[ i ];
        textcolor( tracker->mouse_x == spinner->x && tracker->mouse_y == spinner->y ? 14 : 0 );
        gotoxy( spinner->x, spinner->y );
        cputs( "\x18" );
        textcolor( tracker->mouse_x == spinner->x + 1 && tracker->mouse_y == spinner->y ? 14 : 0 );
        gotoxy( spinner->x +1, spinner->y );
        cputs( "\x19"  );

    }
}


void internal_tracker_update_spinners( struct tracker_t* tracker ) {
    if( tracker->spin_cooldown > 0 ) {
        --tracker->spin_cooldown;
    }
    if( ( internal_tracker_key_down( tracker, KEY_LBUTTON ) && tracker->spin_cooldown <= 0 ) ||
        internal_tracker_key_pressed( tracker, KEY_LBUTTON ) ) {

        for( int i = 0; i < tracker->spinner_count; ++i ) {
            struct internal_tracker_spinner_t* spinner = &tracker->spinners[ i ];
            if( tracker->mouse_x == spinner->x && tracker->mouse_y == spinner->y ) {
                ++(*spinner->value);
                if( *spinner->value >= spinner->max ) {
                    *spinner->value = spinner->min;
                }
                if( spinner->on_changed ) {
                    spinner->on_changed( *spinner->value, spinner->user_data );
                }
                tracker->spin_cooldown = 8;
            }
            else if( tracker->mouse_x == spinner->x + 1 && tracker->mouse_y == spinner->y ) {
                --(*spinner->value);
                if( *spinner->value < spinner->min ) {
                    *spinner->value = spinner->max;
                }
                if( spinner->on_changed ) {
                    spinner->on_changed( *spinner->value, spinner->user_data );
                }
                tracker->spin_cooldown = 8;
            }
        }
    }
}


int internal_tracker_add_button( struct tracker_t* tracker, int x, int y, int width, char const* label, int action ) {
    if( tracker->button_count >= sizeof( tracker->buttons ) / sizeof( *tracker->buttons ) ) {
        return -1;
    }

    int index = tracker->button_count++;
    struct internal_tracker_button_t* button = &tracker->buttons[ index ];
    button->x = x;
    button->y = y;
    button->width = width;
    strcpy( button->label, label );
    button->action = action;
    return index;
}


void internal_tracker_draw_buttons( struct tracker_t* tracker ) {    
    for( int i = 0; i < tracker->button_count; ++i ) {
        struct internal_tracker_button_t* button = &tracker->buttons[ i ];
        int hover = tracker->mouse_x >= button->x && tracker->mouse_x < button->x + button->width && tracker->mouse_y == button->y;
        int pressed = hover && internal_tracker_key_down( tracker, KEY_LBUTTON );
        textcolor( hover ? 14 : 0 );
        textbackground( 2 );
        gotoxy( button->x + pressed, button->y );
        clear( button->x + pressed, button->y, button->width, 1 );
        gotoxy( button->x + pressed + ( button->width - (int) strlen( button->label ) ) / 2, button->y );
        cputs( button->label );

        if( !pressed ) {
            textcolor( 0 );
            textbackground( 7 );
            gotoxy( button->x + button->width, button->y );
            cputs( "\xdc" );
            gotoxy( button->x + 1, button->y + 1 );
            for( int j = 0; j < button->width; ++j ) {
                cputs( "\xdf" );
            }
        }
    }
}


int internal_tracker_update_buttons( struct tracker_t* tracker ) {
    if( !internal_tracker_key_released( tracker, KEY_LBUTTON ) ) {
        return 0;
    }

    for( int i = 0; i < tracker->button_count; ++i ) {
        struct internal_tracker_button_t* button = &tracker->buttons[ i ];
        int hover = tracker->mouse_x >= button->x && tracker->mouse_x < button->x + button->width && tracker->mouse_y == button->y;
        if( hover ) {
            return button->action;
        }
    }

    return 0;
}


int internal_tracker_add_label( struct tracker_t* tracker, int x, int y, char const* label_text ) {
    if( tracker->label_count >= sizeof( tracker->labels ) / sizeof( *tracker->labels ) ) {
        return -1;
    }

    int index = tracker->label_count++;
    struct internal_tracker_label_t* label = &tracker->labels[ index ];
    label->x = x;
    label->y = y;
    strcpy( label->label, label_text );
    return index;
}


void internal_tracker_draw_labels( struct tracker_t* tracker ) {    
    for( int i = 0; i < tracker->label_count; ++i ) {
        struct internal_tracker_label_t* label = &tracker->labels[ i ];
        textcolor( 15 );
        textbackground( 7 );
        gotoxy( label->x, label->y );
        cputs( label->label );
    }
}



int internal_tracker_add_numedit( struct tracker_t* tracker, int x, int y, int width, int* value ) {
    if( tracker->numedit_count >= sizeof( tracker->numedits ) / sizeof( *tracker->numedits ) ) {
        return -1;
    }

    int index = tracker->numedit_count++;
    struct internal_tracker_numedit_t* numedit = &tracker->numedits[ index ];
    numedit->x = x;
    numedit->y = y;
    numedit->width = width;
    numedit->value = value;
    return index;
}


void internal_tracker_draw_numedits( struct tracker_t* tracker ) {    
    for( int i = 0; i < tracker->numedit_count; ++i ) {
        struct internal_tracker_numedit_t* numedit = &tracker->numedits[ i ];
        textcolor( 15 );
        textbackground( 1 );
        char str[ 32 ];
        sprintf( str, "%d", *numedit->value );
        clear( numedit->x, numedit->y, numedit->width, 1 );
        gotoxy( numedit->x + numedit->width - (int)strlen( str ) - 1, numedit->y );
        cputs( str );
    }
}


void on_pattern_change( int value, void* user_data ) {
    struct tracker_t* tracker = (struct tracker_t*) user_data;
    tracker->song[ tracker->song_pos - 1 ] = value;
}


void on_position_change( int value, void* user_data ) {
    struct tracker_t* tracker = (struct tracker_t*) user_data;
    tracker->current_pattern = tracker->song[ value - 1 ];
}


struct tracker_t* tracker_create( void ) {
    struct tracker_t* tracker = (struct tracker_t*) malloc( sizeof( struct tracker_t ) );
    memset( tracker, 0, sizeof( *tracker ) );

    tracker->pattern_length = 64;
    tracker->song_length = 1;
    tracker->song_pos = 1;
    tracker->current_velocity = 127;
    tracker->beats_per_minute = 125;
    tracker->base_octave = 2;
    tracker->current_program = 0;
    tracker->channel_count = INTERNAL_TRACKER_MAX_CHANNEL_COUNT;
    tracker->play_state = INTERNAL_TRACKER_PLAY_STATE_STOPPED;

    int ticks_per_division = 6;
    int divisions_per_minute = ( 24 * tracker->beats_per_minute ) / ticks_per_division;
    tracker->step_time_us = ( 60 * 1000000 ) / divisions_per_minute;

    tracker->curs_x = 6;
    tracker->curs_y = 17;

    tracker->midi_chan_info[ 9 ].drums = 1;
    setinstrument( 9, 128 );

    
    internal_tracker_add_label( tracker, 2, 1, "Position" );
    internal_tracker_add_numedit( tracker, 11, 1, 5, &tracker->song_pos );
    internal_tracker_add_spinner( tracker, 16, 1, 1, 999, &tracker->song_pos, on_position_change, tracker );

    internal_tracker_add_label( tracker, 2, 2, "Pattern" );
    internal_tracker_add_numedit( tracker, 11, 2, 5, &tracker->current_pattern );
    internal_tracker_add_spinner( tracker, 16, 2, 0, 99, &tracker->current_pattern, on_pattern_change, tracker );

    internal_tracker_add_label( tracker, 2, 3, "Length" );
    internal_tracker_add_numedit( tracker, 11, 3, 5, &tracker->song_length );
    internal_tracker_add_spinner( tracker, 16, 3, 1, 999, &tracker->song_length, NULL, NULL );

    internal_tracker_add_label( tracker, 2, 4, "Tempo" );
    internal_tracker_add_numedit( tracker, 11, 4, 5, &tracker->beats_per_minute );
    internal_tracker_add_spinner( tracker, 16, 4, 1, 32, &tracker->beats_per_minute, NULL, NULL );

    internal_tracker_add_label( tracker, 2, 7, "Program" );
    internal_tracker_add_spinner( tracker, 15, 7, 0, 128, &tracker->current_program, NULL, NULL );
    
    internal_tracker_add_label( tracker, 48, 7, "Velocity" );
    internal_tracker_add_numedit( tracker, 57, 7, 5, &tracker->current_velocity );
    internal_tracker_add_spinner( tracker, 62, 7, 0, 127, &tracker->current_velocity, NULL, NULL );

    internal_tracker_add_label( tracker, 66, 7, "Octave" );
    internal_tracker_add_numedit( tracker, 73, 7, 3, &tracker->base_octave );
    internal_tracker_add_spinner( tracker, 76, 7, 0, 8, &tracker->base_octave, NULL, NULL );

    internal_tracker_add_button( tracker, 20, 1, 10, "Load", INTERNAL_TRACKER_ACTION_LOAD );
    internal_tracker_add_button( tracker, 20, 3, 10, "Save", INTERNAL_TRACKER_ACTION_SAVE );

    internal_tracker_add_button( tracker, 32, 1, 16, "Play song", INTERNAL_TRACKER_ACTION_PLAY_SONG );
    internal_tracker_add_button( tracker, 32, 3, 16, "Play pattern", INTERNAL_TRACKER_ACTION_PLAY_PATTERN );

    return tracker;
}


void tracker_destroy( struct tracker_t* tracker ) {
    free( tracker );
}


int internal_tracker_set_program( struct tracker_t* tracker, int channel, int program ) {    
    // for drums, use drum channel only
    if( program == 128 ) {
        for( int i = 0; i < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++i ) {
            if( tracker->midi_chan_info[ i ].drums ) {
                return i;
            }
        }
        return 9; 
    }

    // find channel already allocated
    int oldest_index = 0;
    int oldest_age = tracker->midi_chan_info[ 0 ].age;
    for( int i = 0; i < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++i ) {
        if( tracker->midi_chan_info[ i ].channel == channel && tracker->midi_chan_info[ i ].program == program ) {
            tracker->midi_chan_info[ i ].age = 0;
            return i;
        } else if( tracker->midi_chan_info[ i ].age > oldest_age ) {
            oldest_index = i;
            oldest_age = tracker->midi_chan_info[ i ].age;
        }
    }

    // allocate oldest channel
    tracker->midi_chan_info[ oldest_index ].age = 0;
    tracker->midi_chan_info[ oldest_index ].channel = channel;
    tracker->midi_chan_info[ oldest_index ].program = program;
    setinstrument( oldest_index, program );
    return oldest_index;
}


void internal_tracker_save_song( struct tracker_t* tracker, char const* filename ) {
    FILE* fp = fopen( filename, "wb" );
    if( !fp ) {
        // TODO: error message
        return;
    }

    char const* header = "BLASTTRACKERSONG";
    int version = 0;
    fwrite( header, strlen( header ), 1, fp );
    fwrite( &version, sizeof( version ), 1, fp );
    fwrite( &tracker->pattern_length, sizeof( tracker->pattern_length ), 1, fp );
    int pattern_count = 0;
    int pattern_pos = ftell( fp );
    fwrite( &pattern_count, sizeof( pattern_count ), 1, fp );
    for( int i = 0; i < sizeof( tracker->patterns ) / sizeof( *tracker->patterns ); ++i ) {
        struct internal_tracker_pattern_t* pattern = &tracker->patterns[ i ];
        int is_empty = 1;
        for( int j = 0; j < tracker->channel_count; ++j ) {
            struct internal_tracker_channel_t* channel = &pattern->channels[ j ];
            for( int k = 0; k < tracker->pattern_length; ++k ) {                
                struct internal_tracker_note_t* note = &channel->notes[ k ];
                if( note->note || note->program || note->velocity || note->cc_command || note->cc_data ) {
                    is_empty = 0;
                    goto exit_loops;
                }
            }
        }
        exit_loops:

        if( !is_empty ) {
            ++pattern_count;
            fwrite( &i, sizeof( i ), 1, fp );
            for( int j = 0; j < tracker->channel_count; ++j ) {
                struct internal_tracker_channel_t* channel = &pattern->channels[ j ];
                for( int k = 0; k < tracker->pattern_length; ++k ) {                
                    struct internal_tracker_note_t* note = &channel->notes[ k ];
                    fwrite( &note->note, sizeof( note->note ), 1, fp );
                    fwrite( &note->program, sizeof( note->program ), 1, fp );
                    fwrite( &note->velocity, sizeof( note->velocity ), 1, fp );
                    fwrite( &note->cc_command, sizeof( note->cc_command ), 1, fp );
                    fwrite( &note->cc_data, sizeof( note->cc_data ), 1, fp );
                }
            }
        }
    }

    int current_pos = ftell( fp );
    fseek( fp, pattern_pos, SEEK_SET );
    fwrite( &pattern_count, sizeof( pattern_count ), 1, fp );
    fseek( fp, current_pos, SEEK_SET );
    
    fwrite( &tracker->song_length, sizeof( tracker->song_length ), 1, fp );
    for( int i = 0; i < tracker->song_length; ++i ) {
        fwrite( &tracker->song[ i ], sizeof( tracker->song[ i ] ), 1, fp );
    }

    fclose( fp );
}


void internal_tracker_load_song( struct tracker_t* tracker, char const* filename ) {
    (void) tracker;
    FILE* fp = fopen( filename, "rb" );
    if( !fp ) {
        // TODO: error message
        return;
    }

    char header[ sizeof( "BLASTTRACKERSONG" ) ] = "";
    int version = 0;
    fread( header, sizeof( header ) - 1, 1, fp );
    fread( &version, sizeof( version ), 1, fp );
    fread( &tracker->pattern_length, sizeof( tracker->pattern_length ), 1, fp );
    int pattern_count = 0;
    memset( tracker->patterns, 0, sizeof( tracker->patterns ) );
    fread( &pattern_count, sizeof( pattern_count ), 1, fp );
    for( int i = 0; i < pattern_count; ++i ) {
        int index = 0;
        fread( &index, sizeof( index ), 1, fp );       
        struct internal_tracker_pattern_t* pattern = &tracker->patterns[ index ];
        for( int j = 0; j < tracker->channel_count; ++j ) {
            struct internal_tracker_channel_t* channel = &pattern->channels[ j ];
            for( int k = 0; k < tracker->pattern_length; ++k ) {                
                struct internal_tracker_note_t* note = &channel->notes[ k ];
                fread( &note->note, sizeof( note->note ), 1, fp );
                fread( &note->program, sizeof( note->program ), 1, fp );
                fread( &note->velocity, sizeof( note->velocity ), 1, fp );
                fread( &note->cc_command, sizeof( note->cc_command ), 1, fp );
                fread( &note->cc_data, sizeof( note->cc_data ), 1, fp );
            }
        }
    }
    
    fread( &tracker->song_length, sizeof( tracker->song_length ), 1, fp );
    for( int i = 0; i < tracker->song_length; ++i ) {
        fread( &tracker->song[ i ], sizeof( tracker->song[ i ] ), 1, fp );
    }

    fclose( fp );
}


void internal_tracker_load( struct tracker_t* tracker ) {
    (void) tracker;
    /*
    #ifdef _WIN32 
        (void) tracker;
        OPENFILENAMEA ofn = { sizeof( OPENFILENAMEA ) };
        CHAR szFile[ 260 ] = { 0 };

        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "BlastTracker Song (*.bts)\0*.BTS\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if( GetOpenFileNameA( &ofn ) ) {
            internal_tracker_load_song( tracker, szFile );
        }
    #else
        #error "Unsupported platform"
    #endif
    */
}


void internal_tracker_save( struct tracker_t* tracker ) {
    (void) tracker;
     /*
    #ifdef _WIN32 
        OPENFILENAMEA ofn = { sizeof( OPENFILENAMEA ) };
        CHAR szFile[ 260 ] = { 0 };

        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "BlastTracker Song (*.bts)\0*.BTS\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrDefExt = "bts";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if( GetSaveFileNameA( &ofn ) ) {
            internal_tracker_save_song( tracker, szFile );
        }
    #else
        #error "Unsupported platform"
    #endif
    */
}


void internal_tracker_play_pattern_pos( struct tracker_t* tracker) {

    struct chords_t {
        int note;
        int chord[ 7 ];
    } chords[ 2 ][ 12 ] = {
        /* Major */ { 
            {  0 /* C  */, {  0,  4,  7, -1, -1, -1, -1 } },
            {  1 /* C# */, {  1,  5,  8, -1, -1, -1, -1 } },
            {  2 /* D  */, {  2,  6,  9, -1, -1, -1, -1 } },
            {  3 /* D# */, {  3,  7, 10, -1, -1, -1, -1 } },
            {  4 /* E  */, {  4,  8, 11, -1, -1, -1, -1 } },
            {  5 /* F  */, {  5,  9, 12, -1, -1, -1, -1 } },
            {  6 /* F# */, {  6, 10, 13, -1, -1, -1, -1 } },
            {  7 /* G  */, {  7, 11, 14, -1, -1, -1, -1 } },
            {  8 /* G# */, {  8, 12, 15, -1, -1, -1, -1 } },
            {  9 /* A  */, {  9, 13, 16, -1, -1, -1, -1 } },
            { 10 /* A# */, { 10, 14, 17, -1, -1, -1, -1 } },
            { 11 /* B  */, { 11, 15, 18, -1, -1, -1, -1 } },
        },                      
        /* Minor */ { 
            {  0 /* C  */, {  0,  3,  7, -1, -1, -1, -1 } },
            {  1 /* C# */, {  1,  4,  8, -1, -1, -1, -1 } },
            {  2 /* D  */, {  2,  5,  9, -1, -1, -1, -1 } },
            {  3 /* D# */, {  3,  6, 10, -1, -1, -1, -1 } },
            {  4 /* E  */, {  4,  7, 11, -1, -1, -1, -1 } },
            {  5 /* F  */, {  5,  8, 12, -1, -1, -1, -1 } },
            {  6 /* F# */, {  6,  9, 13, -1, -1, -1, -1 } },
            {  7 /* G  */, {  7, 10, 14, -1, -1, -1, -1 } },
            {  8 /* G# */, {  8, 11, 15, -1, -1, -1, -1 } },
            {  9 /* A  */, {  9, 12, 16, -1, -1, -1, -1 } },
            { 10 /* A# */, { 10, 13, 17, -1, -1, -1, -1 } },
            { 11 /* B  */, { 11, 14, 18, -1, -1, -1, -1 } },
        },      
                
    };          

    int jump = 0;
    for( int i = 0; i < tracker->channel_count; ++i ) {
        struct internal_tracker_note_t* note = &tracker->patterns[ tracker->current_pattern ].channels[ i ].notes[ tracker->pattern_pos ];
        if( note->note ) {
            for( int j = 0; j < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++j ) {
                if( tracker->midi_chan_info[ j ].channel == i ) {
                    allnotesoff( j );
                }
            }
            if( note->note < 128 ) {
                int midi_channel = internal_tracker_set_program( tracker, i, note->program );
                if( note->cc_command == 5 && note->cc_data >= 0 && note->cc_data < sizeof( chords ) / sizeof( *chords ) ) {
                    struct chords_t* chord = &chords[ note->cc_data ][ note->note % 12 ];
                    for( int k = 0; k < sizeof( chord->chord ) / sizeof( *chord->chord ); ++k ) {
                        if( chord->chord[ k ] >= 0 ) {
                            int n = note->note - ( note->note % 12 ) + chord->chord[ k ];
                            noteon( midi_channel, n, note->velocity );
                        }
                    }
                } else {
                    noteon(midi_channel, note->note, note->velocity );
                }
            }
        }
        if( note->cc_command == 6 ) {
            jump = 1;
        }
        if( note->cc_command == 7 ) {
            tracker->step_time_us = note->cc_data * 1000;
        }
    }
    if( jump ) {
        tracker->pattern_pos = 0;
        ++tracker->song_pos;
        if( tracker->song_pos > tracker->song_length ) {
            tracker->song_pos = 1;
        }
        tracker->current_pattern = tracker->song[ tracker->song_pos - 1 ];
    } else {
        ++tracker->pattern_pos;
        if( tracker->pattern_pos >= tracker->pattern_length ) {
            tracker->pattern_pos = 0;
            ++tracker->song_pos;
            if( tracker->song_pos > tracker->song_length ) {
                tracker->song_pos = 1;
            }
            tracker->current_pattern = tracker->song[ tracker->song_pos - 1 ];
        }
    }
}


void tracker_update_play( struct tracker_t* tracker, int delta_time_us ) {
    if( tracker->play_state != INTERNAL_TRACKER_PLAY_STATE_STOPPED ) {
        int time = tracker->play_time_us + delta_time_us;
        while( time >= tracker->step_time_us ) {
            time -= tracker->step_time_us;
            internal_tracker_play_pattern_pos( tracker );
        }
        tracker->play_time_us = time;
    }
}


void tracker_update( struct tracker_t* tracker ) {
    struct keyboard_note_map_t {
        enum keycode_t key;
        int note;
    } keyboard_note_map[] = {
        { KEY_Z,           0 }, // C
        { KEY_S,           1 }, // C#
        { KEY_X,           2 }, // D
        { KEY_D,           3 }, // D#
        { KEY_C,           4 }, // E
        { KEY_V,           5 }, // F
        { KEY_G,           6 }, // F#
        { KEY_B,           7 }, // G
        { KEY_H,           8 }, // G#
        { KEY_N,           9 }, // A
        { KEY_J,          10 }, // A#
        { KEY_M,          11 }, // B
        { KEY_OEM_COMMA,  12 }, // C
        { KEY_L,          13 }, // C#
        { KEY_OEM_PERIOD, 14 }, // D
        { KEY_OEM_1,      15 }, // D#
        { KEY_OEM_2,      16 }, // E

        { KEY_Q,          12 }, // C
        { KEY_2,          13 }, // C#
        { KEY_W,          14 }, // D
        { KEY_3,          15 }, // D#
        { KEY_E,          16 }, // E
        { KEY_R,          17 }, // F
        { KEY_5,          18 }, // F#
        { KEY_T,          19 }, // G
        { KEY_6,          20 }, // G#
        { KEY_Y,          21 }, // A
        { KEY_7,          22 }, // A#
        { KEY_U,          23 }, // B
        { KEY_I,          24 }, // C
        { KEY_9,          25 }, // C#
        { KEY_O,          26 }, // D
        { KEY_0,          27 }, // D#
        { KEY_P,          28 }, // E
        { KEY_OEM_4,      29 }, // F
        { KEY_OEM_PLUS,   30 }, // F#
    };

    tracker->mouse_x = mousex();
    tracker->mouse_y = mousey();
    memcpy( tracker->prev_keyboard_state, tracker->keyboard_state, sizeof( tracker->prev_keyboard_state ) );
    for( int i = 0; i < KEYCOUNT; ++i ) {
        if( !keystate( (enum keycode_t ) i ) ) {
            tracker->keyboard_state[ i ] = 0;
        }
    }
    for( enum keycode_t* keys = readkeys(); *keys != KEY_INVALID; ++keys ) {
        enum keycode_t key = *keys;
        int index = (int) key;
        tracker->keyboard_state[ index ] = 1;
        int in_key_map = 0;
        for( int j = 0; j < sizeof( keyboard_note_map ) / sizeof( *keyboard_note_map ); ++j ) {
            if( keyboard_note_map[ j ].key == key ) {
                in_key_map = 1;
                break;
            }
        }
        if( !in_key_map ) {
            tracker->prev_keyboard_state[ index ] = 0;
        }
    }

    for( int i = 0; i < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++i ) {
        ++tracker->midi_chan_info[ i ].age;
    }

    internal_tracker_update_spinners( tracker );

    int action = internal_tracker_update_buttons( tracker );
    if( action == INTERNAL_TRACKER_ACTION_LOAD ) {
        internal_tracker_load( tracker );
    } else if( action == INTERNAL_TRACKER_ACTION_SAVE ) {
        internal_tracker_save( tracker );
    }

    if( tracker->play_state != INTERNAL_TRACKER_PLAY_STATE_STOPPED ) {
        if( internal_tracker_key_pressed( tracker, KEY_SPACE ) ) {
            tracker->play_state = INTERNAL_TRACKER_PLAY_STATE_STOPPED;
            for( int i = 0; i < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++i ) {
                allnotesoff( i );
            }
            return;
        }
    }


    int min_x = 6 + 18 * tracker->current_channel;
    int max_x = min_x + 16;
    int offs = tracker->curs_x - min_x;


    for( int i = 0; i < sizeof( keyboard_note_map ) / sizeof( *keyboard_note_map ); ++i ) {
        int note_num = tracker->base_octave * 12 + keyboard_note_map[ i ].note;

        if( internal_tracker_key_pressed( tracker, keyboard_note_map[ i ].key ) && 
            ( tracker->current_program < 128 || ( note_num >= 35 && note_num <= 81 ) ) ) {
            int midi_channel = internal_tracker_set_program( tracker, tracker->current_channel, tracker->current_program );
                noteon( midi_channel, note_num, 127 );
            if( tracker->edit_mode && tracker->play_state == INTERNAL_TRACKER_PLAY_STATE_STOPPED && offs < 3 ) {
                struct internal_tracker_note_t* note = &tracker->patterns[ tracker->current_pattern ].channels[ tracker->current_channel ].notes[ tracker->pattern_pos ];
                
                note->note = note_num;
                note->program = tracker->current_program;
                note->velocity = tracker->current_velocity;
                tracker->pattern_pos = ( tracker->pattern_pos + 1 ) % ( tracker->pattern_length );
            }
        } else if( internal_tracker_key_released( tracker, keyboard_note_map[ i ].key ) ) {
            for( int j = 0; j < sizeof( tracker->midi_chan_info ) / sizeof( *tracker->midi_chan_info ); ++j ) {
                if( tracker->midi_chan_info[ j ].channel == tracker->current_channel ) {
                    noteoff( j, note_num );
                }
            }
        } 

    }

    if( tracker->play_state != INTERNAL_TRACKER_PLAY_STATE_STOPPED ) return;

    if( ( !internal_tracker_key_down( tracker, KEY_SHIFT ) && internal_tracker_key_pressed( tracker, KEY_SPACE ) ) || action == INTERNAL_TRACKER_ACTION_PLAY_PATTERN ) {
        tracker->play_state = INTERNAL_TRACKER_PLAY_STATE_PLAYING_PATTERN;
        tracker->pattern_pos = 0;
        tracker->play_time_us = 0;
        internal_tracker_play_pattern_pos( tracker );
    }

    if( ( internal_tracker_key_down( tracker, KEY_SHIFT ) && internal_tracker_key_pressed( tracker, KEY_SPACE ) ) || action == INTERNAL_TRACKER_ACTION_PLAY_SONG ) {
        tracker->play_state = INTERNAL_TRACKER_PLAY_STATE_PLAYING_SONG;
        tracker->song_pos = 1;
        tracker->current_pattern = tracker->song[ tracker->song_pos - 1 ];
        tracker->pattern_pos = 0;
        tracker->play_time_us = 0;
        internal_tracker_play_pattern_pos( tracker );
    }

    if( internal_tracker_key_pressed( tracker, KEY_RETURN ) ) {
        tracker->edit_mode = !tracker->edit_mode;
    }

    if( !internal_tracker_key_down( tracker, KEY_SHIFT ) && internal_tracker_key_pressed( tracker, KEY_TAB ) && tracker->edit_mode ) {
        tracker->current_channel = ( tracker->current_channel + 1 ) % tracker->channel_count;
        tracker->curs_x = 6 + 18 * tracker->current_channel;
    }

    if( internal_tracker_key_down( tracker, KEY_SHIFT ) && internal_tracker_key_pressed( tracker, KEY_TAB ) && tracker->edit_mode ) {
        tracker->current_channel = ( tracker->current_channel - 1 + tracker->channel_count ) % tracker->channel_count;
        tracker->curs_x = 6 + 18 * tracker->current_channel;
    }

    if( internal_tracker_key_pressed( tracker, KEY_UP ) ) {
        tracker->pattern_pos = ( tracker->pattern_pos - 1 + tracker->pattern_length ) % tracker->pattern_length;
    }

    if( internal_tracker_key_pressed( tracker, KEY_DOWN ) ) {
        tracker->pattern_pos = ( tracker->pattern_pos + 1 ) % tracker->pattern_length;
    }

    if( internal_tracker_key_pressed( tracker, KEY_PRIOR ) ) {
        tracker->current_program = ( tracker->current_program - 1 + 129 ) % 129;
    }

    if( internal_tracker_key_pressed( tracker, KEY_NEXT ) ) {
        tracker->current_program = ( tracker->current_program + 1 ) % 129;
    }

    if( internal_tracker_key_pressed( tracker, KEY_SUBTRACT ) ) {
        tracker->base_octave = ( tracker->base_octave - 1 + 9 ) % 9;
    }

    if( internal_tracker_key_pressed( tracker, KEY_ADD ) ) {
        tracker->base_octave = ( tracker->base_octave + 1 ) % 9;
    }

    if( tracker->edit_mode ) {
        if( internal_tracker_key_pressed( tracker, KEY_LEFT ) ) {
            --tracker->curs_x;           
            if( offs == 4 || offs == 8 || offs == 12 || offs == 14 ) {
                --tracker->curs_x;
            }
            if( tracker->curs_x < min_x ) {
                tracker->current_channel = ( tracker->current_channel - 1 + tracker->channel_count ) % tracker->channel_count;
                tracker->curs_x = 6 + 18 * tracker->current_channel + 16;
            }
        }

        if( internal_tracker_key_pressed( tracker, KEY_RIGHT ) ) {
            ++tracker->curs_x;
            if( offs == 2 || offs == 6 || offs == 10 || offs == 12 ) {
                ++tracker->curs_x;
            }
            if( tracker->curs_x > max_x ) {
                tracker->current_channel = ( tracker->current_channel  + 1 ) % tracker->channel_count;
                tracker->curs_x = 6 + 18 * tracker->current_channel;
            }
        }

        if( internal_tracker_key_pressed( tracker, KEY_DELETE ) ) {
            struct internal_tracker_note_t* note = &tracker->patterns[ tracker->current_pattern ].channels[ tracker->current_channel ].notes[ tracker->pattern_pos ];
            note->note = 0;
            note->program = 0;
            note->velocity = 0;
            note->cc_command = 0;
            note->cc_data = 0;
        }

        if( internal_tracker_key_pressed( tracker, KEY_INSERT ) ) {
            struct internal_tracker_note_t* note = &tracker->patterns[ tracker->current_pattern ].channels[ tracker->current_channel ].notes[ tracker->pattern_pos ];
            note->note = 128;
        }

        int num = -1;
        if( internal_tracker_key_pressed( tracker, KEY_0 ) ) num = 0;
        if( internal_tracker_key_pressed( tracker, KEY_1 ) ) num = 1;
        if( internal_tracker_key_pressed( tracker, KEY_2 ) ) num = 2;
        if( internal_tracker_key_pressed( tracker, KEY_3 ) ) num = 3;
        if( internal_tracker_key_pressed( tracker, KEY_4 ) ) num = 4;
        if( internal_tracker_key_pressed( tracker, KEY_5 ) ) num = 5;
        if( internal_tracker_key_pressed( tracker, KEY_6 ) ) num = 6;
        if( internal_tracker_key_pressed( tracker, KEY_7 ) ) num = 7;
        if( internal_tracker_key_pressed( tracker, KEY_8 ) ) num = 8;
        if( internal_tracker_key_pressed( tracker, KEY_9 ) ) num = 9;

        if( num >=0 && offs >= 4 ) {
            struct internal_tracker_note_t* note = &tracker->patterns[ tracker->current_pattern ].channels[ tracker->current_channel ].notes[ tracker->pattern_pos ];
            if( offs >= 4 && offs <= 6 && note->note > 0 ) {
                int val = note->program + 1;
                int a = offs == 4 ? num : ( val / 100 ) % 10;
                int b = offs == 5 ? num : ( val /  10 ) % 10;
                int c = offs == 6 ? num : ( val /   1 ) % 10;
                val = 100 * a + 10 * b + c;
                if( val >= 1 && val <= 129 ) {
                    note->program = val - 1;
                }
            }
            else if( offs >= 8 && offs <= 10 && note->note > 0  ) {
                int val = note->velocity;
                int a = offs == 8 ? num : ( val / 100 ) % 10;
                int b = offs == 9 ? num : ( val /  10 ) % 10;
                int c = offs == 10 ? num : ( val /   1 ) % 10;
                val = 100 * a + 10 * b + c;
                if( val >= 0 && val < 128 ) {
                    note->velocity = val;
                }
            }
            else if( offs == 12 ) {
                int val = note->cc_command;
                val = num;
                if( val >= 0 && val <= 9 ) {
                    note->cc_command = val;
                }
            }
            else if( offs >= 14 && offs <= 16 ) {
                int val = note->cc_data;
                int a = offs == 14 ? num : ( val / 100 ) % 10;
                int b = offs == 15 ? num : ( val /  10 ) % 10;
                int c = offs == 16 ? num : ( val /   1 ) % 10;
                val = 100 * a + 10 * b + c;
                if( val >= 0 && val <= 999 ) {
                    note->cc_data = val;
                }
            }
            ++tracker->curs_x;
            if( offs == 2 || offs == 6 || offs == 10 || offs == 12 ) {
                ++tracker->curs_x;
            }
            if( tracker->curs_x > max_x ) {
                tracker->current_channel = ( tracker->current_channel  + 1 ) % tracker->channel_count;
                tracker->curs_x = 6 + 18 * tracker->current_channel;
            }
        }

    }
}


void tracker_draw( struct tracker_t* tracker ) {
    textcolor( 15 );
    textbackground( 7 );
    clrscr();


    char const* programs[129] = {
        "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano", 
        "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavinet", "Celesta", "Glockenspiel", "Music Box",
        "Vibraphone", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer", "Drawbar Organ", "Percussive Organ",
        "Rock Organ", "Church Organ", "Reed Organ", "Accordion", "Harmonica", "Tango Accordion", 
        "Acoustic Guitar (nylon)", "Acoustic Guitar (steel)", "Electric Guitar (jazz)", "Electric Guitar (clean)",
        "Electric Guitar (muted)", "Overdriven Guitar", "Distortion Guitar", "Guitar harmonics", "Acoustic Bass",
        "Electric Bass (finger)", "Electric Bass (pick)", "Fretless Bass", "Slap Bass 1", "Slap Bass 2", 
        "Synth Bass 1", "Synth Bass 2", "Violin", "Viola", "Cello", "Contrabass", "Tremolo Strings", 
        "Pizzicato Strings", "Orchestral Harp", "Timpani", "String Ensemble 1", "String Ensemble 2", 
        "Synth Strings 1", "Synth Strings 2", "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit", "Trumpet",
        "Trombone", "Tuba", "Muted Trumpet", "French Horn", "Brass Section", "Synth Brass 1", "Synth Brass 2",
        "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax", "Oboe", "English Horn", "Bassoon", "Clarinet",
        "Piccolo", "Flute", "Recorder", "Pan Flute", "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina", 
        "Synth Lead 1 (square)", "Synth Lead 2 (sawtooth)", "Synth Lead 3 (calliope)", "Synth Lead 4 (chiff)",
        "Synth Lead 5 (charang)", "Synth Lead 6 (voice)", "Synth Lead 7 (fifths)", "Synth Lead 8 (bass + lead)",
        "Synth Pad 1 (new age)", "Synth Pad 2 (warm)", "Synth Pad 3 (polysynth)", "Synth Pad 4 (choir)",
        "Synth Pad 5 (bowed)", "Synth Pad 6 (metallic)", "Synth Pad 7 (halo)", "Synth Pad 8 (sweep)",
        "Synth FX 1 (rain)", "Synth FX 2 (soundtrack)", "Synth FX 3 (crystal)", "Synth FX 4 (atmosphere)",
        "Synth FX 5 (brightness)", "Synth FX 6 (goblins)", "Synth FX 7 (echoes)", "Synth FX 8 (sci-fi)", "Sitar",
        "Banjo", "Shamisen", "Koto", "Kalimba", "Bag pipe", "Fiddle", "Shanai", "Tinkle Bell", "Agogo", 
        "Steel Drums", "Woodblock", "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal", 
        "Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet", "Telephone Ring", "Helicopter", "Applause",
        "Gunshot", "DRUMS",
    };

    /*
    char const* drums[128] = {
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
        "", "", "", "", "", "", "", "", "Acoustic Bass Drum", "Bass Drum 1", "Side Stick/Rimshot", "Acoustic Snare", 
        "Hand Clap", "Electric Snare", "Low Floor Tom", "Closed Hi-hat", "High Floor Tom", "Pedal Hi-hat", "Low Tom", 
        "Open Hi-hat", "Low-Mid Tom", "Hi-Mid Tom", "Crash Cymbal 1", "High Tom", "Ride Cymbal 1", "Chinese Cymbal", 
        "Ride Bell", "Tambourine", "Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibra Slap", "Ride Cymbal 2", 
        "High Bongo", "Low Bongo", "Mute High Conga", "Open High Conga", "Low Conga", "High Timbale", "Low Timbale", 
        "High Agogo", "Low Agogo", "Cabasa", "Maracas", "Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro", 
        "Claves", "High Wood Block", "Low Wood Block", "Mute Cuica", "Open Cuica", "Mute Triangle", "Open Triangle", 
    };
    (void) drums;
    */

    char const* drums_short[128] = {
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
        "", "", "", "", "", "", "", "", "AcBassD", "BassDr1", "SideStk", "AcSnare", "HndClap", "ElSnare", "LoFlTom", 
        "ClHiHat", "HiFlTom", "PdHihat", "Low Tom", "OpHiHat", "LoMdTom", "HiMdTom", "CrCymb1", "HighTom", 
        "RdCymb1", "ChiCymb", "RideBel", "Tambour", "SplCymb", "Cowbell", "CrCymb2", "VibrSlp", "RdCymb2", 
        "HiBongo", "LoBongo", "MuHiCon", "OpHiCon", "LoConga", "HiTimba", "LoTimba", "HiAgogo", "LoAgogo", 
        "Cabasa ", "Maracas", "ShWhist", "LnWhist", "ShGuiro", "LnGuiro", "Claves ", "HiWBlck", "LoWBlck", 
        "MuCuica", "OpCuica", "MuteTri", "OpenTri", 
    };

    char const* notes[129] = { "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
        "C-0", "C#0", "D-0", "D#0",	"E-0","F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0", "C-1", "C#1", "D-1", 
        "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1", "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", 
        "F#2", "G-2", "G#2", "A-2", "A#2", "B-2", "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", 
        "A-3", "A#3", "B-3", "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4", 
        "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5", "C-6", "C#6", "D-6", 
        "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6", "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", 
        "F#7", "G-7", "G#7", "A-7", "A#7", "B-7", "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", 
        "A-8", "A#8", "B-8", "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "off"
    };

    char str[ 32 ];


    textcolor( 15 );
    textbackground( 1 );
    gotoxy( 10, 7 );
    sprintf( str, " %3d ", tracker->current_program + 1 );
    cputs( tracker->current_program < 128 ? str : " DRM " );


    textcolor( 0 );
    textbackground( 3 );
    
    clear( 17, 7, 28, 1 );
    gotoxy( 18, 7 );
    cputs( programs[ tracker->current_program ] );


    internal_tracker_draw_labels( tracker );
    internal_tracker_draw_buttons( tracker );
    internal_tracker_draw_spinners( tracker );
    internal_tracker_draw_numedits( tracker );

    textcolor( 15 );
    textbackground( 7 );

    frame( FRAME_DOUBLE, 1, 8, 77, 17 );
    hdivider( FRAME_VDOUBLE, 1, 10, 77 );
    vdivider( FRAME_HDOUBLE, 5, 8, 17 );
    divcross( FRAME_SINGLE, 5, 10 );

    vdivider( FRAME_HDOUBLE, 5 + 18 * 1, 8, 17 );
    divcross( FRAME_SINGLE, 5 + 18 * 1, 10 );

    vdivider( FRAME_HDOUBLE, 5 + 18 * 2, 8, 17 );
    divcross( FRAME_SINGLE, 5 + 18 * 2, 10 );

    vdivider( FRAME_HDOUBLE, 5 + 18 * 3, 8, 17 );
    divcross( FRAME_SINGLE, 5 + 18 * 3, 10 );

    

    textcolor( 0 );
    gotoxy( 2, 9 );
    sprintf( str, "%03d", tracker->current_pattern );
    cputs( str );

    textcolor( 8 );
    for( int j = 0; j < tracker->channel_count; ++j ) {
        textbackground( 7 );
        textcolor( 0 );
        gotoxy( 6 + 18 * j, 9 );
        cputs( "NOT PRG VEL CC" );
    
        textcolor( 10 );
        gotoxy( 6 + 15 + 18 * j, 9 );
        cputs( "\x0E" );

        textbackground( 2 );
        textcolor( 0 );
        gotoxy( 6 + 16 + 18 * j, 9 );
        cputs( "\x1F" );
    }

    
    struct internal_tracker_pattern_t* pattern = &tracker->patterns[ tracker->current_pattern ];

    for( int i = 0; i < 13; ++i ) {
        int pos = tracker->pattern_pos - 6 + i;
        if( pos < 0 || pos >= tracker->pattern_length ) {
            continue;
        }

        textbackground( i == 6 ? ( tracker->edit_mode ? 0 : 8 )  : 7 );

        textcolor( i == 6 ? ( tracker->edit_mode ? 14 : 7 ) : 8 );
        gotoxy( 2, 11 + i );
        sprintf( str, "%03d", pos );
        cputs( str );
        
        struct internal_tracker_note_t* note;

        for( int j = 0; j < tracker->channel_count; ++j ) {
            textcolor( i == 6 ? ( tracker->edit_mode ? ( tracker->current_channel == j ? 12 : 14 ) : 7 ) : 0 );
            note = &pattern->channels[ j ].notes[ pos ];
            if( note->note == 128 ) {
                sprintf( str, "%s         %01d %03d", notes[ note->note ], note->cc_command, note->cc_data );
            } else if( note->note == 0 || note->program < 128 ) {
                sprintf( str, "%s %03d %03d %01d %03d", notes[ note->note ], note->note ? note->program + 1 : 0, note->velocity, note->cc_command, note->cc_data );
            } else {
                sprintf( str, "%s %03d %01d %03d", drums_short[ note->note ], note->velocity, note->cc_command, note->cc_data );
            }

            gotoxy( 6 + 18 * j, 11 + i );
            cputs( str );
        }
    }

    if( tracker->edit_mode ) {
        curson();
        gotoxy( tracker->curs_x, tracker->curs_y );
    } else {
        cursoff();
    }

    if( tracker->mouse_x >= 0 && tracker->mouse_x < screenwidth() && tracker->mouse_y >= 0 && tracker->mouse_y < screenheight() ) {    
        unsigned char* scr = screenbuffer();
        int ofs = tracker->mouse_x + tracker->mouse_y * screenwidth();
        unsigned char v = scr[ ofs * 2 + 1 ];
        v = ~v;
        scr[ ofs * 2 + 1 ] = v;
    }
}


void setcell( int x, int y, char ascii ) {
    if( x < 0 || x >= screenwidth() || y < 0 || y >= screenheight() ) {
        return;
    }
    int old_x = wherex();
    int old_y = wherey();
    gotoxy( x, y );
    char str[ 2 ] = { 0, 0 };
    *str = ascii;
    cputs( str );
    gotoxy( old_x, old_y );
}


void clear( int x, int y, int w, int h ) {   
    for( int iy = y; iy < y + h; ++iy ) {
        for( int ix = x; ix < x + w; ++ix ) {
            setcell( ix, iy, ' ' );
        }
    }
}


typedef unsigned short u16;

void frame( enum frame_t type, int x, int y, int w, int h ) {
    char tl[] = { '\xDA', '\xD5', '\xD6', '\xC9', '\xDB', };
    char t [] = { '\xC4', '\xCD', '\xC4', '\xCD', '\xDB', };
    char tr[] = { '\xBF', '\xB8', '\xB7', '\xBB', '\xDB', };
    char l [] = { '\xB3', '\xB3', '\xBA', '\xBA', '\xDB', };
    char r [] = { '\xB3', '\xB3', '\xBA', '\xBA', '\xDB', };
    char bl[] = { '\xC0', '\xD4', '\xD3', '\xC8', '\xDB', };
    char b [] = { '\xC4', '\xCD', '\xC4', '\xCD', '\xDB', };
    char br[] = { '\xD9', '\xBE', '\xBD', '\xBC', '\xDB', };
    
    if( type < 0 || type >= sizeof( tl ) / sizeof( *tl ) ) {
        return;
    }
    --w; --h;

    setcell( x, y, tl[ type ] );
    for( int i = 1; i < w; ++i ) setcell( x + i, y, t[ type ] );
    setcell( x + w, y, tr[ type ] );

    for( int i = 1; i < h; ++i ) {
        setcell( x, y + i, l[ type ] );
        setcell( x + w, y + i, r[ type ] );
    }

    setcell( x, y + h, bl[ type ] );
    for( int i = 1; i < w; ++i ) setcell( x + i, y + h, b[ type ] );
    setcell( x + w, y + h, br[ type ] );
}



void hdivider( enum frame_t type, int x, int y, int w ) {
    char s[] = { '\xC3', '\xC6', '\xC7', '\xCC', '\xDB', };
    char m[] = { '\xC4', '\xCD', '\xC4', '\xCD', '\xDB', };
    char e[] = { '\xB4', '\xB5', '\xB6', '\xB9', '\xDB', };

    if( type < 0 || type >= sizeof( s ) / sizeof( *s ) ) {
        return;
    }
    --w;

    setcell( x, y, s[ type ] );
    for( int i = 1; i < w; ++i ) setcell( x + i, y, m[ type ] );
    setcell( x + w, y, e[ type ] );
}


void vdivider( enum frame_t type, int x, int y, int h ) {
    char s[] = { '\xC2', '\xD1', '\xD2', '\xCB', '\xDB', };
    char m[] = { '\xB3', '\xB3', '\xBA', '\xBA', '\xDB', };
    char e[] = { '\xC1', '\xCF', '\xD0', '\xCA', '\xDB', };

    if( type < 0 || type >= sizeof( s ) / sizeof( *s ) ) {
        return;
    }
    --h;

    setcell( x, y, s[ type ] );
    for( int i = 1; i < h; ++i ) setcell( x, y + i, m[ type ] );
    setcell( x, y + h, e[ type ] );
}


void divcross( enum frame_t type, int x, int y ) {
    char c[] = { '\xC5', '\xD8', '\xD7', '\xCE', '\xDB', };

    if( type < 0 || type >= sizeof( c ) / sizeof( *c ) ) {
        return;
    }

    setcell( x, y, c[ type ] );
}


int main( int argc, char* argv[] ) {
    setvideomode( videomode_80x25_9x16 );
    setdoublebuffer( 1 );
    //setsoundbank( DEFAULT_SOUNDBANK_SB16 );

    struct tracker_t* tracker = tracker_create();
   
    while( !shuttingdown() ) {
        waitvbl();
        
        tracker_update_play( tracker, 1000000 / 60 );
        tracker_update( tracker );
        tracker_draw( tracker );
        swapbuffers();
    }

    tracker_destroy( tracker );

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
