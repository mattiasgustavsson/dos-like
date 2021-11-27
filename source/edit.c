// Simple code editor. I just thought it would be nice to include a DOS-style 
// text editor, but it is not at all an attempt to make a serious editor.
// The code ended up being really messy, mostly because I only find 10 minutes
// here and there to work on it over the course of a few days. There are most
// likely lots of bugs. I'd welcome pull requests with fixed and refactors ;-)
//      /Mattias Gustavsson

#include "dos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Customization
int use_50lines = 0;
int tab_width = 4;
int col_background = 1;
int col_select_text = 0;
int col_select_background = 3;
int col_comment = 7;
int col_string = 11;
int col_preproc = 13;
int col_symbol = 15;
int col_keyword = 15;
int col_number = 11;
int col_default = 14;
int col_line_continuation = 10;

struct str_t {
    int size;
    int capacity;
    char* chars;
};

struct StbTexteditRowStruct;
int getwidth( struct str_t* str, int n, int i );
void layoutrow( struct StbTexteditRowStruct* row, struct str_t* str, int n );
void deletechars( struct str_t* str, int i, int n );
int insertchars( struct str_t* str, int i, char const* ch, int n );

#define STB_TEXTEDIT_IMPLEMENTATION
#define STB_TEXTEDIT_KEYTYPE unsigned long long
#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSITIONTYPE int
#define STB_TEXTEDIT_UNDOSTATECOUNT (4096)
#define STB_TEXTEDIT_UNDOCHARCOUNT (256*1024)
#define STB_TEXTEDIT_STRING struct str_t
#define STB_TEXTEDIT_STRINGLEN(obj) (obj->size)
#define STB_TEXTEDIT_LAYOUTROW(r,obj,n) layoutrow(r,obj,n)
#define STB_TEXTEDIT_GETWIDTH(obj,n,i) getwidth(obj,n,i)
#define STB_TEXTEDIT_GETWIDTH_NEWLINE 0.0f
#define STB_TEXTEDIT_KEYTOTEXT(k) ((unsigned char)(k >> 32))
#define STB_TEXTEDIT_GETCHAR(obj,i) (obj->chars[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_DELETECHARS(obj,i,n) deletechars(obj,i,n)
#define STB_TEXTEDIT_INSERTCHARS(obj,i,c,n) insertchars(obj,i,c,n)
#define STB_TEXTEDIT_K_SHIFT ( 1 << 30 )
#define STB_TEXTEDIT_K_LEFT KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT KEY_RIGHT
#define STB_TEXTEDIT_K_UP KEY_UP
#define STB_TEXTEDIT_K_DOWN KEY_DOWN
#define STB_TEXTEDIT_K_PGUP KEY_PRIOR
#define STB_TEXTEDIT_K_PGDOWN KEY_NEXT
#define STB_TEXTEDIT_K_LINESTART KEY_HOME
#define STB_TEXTEDIT_K_LINEEND KEY_END
#define STB_TEXTEDIT_K_TEXTSTART ( KEY_HOME | ( 1 << 29 ) )
#define STB_TEXTEDIT_K_TEXTEND ( KEY_END | ( 1 << 29 ) )
#define STB_TEXTEDIT_K_DELETE KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE KEY_BACK
#define STB_TEXTEDIT_K_UNDO ( KEY_Z | ( 1 << 29 ) )
#define STB_TEXTEDIT_K_REDO ( KEY_Y | ( 1 << 29 ) )
#define STB_TEXTEDIT_K_INSERT KEY_INSERT
#define STB_TEXTEDIT_IS_SPACE(ch) ( isspace(ch) && ch != '\t' )
#define STB_TEXTEDIT_K_WORDLEFT ( KEY_LEFT | ( 1 << 29 ) )
#define STB_TEXTEDIT_K_WORDRIGHT ( KEY_RIGHT | ( 1 << 29 ) )
#include "libs/stb_textedit.h"


int getwidth( struct str_t* str, int n, int i ) {
    char* s = str->chars + n;
    char* end = s + i;
    int x = 0; 
    while( s < end ) {
        if( *s == '\t' ) {
            int ofs = tab_width - ( x % tab_width );
            x += ( ofs - 1 );
        }
        ++s;
        ++x;
    }
    if( *s == '\t' ) {
        int ofs = tab_width - ( x % tab_width );
        x += ofs;
        return (float)x;
    } else if( *s < ' ' ) {
        return 0.0f;
    } else {
        return 1.0f;
    }
}


