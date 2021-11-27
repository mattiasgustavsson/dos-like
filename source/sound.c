// Sound and music example. The code is public domain, the
// wav file is from Warcraft 2 audio setup, the midi file
// is from Simon the Sorcerer, the mus file is from Doom.
//      /Mattias Gustavsson

#include "dos.h"

int main( int argc, char* argv[] ) {

    struct music_t* mus = loadmus( "files/sound/doom.mus" );
    struct music_t* mid = loadmid( "files/sound/simon.mid" );
    struct music_t* mod = loadmod( "files/sound/cfodder.mod" );
    struct music_t* opb = loadopb( "files/sound/doom.opb" );
    struct sound_t* wav = loadwav( "files/sound/soundcard.wav" );
    int doom_soundbank = installusersoundbank( "files/sound/doom.op2" );
    int use_awe32 = 1;

    cputs( "SOUND DEMO" );
    gotoxy( 0, 2 ); cputs( "1 - Play MIDI song" );
    gotoxy( 0, 3 ); cputs( "2 - Play MUS song" );
    gotoxy( 0, 4 ); cputs( "3 - Play MOD song" );
    gotoxy( 0, 5 ); cputs( "4 - Play WAV sound on channel 1" );
    gotoxy( 0, 6 ); cputs( "5 - Play WAV sound on channel 2" );
    gotoxy( 0, 7 ); cputs( "6 - Play WAV sound on channel 3" );
    gotoxy( 0, 8 ); cputs( "7 - Stop sound and music" );
    gotoxy( 0, 9 ); cputs( "8 - Sound mode 11khz 8bit mono (default)" );
    gotoxy( 0, 10 ); cputs( "9 - Sound mode 44khz 16bit stereo" );
    gotoxy( 0, 11 ); cputs( "0 - Sound mode 5khz 8bit mono" );
    gotoxy( 0, 12 ); cputs( "A - Use AWE32 for MIDI/MUS (default)" );
    gotoxy( 0, 13 ); cputs( "S - Use SoundBlaster16 for MIDI/MUS" );
    gotoxy( 0, 14 ); cputs( "O - Play OPB song" );
    gotoxy( 0, 16 ); cputs( "ESC - quit" );
    cursoff();
    while( !shuttingdown() ) {
        char key = *readchars();
        if( key == '1' ) {
            if( use_awe32 ) {
                setsoundbank( DEFAULT_SOUNDBANK_AWE32 );
            } else {
                setsoundbank( DEFAULT_SOUNDBANK_SB16 );
            }
            playmusic( mid, 0, 255 );
        }
        if( key == '2' ) {
            if( use_awe32 ) {
                setsoundbank( DEFAULT_SOUNDBANK_AWE32 );
            } else {
                setsoundbank( doom_soundbank );
            }
            playmusic( mus, 0, 255 );
        }
        if( key == '3' ) {
            playmusic( mod, 0, 255 );
        }
        if( key == 'O' || key == 'o' ) {
            playmusic( opb, 0, 255 );
        }
        if( key == '4' ) {
            playsound( 0, wav, 0, 128 );
        }
        if( key == '5' ) {
            playsound( 1, wav, 0, 128 );
        }
        if( key == '6' ) {
            playsound( 2, wav, 0, 128 );
        }
        if( key == '7' ) {
            stopmusic();
            stopsound( 0 );
            stopsound( 1 );
            stopsound( 2 );
        }
        if( key == '8' ) {
            setsoundmode( soundmode_8bit_mono_11025 );
        }
        if( key == '9' ) {
            setsoundmode( soundmode_16bit_mono_44100 );
        }
        if( key == '0' ) {
            setsoundmode( soundmode_8bit_mono_5000 );
        }
        if( key == 'A' || key == 'a' ) {
            if( !use_awe32 ) {
                use_awe32 = 1;
                stopmusic();
            }
        }
        if( key == 'S' || key == 's' ) {
            if( use_awe32 ) {
                use_awe32 = 0;
                stopmusic();
            }
        }
        if( keystate( KEY_ESCAPE ) )  break;
    }

    return 0;
}
