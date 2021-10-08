struct modfile_t {
    char name[ 21 ];

    struct sample_t {
        char name[ 23 ];
        int offset;
        int length;
        int finetune;
        int volume;
        int repeat_start;
        int repeat_length;
    } samples[ 31 ];

    int song_length;
    int song[ 128 ];

    int pattern_count;
    struct pattern_t {
        struct division_t {
            struct channel_t {
                int sample;
                int note;
                int effect;
            } channels[ 4 ];
        } divisions[ 64 ];
    } patterns[ 64 ];
};


struct modfile_t* modfile_load( void* data, size_t size ) {
    struct modfile_t* modfile = (struct modfile_t*) malloc( sizeof( struct modfile_t ) );
    memset( modfile, 0, sizeof( *modfile ) );

    #define INTERNAL_MODFILE_ENDIAN_SWAP( x )  (((x&0xff00) >> 8) | ((x&0x00ff) << 8))

    uintptr_t ptr = (uintptr_t) data;
    uintptr_t end = ptr + size;
    if( ptr + 20 > end ) goto failed;
    memcpy( modfile->name, (void*) ptr, 20 ); // The module's title, padded with null (\0) bytes.
    ptr += 20;

    memset( modfile->samples, 0, sizeof( modfile->samples ) );
    int offset = 0;
    for( int i = 0; i < 31; ++i ) {
        if( ptr + 22 > end ) goto failed;        
        memcpy( modfile->samples[ i ].name, (void*) ptr, 22 ); // Sample's name, padded with null bytes.
        ptr += 22;

        if( ptr + 2 > end ) goto failed;
        unsigned short sample_len = 0;
        memcpy( &sample_len, (void*) ptr, 2 ); // Sample length in words
        if( INTERNAL_MODFILE_ENDIAN_SWAP( sample_len ) > 1 ) { // The first word of the sample is overwritten by the tracker, so a length of 1 still means an empty sample.
            modfile->samples[ i ].offset = offset;
            offset += INTERNAL_MODFILE_ENDIAN_SWAP( sample_len );
        } else {
            sample_len = 0;
        }
        modfile->samples[ i ].length = INTERNAL_MODFILE_ENDIAN_SWAP( sample_len );
        ptr += 2;

        ptr += 1; // Lowest four bits represent a signed nibble (-8..7) which is the finetune value for the sample

        if( ptr + 1 > end ) goto failed;
        unsigned char sample_vol = 0;
        memcpy( &sample_vol, (void*) ptr, 1 ); // Volume of sample. Legal values are 0..64.
        modfile->samples[ i ].volume = sample_vol;
        ptr += 1;

        if( ptr + 2 > end ) goto failed;
        unsigned short rep_start = 0;
        memcpy( &rep_start, (void*) ptr, 2 ); // Start of sample repeat offset in words. 
        modfile->samples[ i ].repeat_start = INTERNAL_MODFILE_ENDIAN_SWAP( rep_start );
        ptr += 2;

        if( ptr + 2 > end ) goto failed;
        unsigned short rep_len = 0;
        memcpy( &rep_len, (void*) ptr, 2 ); // Length of sample repeat in words.
        modfile->samples[ i ].repeat_length = INTERNAL_MODFILE_ENDIAN_SWAP( rep_len );
        ptr += 2;
    }

    if( ptr + 1 > end ) goto failed;
    unsigned char song_len = 0;
    memcpy( &song_len, (void*) ptr, 1 ); // Number of song positions. Legal values are 1..128.
    modfile->song_length = song_len;
    ptr += 1;

    ptr += 1; // Historically set to 127, but can be safely ignored.
    
    if( ptr + 128 > end ) goto failed;
    unsigned char song[ 128 ];
    memcpy( song, (void*) ptr, 128 ); // Pattern table: patterns to play in each song position (0..127)
    ptr+=128;
    int highest_pattern = 0;
    for( int i = 0; i < modfile->song_length; ++i ) {
        modfile->song[ i ] = song[ i ];
        if( song[ i ] > highest_pattern ) {
            highest_pattern = song[ i ];
        }
    }

    if( ptr + 4 > end ) goto failed;
    char letters[ 5 ] = "";
    memcpy( letters, (void*) ptr, 4 ); // The four letters "M.K." 
    if( strcmp( letters, "M.K." ) != 0 ) goto failed;
    ptr += 4;

    #undef INTERNAL_MODFILE_ENDIAN_SWAP
    #define INTERNAL_MODFILE_ENDIAN_SWAP( x )  ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | (((x) & 0xff) << 24))

    unsigned int notes[] = { 
    //    C    C#   D    D#   E    F    F#   G    G#   A    A#   B
        1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907, // Octave 0
         856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453, // Octave 1
         428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226, // Octave 2
         214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113, // Octave 3
         107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57, // Octave 4
    };

    char const* notes_str[] = {
        "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
        "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
        "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
        "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
        "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
    };