void layoutrow( struct StbTexteditRowStruct* row, struct str_t* str, int n ) {
    char const* s = str->chars + n;
    int len = 0; 
    int x = 0;
    while( *s ) {
        if( *s == '\t' ) {
            int ofs = tab_width - ( x % tab_width );
            x += ( ofs - 1 );
        }
        ++x;
        if( *s++ == '\n' ) {
            break;
        }
        ++len;
    }
    row->x0 = (float) 0;
    row->x1 = (float) x;
    row->baseline_y_delta = 1;
    row->ymin = 0;
    row->ymax = 1;
    row->num_chars = (int)( s - str->chars ) - n;
}

int g_modified = 0;

void deletechars( struct str_t* str, int i, int n ) {
    if( str->size - ( i + n ) > 0 ) {
        memmove( str->chars + i, str->chars + i + n, str->size - ( i + n ) );    
    }
    str->size -= n;
    str->chars[ str->size ] = '\0';
    g_modified = 1;
}


int insertchars( struct str_t* str, int i, char const* ch, int n ) {
    if( str->size + n >= str->capacity ) {
        str->capacity *= 2;
        if( str->size + n >= str->capacity ) {
            str->capacity += n;
        }
        str->chars = (char*) realloc( str->chars, str->capacity + 1 ); 
    }
    if( str->size - i > 0 ) {
        memmove( str->chars + i + n, str->chars + i, str->size - i );
    }
    memcpy( str->chars + i, ch, n );
    str->size += n;
    str->chars[ str->size ] = '\0';
    g_modified = 1;
    return 1;
}


int g_pos_x = 0;
int g_pos_y = 0;
int g_fg = 7;
int g_bg = 0;
int g_mx = -1;
int g_my = -1;

void mousepos( int mx, int my ) {
    if( mx < 0 ) mx = 0;
    if( mx >= screenwidth() ) mx = screenwidth() - 1;
    if( my < 0 ) my = 0;
    if( my >= screenheight() ) my = screenheight() - 1;
    g_mx = mx;
    g_my = my;
}

void goxy( int x, int y ) {
    g_pos_x = x;
    g_pos_y = y;
}

void setcol( int c ) {
    g_fg = c;
}

void setbg( int c ) {
    g_bg = c;
}

void putstring( char const* str ) {
    unsigned char* buf = screenbuffer();
    int w = screenwidth();
    int h = screenheight();
    int x = g_pos_x;
    int y = g_pos_y;
    while( *str ) {
        if( x >= 0 && x < w && y >= 0 && y < h ) {
            buf[ ( x + y * w ) * 2 + 0 ] = *str;
            unsigned char attrib = ( g_fg & 0xf ) | ( ( g_bg & 0xf ) << 4 );
            if( x == g_mx && y == g_my ) {
                attrib = ~attrib;
            }
            buf[ ( x + y * w ) * 2 + 1 ] = attrib;
        }
        ++str;
        ++x;
    }
    g_pos_x = x;
}

void clreol( void ) {
    unsigned char* buf = screenbuffer();
    int w = screenwidth();
    int h = screenheight();
    int x = g_pos_x;
    int y = g_pos_y;
    while( x < w ) {
        if( x >= 0 && x < w && y >= 0 && y < h ) {
            buf[ ( x + y * w ) * 2 + 0 ] = ' ';
            unsigned char attrib = ( g_fg & 0xf ) | ( ( g_bg & 0xf ) << 4 );
            if( x == g_mx && y == g_my ) {
                attrib = ~attrib;
            }
            buf[ ( x + y * w ) * 2 + 1 ] = attrib;
        }
        ++x;
    }
    g_pos_x = x;
}

