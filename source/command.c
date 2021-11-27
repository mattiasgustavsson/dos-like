// Very quick-and-dirty wrapper for the built in Windows command line cmd.exe
// Basically, it is just using dos-like to read keyboard input and print text,
// passing all actual commands through to Windows cmd.exe.
// There are lots of things missing and things that won't work as expected, so
// don't use this as a real command line environment - it is merely intended 
// as a bit of fun, to be able to edit (using the bundled edit.c), compile and
// run the samples while staying in a seemingly dos-like environment.
//      /Mattias Gustavsson

#ifndef _WIN32
    #error command.c can only be built for Windows
#endif

#include "dos.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <direct.h>

char* textline;
size_t textline_capacity;
uint16_t attribute = 0x70;


int getnum( const char** str ) {
	int num = -1;
	while( **str >= '0' && **str <= '9' ) {
		if( num < 0 ) {
            num = 0;
        }
		num = num * 10 + ( **str - '0' );
		++(*str);
	}
	return num;
}


int print( const char* format, ... ) {
	va_list args;
	va_start( args, format );
	size_t len = vsnprintf( textline, textline_capacity - 1, format, args );
	while( len < 0 || len >= textline_capacity ) {
		textline_capacity *= 2;
		textline = (char*) realloc( textline, textline_capacity );
		len = vsnprintf( textline, textline_capacity - 1, format, args );
	}
	
	const char* str = textline;
	while( *str ) {
		if( wherex() >= screenwidth() || *str == '\n' )  { 
			gotoxy( 0, wherey() +  1 ); 
		}
		while( wherey() >= screenheight() ) {
			memmove( ((uint16_t*)screenbuffer()), ((uint16_t*)screenbuffer()) + screenwidth(), screenwidth() * ( screenheight() - 1 ) * sizeof( uint16_t ) );
			memset( ((uint16_t*)screenbuffer()) + screenwidth() * ( screenheight() - 1 ), 0, screenwidth() * sizeof( uint16_t ) );
			gotoxy( wherex(), wherey() - 1 );
		}
		if( (unsigned) *str >= ' ' ) {
			((uint16_t*)screenbuffer())[ wherex() + screenwidth() * wherey() ] = ( ( attribute & 0xf0 ) << 4 ) | ( ( attribute & 0x0f ) << 12 ) | (unsigned char) *str;
            gotoxy( wherex() + 1, wherey() );
		} else if( (unsigned) *str == 0x1b && (unsigned) *( str + 1) == '[' ) { // ANSI escape sequence
			const char* ansi = str + 2;
			if( *ansi == '=' ) ++ansi;
			while( ( *ansi >= '0' && *ansi <= '9' ) || *ansi == ';'  ) {
				++ansi;
			}
			switch( *ansi ) {
				case 'A': { // Cursor Up ESC[PnA
					str +=2;
					int num = getnum( &str );
					int y = wherey();
					y -= num;
					gotoxy( wherex(), y < 0 ? 0 : y );
					str = ansi;
				} break;
 
				case 'B': { // Cursor Down ESC[PnB
					str +=2;
					int num = getnum( &str );
					int y = wherey();
					int h = screenheight();
					y += num;
					gotoxy( wherex(), y >= h ? h - 1 : y );
					str = ansi;
				} break;
				
				case 'C': { // Cursor Forward ESC[PnC
					str +=2;
					int num = getnum( &str );
					int x = wherex();
					int w = screenwidth();
					x += num;
					gotoxy(  x >= w ? w - 1 : x, wherey() );
					str = ansi;
				} break;
 
				case 'D': { // Cursor Backward ESC[PnD
					str +=2;
					int num = getnum( &str );
					int x = wherex();
					x -= num;
					gotoxy( x < 0 ? 0 : x, wherey() );
					str = ansi;
				} break;
			
                
				case 'm': { // Set Graphics Mode ESC[Ps;...;Psm 
					str +=2;
					while( *str != 'm' ) {
						int num = getnum( &str );

						if( num == 0 ) {
							attribute = 0xf0;
						} else if( num == 0 ) {
							attribute &= 0x70;
							attribute |= 0x80;
						} else if( num >= 30 && num <= 37 ) {
							attribute &= 0x8f;
							attribute |= ( num - 30 ) << 4;
						} else if( num >= 40 && num <= 47 ) {
							attribute &= 0xf0;
							attribute |= ( num - 40 );
						}
						if( *str == ';' ) ++str;
					}
					str = ansi;
				} break;
 
				default:
					goto nonansi;
					break;
			}
		nonansi: ;
		}
		++str;
	}
	if( wherex() >= screenwidth() || *str == '\n' ) { 
        gotoxy( 0, wherey() + 1 );
	}
	while( wherey() >= screenheight() ) {
		memmove( ((uint16_t*)screenbuffer()), ((uint16_t*)screenbuffer()) + screenwidth(), screenwidth() * ( screenheight() - 1 ) * sizeof( uint16_t ) );
		memset( ((uint16_t*)screenbuffer()) + screenwidth() * ( screenheight() - 1 ), 0, screenwidth() * sizeof( uint16_t ) );
        gotoxy( 0, wherey() - 1 );
	}
	return len;
}