    modfile->pattern_count = highest_pattern + 1;
    for( int i = 0; i <= highest_pattern; ++i ) {
        for( int j = 0; j < 64; ++j ) {
            for( int k = 0; k < 4; ++k ) {
                if( ptr + 4 > end ) goto failed;
                unsigned int v = 0;
                memcpy( &v, (void*) ptr, 4 );
                ptr += 4;
                v = INTERNAL_MODFILE_ENDIAN_SWAP( v );
                unsigned int w = ( v & 0xf0000000 ) >> 28;
                unsigned int x = ( v & 0x0fff0000 ) >> 16;
                unsigned int y = ( v & 0x0000f000 ) >> 12;
                unsigned int z = ( v & 0x00000fff );
                char const* str = "---";
                int n = 0;
                for( int c = 0; c < sizeof( notes ) / sizeof( *notes ); ++c ) {
                    if( notes[ c ] == x ) {
                        str = notes_str[ c ];
                        n =  c;
                        break;
                    }
                }
                if( n > 0 ) {
                    n += 12;
                }
                modfile->patterns[ i ].divisions[ j ].channels[ k ].note = n;
                modfile->patterns[ i ].divisions[ j ].channels[ k ].sample = w * 16 + y;
                modfile->patterns[ i ].divisions[ j ].channels[ k ].effect = z;
                printf( "%s%02d%03X ", str, w * 16 + y, z );
            }
            printf( "\n" );
        }
        printf( "\n" );
    }

    for( int i = 0; i < 31; ++i ) {
    }

    #undef INTERNAL_MODFILE_ENDIAN_SWAP

    return modfile;

failed:
    free( modfile );
    return NULL;
}


void modfile_free( struct modfile_t* modfile ) {
    free( modfile );
}


void mod_to_tracker( struct tracker_t* tracker, char const* filename ) {
    FILE* mf = fopen( filename, "rb" );
    fseek( mf, 0, SEEK_END );
    size_t mod_size = ftell( mf );
    fseek( mf, 0, SEEK_SET );
    void* mod_data = malloc( mod_size );
    fread( mod_data, mod_size, 1, mf );
    fclose( mf );
    struct modfile_t* modfile = modfile_load( mod_data, mod_size );
    free( mod_data );

    tracker->song_length = modfile->song_length;
    for( int i = 0; i < modfile->song_length; ++i ) {
        tracker->song[ i ] = modfile->song[ i ];
    }
    
    int samples_map[] = {
        0, 117, 66, 35, 128, 128, 128, 128,
    };
    int drums_map[] = {
        0, 0, 0, 0, 35, 38, 42, 46,
    };
    int transpose[] = {
        12, 24, 12, 0, 0, 0, 0, 0,
    };
    float velocities[] = {
        1.0f, 1.0f, 0.25f, 0.5f, 0.8f, 0.8f, 0.3f, 0.15f,
    };
    modfile->samples[ 2 ].volume = 24;
    int beats_per_minute = 125;
    int ticks_per_division = 6;
    int divisions_per_minute = ( 24 * beats_per_minute ) / ticks_per_division;

    float step_time = 60.0f / (float) divisions_per_minute;
    for( int i = 0; i < modfile->pattern_count; ++i ) {
        printf( "\n%d\n", i );
        for( int j = 0; j < 64; ++j ) {
            for( int k = 0; k < 4; ++k ) {
                if( modfile->patterns[ i ].divisions[ j ].channels[ k ].note ) {
                    tracker->patterns[ i ].channels[ k ].notes[ j ].note = modfile->patterns[ i ].divisions[ j ].channels[ k ].note;
                    int index = modfile->patterns[ i ].divisions[ j ].channels[ k ].sample - 1;
                    int vel = modfile->samples[ index ].volume * 2;
                    if( vel > 127 ) vel = 127;
                    tracker->patterns[ i ].channels[ k ].notes[ j ].velocity = (int)( vel * velocities[ index ] );
                    tracker->patterns[ i ].channels[ k ].notes[ j ].program = samples_map[ index ];
                    if( samples_map[ index ] == 128 ) {
                        tracker->patterns[ i ].channels[ k ].notes[ j ].note = drums_map[ index ];
                    } else {
                        tracker->patterns[ i ].channels[ k ].notes[ j ].note += transpose[ index ];
                    }
                }
                int effect = modfile->patterns[ i ].divisions[ j ].channels[ k ].effect;
                int effect_type = effect >> 8;
                int effect_value = effect & 0xff;
                if( effect_type == 0xC ) {
                    if( effect_value < 4 ) {
                        tracker->patterns[ i ].channels[ k ].notes[ j ].note = 128;
                    } else {
                        int vel = tracker->patterns[ i ].channels[ k ].notes[ j ].velocity;
                        vel = ( vel * effect_value ) / 64;
                        if( vel > 127 ) vel = 127;
                        tracker->patterns[ i ].channels[ k ].notes[ j ].velocity = vel; 
                    }
                } else if( effect_type == 0xD ) {
                    tracker->patterns[ i ].channels[ k ].notes[ j ].cc_command = 6;
                    tracker->patterns[ i ].channels[ k ].notes[ j ].cc_data = 0;//TODO
                } else if( effect_type == 0xF ) {
                    if( effect_value <= 32 ) {
                        ticks_per_division = effect_value;
                    } else {
                        beats_per_minute = effect_value;
                    }
                    divisions_per_minute = ( 24 * beats_per_minute ) / ticks_per_division;
                    step_time = 60.0f / (float) divisions_per_minute;
                    tracker->patterns[ i ].channels[ k ].notes[ j ].cc_command = 7;
                    tracker->patterns[ i ].channels[ k ].notes[ j ].cc_data = (int)( step_time * 1000.0f );
                } else if( effect_type == 0xA || effect_type == 0x1 || effect_type == 0x2 ) {
                    printf( "%X  %d  prg: %d ch: %d\n", effect_type, effect_value, tracker->patterns[ i ].channels[ k ].notes[ j ].program, k );
                }
            }
        }
    }

    modfile_free( modfile );
}


/*
#include "modfile.h"
*/

/*
    mod_to_tracker( tracker, "newhill.mod" );
*/