void help( void ) {
    cursoff();
    textbackground( 3 );
    setbg( 3 );
    setcol( 0 );
    clrscr();
    int row = 1;
    goxy( 1, row++ ); putstring( " EDIT.EXE - Simple code editor" );   
    goxy( 1, row++ ); putstring( " \xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf" );
    goxy( 1, row++ ); putstring( "This is a very basic example of an editor for editing code. It supports only  " );   
    goxy( 1, row++ ); putstring( "the most common editing operations, and some basic syntax highlighting for C. " );   
    goxy( 1, row++ ); putstring( "For serious programming, you are better off using another editor - this one is" );   
    goxy( 1, row++ ); putstring( "kind of experimental, quickly thrown together and not thoroughly tested.      " );   
    goxy( 1, row++ ); putstring( "Use at your own risk, there's no guarantee that EDIT won't corrupt your files." );   
    goxy( 1, row++ ); putstring( "                                                                              " );   
    goxy( 1, row++ ); putstring( " Keyboard Shortcuts" );   
    goxy( 1, row++ ); putstring( " \xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf\xdf" );
    goxy( 1, row++ ); putstring( "F1     Show help screen                                                       " );   
    goxy( 1, row++ ); putstring( "CTRL+S Save the current document                                              " );   
    goxy( 1, row++ ); putstring( "CTRL+W Exit without saving                                                    " );   
    goxy( 1, row++ ); putstring( "CTRL+C Copy selection to internal clipboard                                   " );   
    goxy( 1, row++ ); putstring( "CTRL+V Paste from internal clipboard                                          " );   
    goxy( 1, row++ ); putstring( "CTRL+X Cut selection into internal clipboard                                  " );   
    goxy( 1, row++ ); putstring( "CTRL+Z Undo                                                                   " );   
    goxy( 1, row++ ); putstring( "CTRL+Y Redo                                                                   " );   
    goxy( 1, row++ ); putstring( "HOME   Start of line                                                          " );   
    goxy( 1, row++ ); putstring( "END    End of line                                                            " );   
    goxy( 1, row++ ); putstring( "PGUP   Move one page up                                                       " );   
    goxy( 1, row++ ); putstring( "PGDOWN Move one page down                                                     " );   
    goxy( 1, row++ ); putstring( "The mouse can be used to move the cursor, or drag to define a selection.      " );   
    while( !*readkeys() ) waitvbl();
    readchars();
    curson();
}


#ifndef _WIN32 
    #include <strings.h>
    #define stricmp strcasecmp
#endif