int input( char* buffer, int size ) {
	int c = *readchars();
	int len = 0;

	while( c != '\r' && len + 1 < size ) {
		if( c >= ' ' && c < 0xff ) {
			buffer[ len++ ] = (char) c;
			print( "%c", (char) c );
        } else {
			switch( c >> 8 ) {
				case 0x08: { // Backspace
					if( len > 0 ) {
						--len; 
                        gotoxy( wherex() - 1, wherey() );
						print( " " );
                        gotoxy( wherex() - 1, wherey() );
                    }
                } break;
            }
        }
		c = *readchars();
        if( c == '\0' ) {
            if( *readkeys() == KEY_BACK ) {
                c = 0x08 << 8;
            }
            waitvbl();
        }
    }
	buffer[ len ] = 0;

	return len;
}


void exec( char* cmd ) {
    char* s = cmd;
    while( *s <= ' ' ) {
        if( *s == '\0' ) return;
        ++s;
    }
	char command_string[256];
	strcpy( command_string, "%WINDIR%\\System32\\cmd.exe /s /c \"SET DIRCMD=/OGN&&" );
	strcat( command_string,  cmd );
	strcat( command_string, " \" 2>&1" );
	FILE* pipe = _popen( command_string, "r" );
	if (!pipe) { 
        print( "cmd.exe not found." ); 
        return; 
    }
	char buffer[128];
	while( !feof( pipe ) ) {
    	if( fgets(buffer, 128, pipe) != NULL ) {
    		print( buffer );
            waitvbl();
        }
	}
	_pclose( pipe );
}


int main( int argc, char *argv[] ) {
    setvideomode( videomode_80x25_9x16 );

    textline_capacity = 256;
	textline = (char*) malloc( textline_capacity );
    
    print( "Starting dos-like...\n" );
    for( int i = 0; i < 60; ++i ) waitvbl();
    
	char command[ 256 ];
	getcwd( command, 256 );
	print( "\n%s>",command );
	int len = input( command, 255 );
	while( stricmp( command, "exit" ) != 0 ) {
        if( strnicmp( command, "cd ", 3 ) == 0 ) {
			char* str  = command +2;
			while( *str <=' ' && *str != 0 ) ++str;
			if( chdir( str ) != 0 ) {
                print( "\nThe system cannot find the path specified." );
            }
		    print( "\n" );
			goto next;
		}
        if( strlen( command ) == 2 && command[ 1 ] == ':' && ( ( command[ 0 ] >= 'A' && command[ 0 ] <= 'Z' ) || ( command[ 0 ] >= 'a' && command[ 0 ] <= 'z' ) ) ) {
            if( _chdrive( command[ 0 ] - ( command[ 0 ] <= 'Z' ? 'A' : 'a' ) + 1 ) != 0 ) {
                print( "\nThe device is not ready." );
            }
		    print( "\n" );
			goto next;
        }
        print( "\n" );
		exec( command );
	next:
		getcwd( command, 256 );
		print( "\n%s>",command );
		len = input( command, 255 );
	}
    free( textline );
    return 0;
}