int main( int argc, char* argv[] ) {
    
    #ifdef __wasm__  
        // This is a hack to make web assembly builds of the editor open a source file for demo purposes
        argc = 2; argv[ 1 ] = "source/rotozoom.c";
    #endif

    setvideomode( videomode_80x25_9x16 );
   
    if( argc != 2 ) {
        goxy( 0, 0 ); putstring( "Usage:" );
        goxy( 4, 1 ); putstring( "edit filename.ext" );
        goxy( 0, 3 ); putstring( "Example:" );
        goxy( 4, 4 ); putstring( "edit source/rotozoom.c" );
        gotoxy( 0, 5 );
        while( !*readkeys() ) waitvbl();
        return 1;
    }
    
    int scrlines = 25;
    if( use_50lines ) {
        setvideomode( videomode_80x50_8x8 );
        scrlines = 50;
    }
    
    char const* filename = argv[ 1 ];
    
    int syntax_highlight = 0;
    char const* ext = strrchr( filename, '.' );
    if( ext && ( stricmp( ext, ".c" ) == 0 || stricmp( ext, ".h" ) == 0 ) ) {
        syntax_highlight = 1;
    }

    
    struct str_t str;

    FILE* fp = fopen( filename, "r" );
    if( fp ) {
        fseek( fp, 0, SEEK_END );
        size_t sz = ftell( fp );
        fseek( fp, 0, SEEK_SET );
        char* data = (char*) malloc( sz + 1);
        size_t len = fread( data, 1, sz, fp );
        fclose( fp );
        data[ len ] = '\0';
        str.size = (int) len;
        str.capacity = (int) sz;
        str.chars = data;
    } else {
        str.size = 0;
        str.capacity = 256;
        str.chars = (char*)malloc(str.capacity + 1);
        *str.chars = '\0';
    }
    
    int clipboard_capacity = 4096;
    int clipboard_size = 0;
    char* clipboard_chars = (char*) malloc( clipboard_capacity );

    static STB_TexteditState state;
    stb_textedit_initialize_state( &state, 0 );
    state.row_count_per_page = scrlines - 2;
    
    int prev_mx = 0;
    int prev_my = 0;
    int first = 1;
    int isdragging = 0;
    int xscroll = 0;
    int yscroll = 0;
    while( !shuttingdown() ) {
        unsigned char const* chars = (unsigned char*)readchars();
        enum keycode_t* keys = readkeys();
        int mx = mousex();
        int my = mousey();
        if( *chars == '\0' && *keys == KEY_INVALID && !first && prev_mx == mx && prev_my == my) {
            waitvbl();
            continue;
        }
        prev_mx = mx;
        prev_my = my;
        mousepos( mx, my );
        first = 0;
        while( *chars ) {
            if( *chars >= ' ' || *chars == '\t' ) {
                unsigned long long ch = ( (unsigned long long)*chars ) << 32;
                stb_textedit_key( &str, &state, ch );
            }
            ++chars;
        }
        while( *keys ) {
            unsigned long long key = ( (unsigned long long) *keys );
            
            if( key == KEY_F1 ) {
                help();
                break;
            }

            if( key == KEY_S && keystate( KEY_CONTROL ) ) {
                FILE* fp = fopen( filename, "w" );
                if( fp ) {
                    fprintf( fp, "%s", str.chars );
                    fclose( fp );
                    g_modified = 0;                    
                }
            }

            if( ( key == KEY_C && keystate( KEY_CONTROL ) ) 
                || ( key == KEY_X && keystate( KEY_CONTROL ) ) 
                || ( key == KEY_INSERT && keystate( KEY_CONTROL ) ) 
                || ( key == KEY_DELETE && keystate( KEY_SHIFT ) ) ) {
            
                int start = state.select_start;
                int end = state.select_end;
                if( start > end ) {
                    int temp = start;
                    start = end;
                    end = temp;
                }
                if( end - start > 0 ) {
                    int size = end - start;
                    if( size > clipboard_capacity ) {
                        clipboard_capacity = size;
                        clipboard_chars = (char*) realloc( clipboard_chars, clipboard_capacity );
                    }
                    memcpy( clipboard_chars, str.chars + start, size );
                    clipboard_size = size;
                }
                if( key == KEY_X || key == KEY_DELETE ) {
                    stb_textedit_cut( &str, &state );
                }
            }
            if( ( key == KEY_V && keystate( KEY_CONTROL ) ) 
                || ( key == KEY_INSERT && keystate( KEY_SHIFT ) ) ) {
             
                stb_textedit_paste( &str, &state, clipboard_chars, clipboard_size );
            }

            if( keystate( KEY_CONTROL ) ) {
                key |= ( 1 << 29 );
            }
            if( keystate( KEY_SHIFT ) ) {
                key |= ( 1 << 30 );
            }
            if( key == KEY_RETURN ) {
                key |= ( ( (unsigned long long) '\n' ) << 32 );
            }
            if( key == KEY_PRIOR ) {
                yscroll -= state.row_count_per_page;
            }
            if( key == KEY_NEXT) {
                yscroll += state.row_count_per_page;
            }          
            
            stb_textedit_key( &str, &state, key );
            ++keys;
        }
        
        
        int cursrow = 0;
        int curscol = 0;
        int rows = 0;
        char* s = str.chars;
        while( *s ) {
            if( s - str.chars == state.cursor ) {
                break;
            }
            if( *s == '\n' ) {
                ++rows;
                ++cursrow;
                curscol = 0;
            } else if( *s == '\t' ) {
                int ofs = tab_width - ( ( curscol ) % tab_width );
                curscol += ofs;
            } else {
                ++curscol;
            }
            ++s;
        }
        while( *s ) {
            if( *s == '\n' ) {
                ++rows;
            }
            ++s;
        }
        if( yscroll < 0 ) {
            yscroll = 0;
        }
        if( rows > state.row_count_per_page && yscroll > rows - state.row_count_per_page ) {
            yscroll = rows - state.row_count_per_page;
        }
        if( cursrow > yscroll + ( scrlines - 2 ) ) {
            yscroll = cursrow - ( scrlines - 2 );
        }
        if( cursrow < yscroll ) {
            yscroll = cursrow;
        }
        if( curscol > xscroll + 79 ) {
            xscroll = curscol - 79;
        }
        if( curscol < xscroll ) {
            xscroll = curscol;
        }
        
        int cursx = curscol - xscroll;
        int cursy = cursrow - yscroll;
        setbg( col_background );
        setcol( col_default );
        s = str.chars;
        int x = -xscroll;
        int y = -yscroll;
        int in_line_comment = 0;
        int in_block_comment = 0;
        char const* entered_block_comment = NULL;
        char const* entered_string = NULL;
        int in_string = 0;
        int in_preproc = 0;
        int is_keyword = 0;
        int is_number = 0;
        char prevchar = ' ';
        while( y < 0 ) {
            if( *s == '\n' ) {
                ++y;
            }
            if( syntax_highlight ) {
                if( !in_block_comment && !in_line_comment ) {
                    if( *s == '/' && *(s+1) == '/' ) {
                        in_line_comment = 1;
                    }
                    if( *s == '/' && *(s+1) == '*' ) {
                        in_block_comment = 1;
                        entered_block_comment = s + 1;
                    }
                }
                if( in_line_comment && *s == '\n' ) {
                    in_line_comment = 0;
                }            
                if( entered_block_comment != s - 1 && in_block_comment && *s == '/' && ( *(s-1) == '*' ) ) {
                    in_block_comment = 0;
                }
            }
            ++s;
        }
        while( y < scrlines - 1 && *s ) {
            if( syntax_highlight ) {
                if( !in_block_comment && !in_line_comment && !in_string && !in_preproc ) {
                    if( *s == '/' && *(s+1) == '/' ) {
                        in_line_comment = 1;
                    }
                    if( *s == '/' && *(s+1) == '*' ) {
                        in_block_comment = 1;
                        entered_block_comment = s + 1;
                    }
                    if( *s == '"' ) {
                        in_string = 1;
                        entered_string = s;
                    }
                    if( *s == '#' ) {
                        in_preproc = 1;
                    }
                }
                if( !in_block_comment && !in_line_comment && !in_string && !in_preproc ) {
                    if( ( isalnum( *s ) || *s == '_' ) && !isalnum( prevchar ) && prevchar != '_' ) {
                        int is_float = 0;
                        if( is_number && prevchar == '.' ) {
                            is_float = 1;
                            char* st = s;
                            while( *st && ( isdigit( *st ) ) ) {
                                ++st;
                            }
                            if( isalnum( *st ) || *st == '_' ) {
                                if( !( *st == 'f' || *st == 'F' ) ) {
                                    is_float = 0;
                                }
                            }
                        }

                        int len = 0;
                        int all_digits = 1;
                        while( s[len] && ( isalnum( s[len] ) || s[len] == '_' ) ) {
                            if( !isdigit( s[len] ) ) {
                                all_digits = 0;
                            }
                            ++len;
                        }
                        if( all_digits || is_float) {
                            is_number = 1;
                        } else {
                            static char* keywords[] = { 
                                "auto", "double", "int", "struct", "break", "else", "long", "switch",
                                "case", "enum", "register", "typedef", "char", "extern", "return", "union",
                                "const", "float", "short", "unsigned", "continue", "for", "signed", "void",
                                "default", "goto", "sizeof", "volatile", "do", "if", "static", "while",
                            };
                            for( int i = 0; i < sizeof( keywords ) / sizeof( *keywords ); ++i ) {
                                int keylen = strlen( keywords[i] );
                                if( keylen == len && strncmp( s, keywords[i], len ) == 0 ) {
                                    is_keyword = 1;
                                    break;;
                                }
                            }
                        }
                    }
                }
                if( is_keyword  ) {
                    if( !( isalnum( *s ) || *s == '_' ) ) {
                        is_keyword = 0;
                    }
                }
                if( is_number ) {
                    if( !isdigit( *s ) && *s != '.' && *s != 'f' && *s != 'F' ) {
                        is_number = 0;
                    }
                }
            }
            if( y >= 0 ) {
                if( s - str.chars == state.cursor ) {
                    cursx = x;
                    cursy = y;
                }
                int selstart = state.select_start;
                int selend = state.select_end;
                if( selstart > selend ) {
                    int temp = selend;
                    selend = selstart;
                    selstart = temp;
                }
                if( s - str.chars >= selstart && s - str.chars < selend ) {
                    setbg( col_select_background );
                    setcol( col_select_text );                
                } else {
                    setbg( col_background );
                    setcol( col_default );
                    if( syntax_highlight ) {
                        if( in_block_comment || in_line_comment ) {
                            setcol( col_comment );
                        } else if( in_string ) { 
                            setcol( col_string );
                        } else if( in_preproc ) { 
                            setcol( col_preproc );
                        } else if( !isalnum( *s ) && *s != '_' ) {
                            setcol( col_symbol );
                        } else if( is_keyword ) {
                            setcol( col_keyword );
                        } else if( is_number ) {
                            setcol( col_number );
                        } else {
                            setcol( col_default );
                        }
                    }
                }            
                if( *s == '\t' ) {
                    int ofs = tab_width - ( ( x + xscroll ) % tab_width );
                    for( int i = 0; i < ofs; ++i ) {
                        char chs[2] = { ' ', 0 };
                        goxy(x,y);
                        putstring(chs);
                        x++;
                    }
                }
                char nextchar = *(s+1);
                if( x == 79 && *s != '\n' && nextchar != '\n' ) {
                    setcol( col_line_continuation );
                    if( x >= 0 && x < 80 ) {
                        goxy( x, y );
                        char chs[2] = { 0 };
                        chs[0] = '\x1a';
                        putstring(chs);
                    }
                    setcol( 14 );
                    ++x;
                } else if( x < 80 ) {
                    if( (unsigned char)*s >= ' ' ) {
                        if( x >= 0 && x < 80 ) {
                            goxy( x, y );
                            char chs[2] = { 0 };
                            chs[0] = *s;
                            putstring(chs);
                        }
                        ++x;
                    }
                }
            }
            if( syntax_highlight ) {
                if( in_line_comment && *s == '\n' ) {
                    in_line_comment = 0;
                }            
                if( entered_block_comment != s - 1 && in_block_comment && *s == '/' && ( *(s-1) == '*' ) ) {
                    in_block_comment = 0;
                }
                if( entered_string != s && in_string && *s == '"' ) {
                    in_string = 0;
                }
                if( in_preproc && *s == '\n' ) {
                    in_preproc = 0;
                }            
            }
            if( *s == '\n' ) {
                goxy(x,y);
                clreol();
                ++y;
                x = -xscroll;
            }
            prevchar = *s;
            if( *s ) {
                ++s;
            }
        }
        setbg( col_background );
        while( y < scrlines - 1 ) {
            goxy( x, y );
            clreol();
            ++y;
            x = 0;
        }
        
        goxy( 0, scrlines - 1 );
        setbg( 7 );
        setcol( 4 ); putstring( " F1" );
        setcol( 0 ); putstring( " Help " );
        setcol( 4 ); putstring( " Ctrl+S" );
        setcol( 0 ); putstring( " Save " );
        setcol( 4 ); putstring( " Ctrl+W" );
        setcol( 0 ); putstring( " Quit " );
        setcol( 0 ); putstring( "  " );
        char curs[ 256 ];
        sprintf( curs, "%4d:%-4d   ", cursrow + 1, curscol + 1 );
        setcol( 0 ); putstring( curs );
        if( g_modified ) {
            setcol( 0 ); putstring( "\xf" );
        } 
        setcol( 0 ); putstring( filename );
        clreol();
        
        gotoxy( cursx, cursy );

        if( mx < -1 ) mx = -1;
        if( mx > screenwidth() ) mx = screenwidth();
        if( my < -1 ) my = -1;
        if( my >= screenheight() ) my = screenheight() - 1;

        if( keystate( KEY_LBUTTON ) && !isdragging && my < scrlines - 1 ) {
            stb_textedit_click( &str, &state, (float) mx + xscroll, (float) my + yscroll );
            isdragging = 1;
        }
        if( keystate( KEY_LBUTTON ) && isdragging ) {
            stb_textedit_drag( &str, &state, (float) mx + xscroll, (float) my + yscroll );
        }
        if( !keystate( KEY_LBUTTON ) && isdragging ) {
            stb_textedit_drag( &str, &state, (float) mx + xscroll, (float) my + yscroll );
            isdragging = 0;
        }

        if( keystate( KEY_W ) && keystate( KEY_CONTROL ) )  break;
    }
    
    free( clipboard_chars );
    free( str.chars );

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
