/*
------------------------------------------------------------------------------
          Licensing information can be found at the end of the file.
------------------------------------------------------------------------------

crtemu_pc.h - v0.1 - Cathode ray tube emulation shader for C/C++.

Do this:
    #define CRTEMU_PC_IMPLEMENTATION
before you include this file in *one* C/C++ file to create the implementation.
*/


#ifndef crtemu_pc_h
#define crtemu_pc_h

#ifndef CRTEMU_PC_U32
    #define CRTEMU_PC_U32 unsigned int
#endif
#ifndef CRTEMU_PC_U64
    #define CRTEMU_PC_U64 unsigned long long
#endif

typedef struct crtemu_pc_t crtemu_pc_t;

crtemu_pc_t* crtemu_pc_create( void* memctx );

void crtemu_pc_destroy( crtemu_pc_t* crtemu_pc );

typedef struct crtemu_pc_config_t 
    { 
    float curvature, scanlines, shadow_mask, separation, ghosting, noise, flicker, vignette, distortion, aspect_lock,
        hpos, vpos, hsize, vsize, contrast, brightness, saturation, blur, degauss; // range -1.0f to 1.0f, default=0.0f
    } crtemu_pc_config_t;

void crtemu_pc_config(crtemu_pc_t* crtemu_pc, crtemu_pc_config_t const* config);

void crtemu_pc_frame( crtemu_pc_t* crtemu_pc, CRTEMU_PC_U32* frame_abgr, int frame_width, int frame_height );

void crtemu_pc_present( crtemu_pc_t* crtemu_pc, CRTEMU_PC_U64 time_us, CRTEMU_PC_U32 const* pixels_xbgr, int width, int height, 
    CRTEMU_PC_U32 mod_xbgr, CRTEMU_PC_U32 border_xbgr );

void crtemu_pc_coordinates_window_to_bitmap( crtemu_pc_t* crtemu_pc, int width, int height, int* x, int* y );
void crtemu_pc_coordinates_bitmap_to_window( crtemu_pc_t* crtemu_pc, int width, int height, int* x, int* y );

#endif /* crtemu_pc_h */

/*
----------------------
    IMPLEMENTATION
----------------------
*/
#ifdef CRTEMU_PC_IMPLEMENTATION
#undef CRTEMU_PC_IMPLEMENTATION

#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS
#include <stddef.h>
#include <string.h>

#ifndef CRTEMU_PC_MALLOC
    #include <stdlib.h>
    #if defined(__cplusplus)
        #define CRTEMU_PC_MALLOC( ctx, size ) ( ::malloc( size ) )
        #define CRTEMU_PC_FREE( ctx, ptr ) ( ::free( ptr ) )
    #else
        #define CRTEMU_PC_MALLOC( ctx, size ) ( malloc( size ) )
        #define CRTEMU_PC_FREE( ctx, ptr ) ( free( ptr ) )
    #endif
#endif

#ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
    #ifndef CRTEMU_PC_REPORT_ERROR
        #define _CRT_NONSTDC_NO_DEPRECATE 
        #define _CRT_SECURE_NO_WARNINGS
        #include <stdio.h>
        #define CRTEMU_PC_REPORT_ERROR( str ) printf( "%s", str )
    #endif
#endif

#ifndef _WIN32 
    #define CRTEMU_PC_SDL
#endif

#ifdef __wasm__
    #define CRTEMU_PC_WEBGL
#endif

#ifndef CRTEMU_PC_SDL

    #ifdef __cplusplus
    extern "C" {
    #endif

        __declspec(dllimport) struct HINSTANCE__* __stdcall LoadLibraryA( char const* lpLibFileName );
        __declspec(dllimport) int __stdcall FreeLibrary( struct HINSTANCE__* hModule );
        #if defined(_WIN64)
            typedef __int64 (__stdcall* CRTEMU_PC_PROC)( void );
            __declspec(dllimport) CRTEMU_PC_PROC __stdcall GetProcAddress( struct HINSTANCE__* hModule, char const* lpLibFileName );
        #else
            typedef __int32 (__stdcall* CRTEMU_PC_PROC)( void );
            __declspec(dllimport) CRTEMU_PC_PROC __stdcall GetProcAddress( struct HINSTANCE__* hModule, char const* lpLibFileName );
        #endif

    #ifdef __cplusplus
        }
    #endif

    #define CRTEMU_PC_GLCALLTYPE __stdcall
    typedef unsigned int CRTEMU_PC_GLuint;
    typedef int CRTEMU_PC_GLsizei;
    typedef unsigned int CRTEMU_PC_GLenum;
    typedef int CRTEMU_PC_GLint;
    typedef float CRTEMU_PC_GLfloat;
    typedef char CRTEMU_PC_GLchar;
    typedef unsigned char CRTEMU_PC_GLboolean;
    typedef size_t CRTEMU_PC_GLsizeiptr;
    typedef unsigned int CRTEMU_PC_GLbitfield;

    #define CRTEMU_PC_GL_FLOAT 0x1406
    #define CRTEMU_PC_GL_FALSE 0
    #define CRTEMU_PC_GL_FRAGMENT_SHADER 0x8b30
    #define CRTEMU_PC_GL_VERTEX_SHADER 0x8b31
    #define CRTEMU_PC_GL_COMPILE_STATUS 0x8b81
    #define CRTEMU_PC_GL_LINK_STATUS 0x8b82
    #define CRTEMU_PC_GL_INFO_LOG_LENGTH 0x8b84
    #define CRTEMU_PC_GL_ARRAY_BUFFER 0x8892
    #define CRTEMU_PC_GL_TEXTURE_2D 0x0de1
    #define CRTEMU_PC_GL_TEXTURE0 0x84c0
    #define CRTEMU_PC_GL_TEXTURE1 0x84c1
    #define CRTEMU_PC_GL_TEXTURE2 0x84c2
    #define CRTEMU_PC_GL_TEXTURE3 0x84c3
    #define CRTEMU_PC_GL_TEXTURE_MIN_FILTER 0x2801
    #define CRTEMU_PC_GL_TEXTURE_MAG_FILTER 0x2800
    #define CRTEMU_PC_GL_NEAREST 0x2600
    #define CRTEMU_PC_GL_LINEAR 0x2601
    #define CRTEMU_PC_GL_STATIC_DRAW 0x88e4
    #define CRTEMU_PC_GL_RGBA 0x1908
    #define CRTEMU_PC_GL_UNSIGNED_BYTE 0x1401
    #define CRTEMU_PC_GL_COLOR_BUFFER_BIT 0x00004000
    #define CRTEMU_PC_GL_TRIANGLE_FAN 0x0006
    #define CRTEMU_PC_GL_FRAMEBUFFER 0x8d40
    #define CRTEMU_PC_GL_VIEWPORT 0x0ba2
    #define CRTEMU_PC_GL_RGB 0x1907
    #define CRTEMU_PC_GL_COLOR_ATTACHMENT0 0x8ce0
    #define CRTEMU_PC_GL_TEXTURE_WRAP_S 0x2802
    #define CRTEMU_PC_GL_TEXTURE_WRAP_T 0x2803
    #define CRTEMU_PC_GL_CLAMP_TO_BORDER 0x812D
    #define CRTEMU_PC_GL_TEXTURE_BORDER_COLOR 0x1004

#else

    #ifndef CRTEMU_PC_WEBGL
        #include <GL/glew.h>
        #include "SDL_opengl.h"
    #else
        #include <wajic_gl.h>
    #endif
    #define CRTEMU_PC_GLCALLTYPE GLAPIENTRY
    
    typedef GLuint CRTEMU_PC_GLuint;
    typedef GLsizei CRTEMU_PC_GLsizei;
    typedef GLenum CRTEMU_PC_GLenum;
    typedef GLint CRTEMU_PC_GLint;
    typedef GLfloat CRTEMU_PC_GLfloat;
    typedef GLchar CRTEMU_PC_GLchar;
    typedef GLboolean CRTEMU_PC_GLboolean;
    typedef GLsizeiptr CRTEMU_PC_GLsizeiptr;
    typedef GLbitfield CRTEMU_PC_GLbitfield;

     #define CRTEMU_PC_GL_FLOAT GL_FLOAT 
     #define CRTEMU_PC_GL_FALSE GL_FALSE 
     #define CRTEMU_PC_GL_FRAGMENT_SHADER GL_FRAGMENT_SHADER 
     #define CRTEMU_PC_GL_VERTEX_SHADER GL_VERTEX_SHADER 
     #define CRTEMU_PC_GL_COMPILE_STATUS GL_COMPILE_STATUS 
     #define CRTEMU_PC_GL_LINK_STATUS GL_LINK_STATUS 
     #define CRTEMU_PC_GL_INFO_LOG_LENGTH GL_INFO_LOG_LENGTH 
     #define CRTEMU_PC_GL_ARRAY_BUFFER GL_ARRAY_BUFFER 
     #define CRTEMU_PC_GL_TEXTURE_2D GL_TEXTURE_2D 
     #define CRTEMU_PC_GL_TEXTURE0 GL_TEXTURE0 
     #define CRTEMU_PC_GL_TEXTURE1 GL_TEXTURE1 
     #define CRTEMU_PC_GL_TEXTURE2 GL_TEXTURE2 
     #define CRTEMU_PC_GL_TEXTURE3 GL_TEXTURE3 
     #define CRTEMU_PC_GL_TEXTURE_MIN_FILTER GL_TEXTURE_MIN_FILTER 
     #define CRTEMU_PC_GL_TEXTURE_MAG_FILTER GL_TEXTURE_MAG_FILTER 
     #define CRTEMU_PC_GL_NEAREST GL_NEAREST 
     #define CRTEMU_PC_GL_LINEAR GL_LINEAR 
     #define CRTEMU_PC_GL_STATIC_DRAW GL_STATIC_DRAW 
     #define CRTEMU_PC_GL_RGBA GL_RGBA 
     #define CRTEMU_PC_GL_UNSIGNED_BYTE GL_UNSIGNED_BYTE 
     #define CRTEMU_PC_GL_COLOR_BUFFER_BIT GL_COLOR_BUFFER_BIT 
     #define CRTEMU_PC_GL_TRIANGLE_FAN GL_TRIANGLE_FAN 
     #define CRTEMU_PC_GL_FRAMEBUFFER GL_FRAMEBUFFER 
     #define CRTEMU_PC_GL_VIEWPORT GL_VIEWPORT 
     #define CRTEMU_PC_GL_RGB GL_RGB 
     #define CRTEMU_PC_GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0 
     #define CRTEMU_PC_GL_TEXTURE_WRAP_S GL_TEXTURE_WRAP_S 
     #define CRTEMU_PC_GL_TEXTURE_WRAP_T GL_TEXTURE_WRAP_T 
     #ifndef CRTEMU_PC_WEBGL
         #define CRTEMU_PC_GL_CLAMP_TO_BORDER GL_CLAMP_TO_BORDER 
         #define CRTEMU_PC_GL_TEXTURE_BORDER_COLOR GL_TEXTURE_BORDER_COLOR
     #else
         // WebGL does not support GL_CLAMP_TO_BORDER, we have to emulate
         // this behavior with code in the fragment shader
         #define CRTEMU_PC_GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
     #endif
#endif


struct crtemu_pc_t
    {
    void* memctx;
    crtemu_pc_config_t config;

    CRTEMU_PC_GLuint vertexbuffer;
    CRTEMU_PC_GLuint backbuffer; 

    CRTEMU_PC_GLuint accumulatetexture_a; 
    CRTEMU_PC_GLuint accumulatetexture_b; 
    CRTEMU_PC_GLuint accumulatebuffer_a; 
    CRTEMU_PC_GLuint accumulatebuffer_b; 

    CRTEMU_PC_GLuint blurtexture_a; 
    CRTEMU_PC_GLuint blurtexture_b; 
    CRTEMU_PC_GLuint blurbuffer_a; 
    CRTEMU_PC_GLuint blurbuffer_b; 

    CRTEMU_PC_GLuint frametexture; 
    float use_frame;

    CRTEMU_PC_GLuint crt_shader;   
    CRTEMU_PC_GLuint blur_shader;  
    CRTEMU_PC_GLuint accumulate_shader;    
    CRTEMU_PC_GLuint blend_shader; 
    CRTEMU_PC_GLuint copy_shader;  

    int last_present_width;
    int last_present_height;


    #ifndef CRTEMU_PC_SDL
        struct HINSTANCE__* gl_dll;
        CRTEMU_PC_PROC (CRTEMU_PC_GLCALLTYPE *wglGetProcAddress) (char const* );
    #endif
    
    void (CRTEMU_PC_GLCALLTYPE* TexParameterfv) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLenum pname, CRTEMU_PC_GLfloat const* params);
    void (CRTEMU_PC_GLCALLTYPE* DeleteFramebuffers) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint const* framebuffers);
    void (CRTEMU_PC_GLCALLTYPE* GetIntegerv) (CRTEMU_PC_GLenum pname, CRTEMU_PC_GLint *data);
    void (CRTEMU_PC_GLCALLTYPE* GenFramebuffers) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint *framebuffers);
    void (CRTEMU_PC_GLCALLTYPE* BindFramebuffer) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLuint framebuffer);    
    void (CRTEMU_PC_GLCALLTYPE* Uniform1f) (CRTEMU_PC_GLint location, CRTEMU_PC_GLfloat v0);
    void (CRTEMU_PC_GLCALLTYPE* Uniform2f) (CRTEMU_PC_GLint location, CRTEMU_PC_GLfloat v0, CRTEMU_PC_GLfloat v1);
    void (CRTEMU_PC_GLCALLTYPE* FramebufferTexture2D) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLenum attachment, CRTEMU_PC_GLenum textarget, CRTEMU_PC_GLuint texture, CRTEMU_PC_GLint level);
    CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE* CreateShader) (CRTEMU_PC_GLenum type);
    void (CRTEMU_PC_GLCALLTYPE* ShaderSource) (CRTEMU_PC_GLuint shader, CRTEMU_PC_GLsizei count, CRTEMU_PC_GLchar const* const* string, CRTEMU_PC_GLint const* length);
    void (CRTEMU_PC_GLCALLTYPE* CompileShader) (CRTEMU_PC_GLuint shader);
    void (CRTEMU_PC_GLCALLTYPE* GetShaderiv) (CRTEMU_PC_GLuint shader, CRTEMU_PC_GLenum pname, CRTEMU_PC_GLint *params);
    CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE* CreateProgram) (void);
    void (CRTEMU_PC_GLCALLTYPE* AttachShader) (CRTEMU_PC_GLuint program, CRTEMU_PC_GLuint shader);
    void (CRTEMU_PC_GLCALLTYPE* BindAttribLocation) (CRTEMU_PC_GLuint program, CRTEMU_PC_GLuint index, CRTEMU_PC_GLchar const* name);
    void (CRTEMU_PC_GLCALLTYPE* LinkProgram) (CRTEMU_PC_GLuint program);
    void (CRTEMU_PC_GLCALLTYPE* GetProgramiv) (CRTEMU_PC_GLuint program, CRTEMU_PC_GLenum pname, CRTEMU_PC_GLint *params);
    void (CRTEMU_PC_GLCALLTYPE* GenBuffers) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint *buffers);
    void (CRTEMU_PC_GLCALLTYPE* BindBuffer) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLuint buffer);
    void (CRTEMU_PC_GLCALLTYPE* EnableVertexAttribArray) (CRTEMU_PC_GLuint index);
    void (CRTEMU_PC_GLCALLTYPE* VertexAttribPointer) (CRTEMU_PC_GLuint index, CRTEMU_PC_GLint size, CRTEMU_PC_GLenum type, CRTEMU_PC_GLboolean normalized, CRTEMU_PC_GLsizei stride, void const* pointer);
    void (CRTEMU_PC_GLCALLTYPE* GenTextures) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint* textures);
    void (CRTEMU_PC_GLCALLTYPE* Enable) (CRTEMU_PC_GLenum cap);
    void (CRTEMU_PC_GLCALLTYPE* ActiveTexture) (CRTEMU_PC_GLenum texture);
    void (CRTEMU_PC_GLCALLTYPE* BindTexture) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLuint texture);
    void (CRTEMU_PC_GLCALLTYPE* TexParameteri) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLenum pname, CRTEMU_PC_GLint param);
    void (CRTEMU_PC_GLCALLTYPE* DeleteBuffers) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint const* buffers);
    void (CRTEMU_PC_GLCALLTYPE* DeleteTextures) (CRTEMU_PC_GLsizei n, CRTEMU_PC_GLuint const* textures);
    void (CRTEMU_PC_GLCALLTYPE* BufferData) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLsizeiptr size, void const *data, CRTEMU_PC_GLenum usage);
    void (CRTEMU_PC_GLCALLTYPE* UseProgram) (CRTEMU_PC_GLuint program);
    void (CRTEMU_PC_GLCALLTYPE* Uniform1i) (CRTEMU_PC_GLint location, CRTEMU_PC_GLint v0);
    void (CRTEMU_PC_GLCALLTYPE* Uniform3f) (CRTEMU_PC_GLint location, CRTEMU_PC_GLfloat v0, CRTEMU_PC_GLfloat v1, CRTEMU_PC_GLfloat v2);
    CRTEMU_PC_GLint (CRTEMU_PC_GLCALLTYPE* GetUniformLocation) (CRTEMU_PC_GLuint program, CRTEMU_PC_GLchar const* name);
    void (CRTEMU_PC_GLCALLTYPE* TexImage2D) (CRTEMU_PC_GLenum target, CRTEMU_PC_GLint level, CRTEMU_PC_GLint internalformat, CRTEMU_PC_GLsizei width, CRTEMU_PC_GLsizei height, CRTEMU_PC_GLint border, CRTEMU_PC_GLenum format, CRTEMU_PC_GLenum type, void const* pixels);
    void (CRTEMU_PC_GLCALLTYPE* ClearColor) (CRTEMU_PC_GLfloat red, CRTEMU_PC_GLfloat green, CRTEMU_PC_GLfloat blue, CRTEMU_PC_GLfloat alpha);
    void (CRTEMU_PC_GLCALLTYPE* Clear) (CRTEMU_PC_GLbitfield mask);
    void (CRTEMU_PC_GLCALLTYPE* DrawArrays) (CRTEMU_PC_GLenum mode, CRTEMU_PC_GLint first, CRTEMU_PC_GLsizei count);
    void (CRTEMU_PC_GLCALLTYPE* Viewport) (CRTEMU_PC_GLint x, CRTEMU_PC_GLint y, CRTEMU_PC_GLsizei width, CRTEMU_PC_GLsizei height);
    void (CRTEMU_PC_GLCALLTYPE* DeleteShader) (CRTEMU_PC_GLuint shader);
    void (CRTEMU_PC_GLCALLTYPE* DeleteProgram) (CRTEMU_PC_GLuint program);
    #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
        void (CRTEMU_PC_GLCALLTYPE* GetShaderInfoLog) (CRTEMU_PC_GLuint shader, CRTEMU_PC_GLsizei bufSize, CRTEMU_PC_GLsizei *length, CRTEMU_PC_GLchar *infoLog);
    #endif
    };


static CRTEMU_PC_GLuint crtemu_pc_internal_build_shader( crtemu_pc_t* crtemu_pc, char const* vs_source, char const* fs_source )
    {
    #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
        char error_message[ 1024 ]; 
    #endif

    CRTEMU_PC_GLuint vs = crtemu_pc->CreateShader( CRTEMU_PC_GL_VERTEX_SHADER );
    crtemu_pc->ShaderSource( vs, 1, (char const**) &vs_source, NULL );
    crtemu_pc->CompileShader( vs );
    CRTEMU_PC_GLint vs_compiled;
    crtemu_pc->GetShaderiv( vs, CRTEMU_PC_GL_COMPILE_STATUS, &vs_compiled );
    if( !vs_compiled )
        {
        #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            char const* prefix = "Vertex Shader Error: ";
            strcpy( error_message, prefix );
            int len = 0, written = 0;
            crtemu_pc->GetShaderiv( vs, CRTEMU_PC_GL_INFO_LOG_LENGTH, &len );
            crtemu_pc->GetShaderInfoLog( vs, (CRTEMU_PC_GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
                error_message + strlen( prefix ) );     
            CRTEMU_PC_REPORT_ERROR( error_message );
        #endif
        return 0;
        }
    
    CRTEMU_PC_GLuint fs = crtemu_pc->CreateShader( CRTEMU_PC_GL_FRAGMENT_SHADER );
    crtemu_pc->ShaderSource( fs, 1, (char const**) &fs_source, NULL );
    crtemu_pc->CompileShader( fs );
    CRTEMU_PC_GLint fs_compiled;
    crtemu_pc->GetShaderiv( fs, CRTEMU_PC_GL_COMPILE_STATUS, &fs_compiled );
    if( !fs_compiled )
        {
        #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            char const* prefix = "Fragment Shader Error: ";
            strcpy( error_message, prefix );
            int len = 0, written = 0;
            crtemu_pc->GetShaderiv( vs, CRTEMU_PC_GL_INFO_LOG_LENGTH, &len );
            crtemu_pc->GetShaderInfoLog( fs, (CRTEMU_PC_GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
                error_message + strlen( prefix ) );     
            CRTEMU_PC_REPORT_ERROR( error_message );
        #endif
        return 0;
        }


    CRTEMU_PC_GLuint prg = crtemu_pc->CreateProgram();
    crtemu_pc->AttachShader( prg, fs );
    crtemu_pc->AttachShader( prg, vs );
    crtemu_pc->BindAttribLocation( prg, 0, "pos" );
    crtemu_pc->LinkProgram( prg );

    CRTEMU_PC_GLint linked;
    crtemu_pc->GetProgramiv( prg, CRTEMU_PC_GL_LINK_STATUS, &linked );
    if( !linked )
        {
        #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            char const* prefix = "Shader Link Error: ";
            strcpy( error_message, prefix );
            int len = 0, written = 0;
            crtemu_pc->GetShaderiv( vs, CRTEMU_PC_GL_INFO_LOG_LENGTH, &len );
            crtemu_pc->GetShaderInfoLog( prg, (CRTEMU_PC_GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
                error_message + strlen( prefix ) );     
            CRTEMU_PC_REPORT_ERROR( error_message );
        #endif
        return 0;
        }

    return prg;
    }


crtemu_pc_t* crtemu_pc_create( void* memctx )
    {

    char const* vs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
        ""
        "attribute vec4 pos;"
        "varying vec2 uv;"
        ""
        "void main( void )"
        "    {"
        "    gl_Position = vec4( pos.xy, 0.0, 1.0 );"
        "    uv = pos.zw;"
        "    }";

    char const* crt_fs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
        "\n"
        "varying vec2 uv;\n"
        "\n"
        "uniform vec3 modulate;\n"
        "uniform vec2 resolution;\n"
        "uniform vec2 size;\n"
        "uniform float time;\n"
		"uniform sampler2D backbuffer;\n"
        "uniform sampler2D blurbuffer;\n"
        "uniform sampler2D frametexture;\n"
        "uniform float use_frame;\n"
        "\n"
        #ifdef CRTEMU_PC_WEBGL
            // WebGL does not support GL_CLAMP_TO_BORDER so we overwrite texture2D
            // with this function which emulates the clamp-to-border behavior
            "vec4 texture2Dborder(sampler2D samp, vec2 tc)\n"
            "    {\n"
            "    float borderdist = .502-max(abs(.5-tc.x), abs(.5-tc.y));\n"
            "    float borderfade = clamp(borderdist * 400.0, 0.0, 1.0);\n"
            "    return texture2D( samp, tc ) * borderfade;\n"
            "    }\n"
            "#define texture2D texture2Dborder\n"
        #endif
        "vec3 tsample( sampler2D samp, vec2 tc, float offs, vec2 resolution )\n"
	    "    {\n"
	    "    tc = tc * vec2(1.035, 0.96) + vec2( mix( -0.018,-0.0125*0.75,use_frame), 0.02);\n"
		"	tc = tc * 1.2 - 0.1;\n"
	    "    vec3 s = pow( abs( texture2D( samp, vec2( tc.x, 1.0-tc.y ) ).rgb), vec3( 2.2 ) );\n"
	    "    return s*vec3(1.25);\n"
	    "    }\n"
        "\n"
        "vec3 filmic( vec3 LinearColor )\n"
	    "    {\n"
	    "    vec3 x = max( vec3(0.0), LinearColor-vec3(0.004));\n"
	    "    return (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);\n"
	    "    }\n"
        "\n"
        "vec2 curve( vec2 uv )\n"
	    "    {\n"
	    "    uv = (uv - 0.5) * 2.0;\n"
	    "    uv *= 1.1;	\n"
	    "    uv.x *= 1.0 + pow((abs(uv.y) / 5.0), 2.0);\n"
	    "    uv.y *= 1.0 + pow((abs(uv.x) / 4.0), 2.0);\n"
	    "    uv  = (uv / 2.0) + 0.5;\n"
	    "    uv =  uv *0.92 + 0.04;\n"
	    "    return uv;\n"
	    "    }\n"
        "\n"
        "float rand(vec2 co)\n"
	    "    {\n"
        "    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
	    "    }\n"
	    "    \n"
        "void main(void)\n"
		"	{\n"
	    "    /* Curve */\n"
	    "    vec2 curved_uv = mix( curve( uv ), uv, 0.8 );\n"
	    "    float scale = 0.04;\n"
	    "    vec2 scuv = curved_uv*(1.0-scale)+scale/2.0+vec2(0.003, -0.001);\n"
        "\n"
	    "    /* Main color, Bleed */\n"
	    "    vec3 col;\n"
	    "    float x =  sin(0.1*time+curved_uv.y*13.0)*sin(0.23*time+curved_uv.y*19.0)*sin(0.3+0.11*time+curved_uv.y*23.0)*0.0012;\n"
	    "    float o =sin(gl_FragCoord.y*1.5)/resolution.x;\n"
	    "    x+=o*0.25;\n"
		"	x *= 0.2;\n"
	    "    col.r = tsample(backbuffer,vec2(x+scuv.x+0.0009*0.25,scuv.y+0.0009*0.25),resolution.y/800.0, resolution ).x+0.02;\n"
	    "    col.g = tsample(backbuffer,vec2(x+scuv.x+0.0000*0.25,scuv.y-0.0011*0.25),resolution.y/800.0, resolution ).y+0.02;\n"
	    "    col.b = tsample(backbuffer,vec2(x+scuv.x-0.0015*0.25,scuv.y+0.0000*0.25),resolution.y/800.0, resolution ).z+0.02;\n"
	    "    float i = clamp(col.r*0.299 + col.g*0.587 + col.b*0.114, 0.0, 1.0 );		\n"
	    "    i = pow( 1.0 - pow(i,2.0), 1.0 );\n"
	    "    i = (1.0-i) * 0.85 + 0.15;	\n"
        "\n"
	    "    /* Ghosting */\n"
        "    float ghs = 0.05;\n"
	    "    vec3 r = tsample(blurbuffer, vec2(x-0.014*1.0, -0.027)*0.45+0.007*vec2( 0.35*sin(1.0/7.0 + 15.0*curved_uv.y + 0.9*time), \n"
        "        0.35*sin( 2.0/7.0 + 10.0*curved_uv.y + 1.37*time) )+vec2(scuv.x+0.001,scuv.y+0.001),\n"
        "        5.5+1.3*sin( 3.0/9.0 + 31.0*curved_uv.x + 1.70*time),resolution).xyz*vec3(0.5,0.25,0.25);\n"
	    "    vec3 g = tsample(blurbuffer, vec2(x-0.019*1.0, -0.020)*0.45+0.007*vec2( 0.35*cos(1.0/9.0 + 15.0*curved_uv.y + 0.5*time), \n"
        "        0.35*sin( 2.0/9.0 + 10.0*curved_uv.y + 1.50*time) )+vec2(scuv.x+0.000,scuv.y-0.002),\n"
        "        5.4+1.3*sin( 3.0/3.0 + 71.0*curved_uv.x + 1.90*time),resolution).xyz*vec3(0.25,0.5,0.25);\n"
	    "    vec3 b = tsample(blurbuffer, vec2(x-0.017*1.0, -0.003)*0.35+0.007*vec2( 0.35*sin(2.0/3.0 + 15.0*curved_uv.y + 0.7*time), \n"
        "        0.35*cos( 2.0/3.0 + 10.0*curved_uv.y + 1.63*time) )+vec2(scuv.x-0.002,scuv.y+0.000),\n"
        "        5.3+1.3*sin( 3.0/7.0 + 91.0*curved_uv.x + 1.65*time),resolution).xyz*vec3(0.25,0.25,0.5);\n"
	    "\n"
	    "    col += vec3(ghs*(1.0-0.299))*pow(clamp(vec3(3.0)*r,vec3(0.0),vec3(1.0)),vec3(2.0))*vec3(i);\n"
        "    col += vec3(ghs*(1.0-0.587))*pow(clamp(vec3(3.0)*g,vec3(0.0),vec3(1.0)),vec3(2.0))*vec3(i);\n"
        "    col += vec3(ghs*(1.0-0.114))*pow(clamp(vec3(3.0)*b,vec3(0.0),vec3(1.0)),vec3(2.0))*vec3(i);\n"
	    "\n"
	    "    /* Level adjustment (curves) */\n"
	    "    col *= vec3(1.0,1.1,1.0);\n"
        "    col = clamp(col*1.3 + 0.75*col*col + 1.25*col*col*col*col*col,vec3(0.0),vec3(10.0));\n"
	    "\n"
	    "    /* Vignette */\n"
        "    float vig = (0.1 + 1.0*16.0*curved_uv.x*curved_uv.y*(1.0-curved_uv.x)*(1.0-curved_uv.y));\n"
	    "    vig = 1.3*pow(vig,0.5);\n"
	    "    col *= vig;\n"
	    "\n"
	    "    /* Scanlines */\n"
	    "    float scans = clamp( 0.5+0.2*sin(cos(20.0*time)*0.32+curved_uv.y*size.y*1.75), 0.0, 1.0);\n"
	    "    float s = pow(scans,0.9);\n"
	    "    col = col * vec3(s);\n"
        "\n"
	    "    /* Vertical lines (shadow mask) */\n"
	    "    col*=1.0-0.23*(clamp((mod(gl_FragCoord.xy.x, 3.0))/2.0,0.0,1.0));\n"
        "\n"
	    "    /* Tone map */\n"
	    "    col = filmic( col );\n"
        "\n"
	    "    /* Noise */\n"
	    "    //vec2 seed = floor(curved_uv*resolution.xy*vec2(0.5))/resolution.xy;\n"
        "    vec2 seed = curved_uv*resolution.xy;;\n"
	    "    /* seed = curved_uv; */\n"
	    "    col -= 0.015*pow(vec3(rand( seed +time ), rand( seed +time*2.0 ), rand( seed +time * 3.0 ) ), vec3(1.5) );\n"
	    "\n"
	    "    /* Flicker */\n"
        "    col *= (1.0-0.004*(sin(50.0*time+curved_uv.y*2.0)*0.5+0.5));\n"
        "\n"
	    "    /* Clamp */\n"
	    "    if (curved_uv.x < 0.0 || curved_uv.x > 1.0)\n"
		"        col *= 0.0;\n"
	    "    if (curved_uv.y < 0.0 || curved_uv.y > 1.0)\n"
		"        col *= 0.0;\n"
		"    col*=modulate; \n"
	    "    /* Frame */\n"
	    "    vec2 fscale = vec2( -0.019, -0.018 );\n"
		"	vec2 fuv=vec2( uv.x, 1.0 - uv.y)*((1.0)+2.0*fscale)-fscale-vec2(-0.0, 0.005);\n"
	    "    vec4 f=texture2D(frametexture, fuv * vec2(0.925, 0.81) + vec2( 0.042, 0.09 ));\n"
	    "    f.xyz = mix( f.xyz, vec3(0.5,0.5,0.5), 0.5 );\n"
	    "    float fvig = clamp( -0.00+512.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.2, 0.85 );\n"
		"	col *= fvig;\n"
        "    float expon = 1.4;\n"
        "//    f.xyz = vec3(26.0/255.0,26.0/255.0,26.0/255.0);expon=1.0;\n"
	    "    col = mix( col, mix( max( col, 0.0), pow( abs( f.xyz ), vec3( expon ) ), f.w), vec3( use_frame) );\n"
        "    \n"
		"	gl_FragColor = vec4( col, 1.0 );\n"
		"	}\n"
		"	\n"
        "";

    char const* blur_fs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
		""
        "varying vec2 uv;"
		""
        "uniform vec2 blur;"
        "uniform sampler2D texture;"
		""
        "void main( void )"
        "    {"
        "    vec4 sum = texture2D( texture, uv ) * 0.2270270270;"
        "    sum += texture2D(texture, vec2( uv.x - 4.0 * blur.x, uv.y - 4.0 * blur.y ) ) * 0.0162162162;"
        "    sum += texture2D(texture, vec2( uv.x - 3.0 * blur.x, uv.y - 3.0 * blur.y ) ) * 0.0540540541;"
        "    sum += texture2D(texture, vec2( uv.x - 2.0 * blur.x, uv.y - 2.0 * blur.y ) ) * 0.1216216216;"
        "    sum += texture2D(texture, vec2( uv.x - 1.0 * blur.x, uv.y - 1.0 * blur.y ) ) * 0.1945945946;"
        "    sum += texture2D(texture, vec2( uv.x + 1.0 * blur.x, uv.y + 1.0 * blur.y ) ) * 0.1945945946;"
        "    sum += texture2D(texture, vec2( uv.x + 2.0 * blur.x, uv.y + 2.0 * blur.y ) ) * 0.1216216216;"
        "    sum += texture2D(texture, vec2( uv.x + 3.0 * blur.x, uv.y + 3.0 * blur.y ) ) * 0.0540540541;"
        "    sum += texture2D(texture, vec2( uv.x + 4.0 * blur.x, uv.y + 4.0 * blur.y ) ) * 0.0162162162;"
        "    gl_FragColor = sum;"
        "    }   "
		"";


    char const* accumulate_fs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
		""
        "varying vec2 uv;"
		""
        "uniform sampler2D tex0;"
        "uniform sampler2D tex1;"
        "uniform float modulate;"
		""
        "void main( void )"
        "    {"
        "    vec4 a = texture2D( tex0, uv ) * vec4( modulate );"
        "    vec4 b = texture2D( tex1, uv );"
		""
        "    gl_FragColor = max( a, b * 0.96 );"
        "    }   "
		"";

    char const* blend_fs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
		""
        "varying vec2 uv;"
		""
        "uniform sampler2D tex0;"
        "uniform sampler2D tex1;"
        "uniform float modulate;"
		""
        "void main( void )"
        "    {"
        "    vec4 a = texture2D( tex0, uv ) * vec4( modulate );"
        "    vec4 b = texture2D( tex1, uv );"
		""
        "    gl_FragColor = max( a, b * 0.24 );"
        "    }   "
		"";

    char const* copy_fs_source = 
        #ifdef CRTEMU_PC_WEBGL
            "precision highp float;\n\n"
        #else
            "#version 120\n\n"
        #endif
		""
        "varying vec2 uv;"
		""
        "uniform sampler2D tex0;"
		""
        "void main( void )"
        "    {"
        "    gl_FragColor = texture2D( tex0, uv );"
        "    }   "
		"";
	
    crtemu_pc_t* crtemu_pc = (crtemu_pc_t*) CRTEMU_PC_MALLOC( memctx, sizeof( crtemu_pc_t ) );
    memset( crtemu_pc, 0, sizeof( crtemu_pc_t ) );
    crtemu_pc->memctx = memctx;

    crtemu_pc->config.curvature = 0.0f;
    crtemu_pc->config.scanlines = 0.0f;
    crtemu_pc->config.shadow_mask = 0.0f;
    crtemu_pc->config.separation = 0.0f;
    crtemu_pc->config.ghosting = 0.0f;
    crtemu_pc->config.noise = 0.0f;
    crtemu_pc->config.flicker = 0.0f;
    crtemu_pc->config.vignette = 0.0f;
    crtemu_pc->config.distortion = 0.0f;
    crtemu_pc->config.aspect_lock = 0.0f;
    crtemu_pc->config.hpos = 0.0f;
    crtemu_pc->config.vpos = 0.0f;
    crtemu_pc->config.hsize = 0.0f;
    crtemu_pc->config.vsize = 0.0f;
    crtemu_pc->config.contrast = 0.0f;
    crtemu_pc->config.brightness = 0.0f;
    crtemu_pc->config.saturation = 0.0f;
    crtemu_pc->config.blur = 0.0f;
    crtemu_pc->config.degauss = 0.0f;

    crtemu_pc->use_frame = 0.0f;
    
    crtemu_pc->last_present_width = 0;
    crtemu_pc->last_present_height = 0;
    
    #ifndef CRTEMU_PC_SDL

        crtemu_pc->gl_dll = LoadLibraryA( "opengl32.dll" );
        if( !crtemu_pc->gl_dll ) goto failed;

        crtemu_pc->wglGetProcAddress = (CRTEMU_PC_PROC (CRTEMU_PC_GLCALLTYPE*)(char const*)) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "wglGetProcAddress" );
        if( !crtemu_pc->gl_dll ) goto failed;

        // Attempt to bind opengl functions using GetProcAddress
        crtemu_pc->TexParameterfv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLfloat const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glTexParameterfv" );
        crtemu_pc->DeleteFramebuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDeleteFramebuffers" );
        crtemu_pc->GetIntegerv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint *) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGetIntegerv" );
        crtemu_pc->GenFramebuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint *) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGenFramebuffers" );
        crtemu_pc->BindFramebuffer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glBindFramebuffer" );    
        crtemu_pc->Uniform1f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glUniform1f" );
        crtemu_pc->Uniform2f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glUniform2f" );
        crtemu_pc->FramebufferTexture2D = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLuint, CRTEMU_PC_GLint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glFramebufferTexture2D" );    
        crtemu_pc->CreateShader = ( CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glCreateShader" );
        crtemu_pc->ShaderSource = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLchar const* const*, CRTEMU_PC_GLint const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glShaderSource" );
        crtemu_pc->CompileShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glCompileShader" );
        crtemu_pc->GetShaderiv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLenum, CRTEMU_PC_GLint*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGetShaderiv" );
        crtemu_pc->CreateProgram = ( CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE*) (void) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glCreateProgram" );
        crtemu_pc->AttachShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glAttachShader" );
        crtemu_pc->BindAttribLocation = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLuint, CRTEMU_PC_GLchar const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glBindAttribLocation" );
        crtemu_pc->LinkProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glLinkProgram" );
        crtemu_pc->GetProgramiv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLenum, CRTEMU_PC_GLint*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGetProgramiv" );
        crtemu_pc->GenBuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGenBuffers" );
        crtemu_pc->BindBuffer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glBindBuffer" );
        crtemu_pc->EnableVertexAttribArray = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glEnableVertexAttribArray" );
        crtemu_pc->VertexAttribPointer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLint, CRTEMU_PC_GLenum, CRTEMU_PC_GLboolean, CRTEMU_PC_GLsizei, void const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glVertexAttribPointer" );
        crtemu_pc->GenTextures = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGenTextures" );
        crtemu_pc->Enable = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glEnable" );
        crtemu_pc->ActiveTexture = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glActiveTexture" );
        crtemu_pc->BindTexture = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glBindTexture" );
        crtemu_pc->TexParameteri = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glTexParameteri" );
        crtemu_pc->DeleteBuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDeleteBuffers" );
        crtemu_pc->DeleteTextures = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDeleteTextures" );
        crtemu_pc->BufferData = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLsizeiptr, void const *, CRTEMU_PC_GLenum) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glBufferData" );
        crtemu_pc->UseProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glUseProgram" );
        crtemu_pc->Uniform1i = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glUniform1i" );
        crtemu_pc->Uniform3f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glUniform3f" );
        crtemu_pc->GetUniformLocation = ( CRTEMU_PC_GLint (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLchar const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGetUniformLocation" );
        crtemu_pc->TexImage2D = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei, CRTEMU_PC_GLint, CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, void const*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glTexImage2D" );
        crtemu_pc->ClearColor = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glClearColor" );
        crtemu_pc->Clear = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLbitfield) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glClear" );
        crtemu_pc->DrawArrays = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDrawArrays" );
        crtemu_pc->Viewport = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glViewport" );
        crtemu_pc->DeleteShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDeleteShader" );
        crtemu_pc->DeleteProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glDeleteProgram" );
        #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            crtemu_pc->GetShaderInfoLog = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei*, CRTEMU_PC_GLchar*) ) (uintptr_t) GetProcAddress( crtemu_pc->gl_dll, "glGetShaderInfoLog" );
        #endif

        // Any opengl functions which didn't bind, try binding them using wglGetProcAddrss
        if( !crtemu_pc->TexParameterfv ) crtemu_pc->TexParameterfv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLfloat const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glTexParameterfv" );
        if( !crtemu_pc->DeleteFramebuffers ) crtemu_pc->DeleteFramebuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDeleteFramebuffers" );
        if( !crtemu_pc->GetIntegerv ) crtemu_pc->GetIntegerv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint *) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGetIntegerv" );
        if( !crtemu_pc->GenFramebuffers ) crtemu_pc->GenFramebuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint *) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGenFramebuffers" );
        if( !crtemu_pc->BindFramebuffer ) crtemu_pc->BindFramebuffer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glBindFramebuffer" );    
        if( !crtemu_pc->Uniform1f ) crtemu_pc->Uniform1f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glUniform1f" );
        if( !crtemu_pc->Uniform2f ) crtemu_pc->Uniform2f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glUniform2f" );
        if( !crtemu_pc->FramebufferTexture2D ) crtemu_pc->FramebufferTexture2D = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLuint, CRTEMU_PC_GLint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glFramebufferTexture2D" );    
        if( !crtemu_pc->CreateShader ) crtemu_pc->CreateShader = ( CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glCreateShader" );
        if( !crtemu_pc->ShaderSource ) crtemu_pc->ShaderSource = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLchar const* const*, CRTEMU_PC_GLint const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glShaderSource" );
        if( !crtemu_pc->CompileShader ) crtemu_pc->CompileShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glCompileShader" );
        if( !crtemu_pc->GetShaderiv ) crtemu_pc->GetShaderiv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLenum, CRTEMU_PC_GLint*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGetShaderiv" );
        if( !crtemu_pc->CreateProgram ) crtemu_pc->CreateProgram = ( CRTEMU_PC_GLuint (CRTEMU_PC_GLCALLTYPE*) (void) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glCreateProgram" );
        if( !crtemu_pc->AttachShader ) crtemu_pc->AttachShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glAttachShader" );
        if( !crtemu_pc->BindAttribLocation ) crtemu_pc->BindAttribLocation = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLuint, CRTEMU_PC_GLchar const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glBindAttribLocation" );
        if( !crtemu_pc->LinkProgram ) crtemu_pc->LinkProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glLinkProgram" );
        if( !crtemu_pc->GetProgramiv ) crtemu_pc->GetProgramiv = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLenum, CRTEMU_PC_GLint*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGetProgramiv" );
        if( !crtemu_pc->GenBuffers ) crtemu_pc->GenBuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGenBuffers" );
        if( !crtemu_pc->BindBuffer ) crtemu_pc->BindBuffer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glBindBuffer" );
        if( !crtemu_pc->EnableVertexAttribArray ) crtemu_pc->EnableVertexAttribArray = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glEnableVertexAttribArray" );
        if( !crtemu_pc->VertexAttribPointer ) crtemu_pc->VertexAttribPointer = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLint, CRTEMU_PC_GLenum, CRTEMU_PC_GLboolean, CRTEMU_PC_GLsizei, void const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glVertexAttribPointer" );
        if( !crtemu_pc->GenTextures ) crtemu_pc->GenTextures = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGenTextures" );
        if( !crtemu_pc->Enable ) crtemu_pc->Enable = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glEnable" );
        if( !crtemu_pc->ActiveTexture ) crtemu_pc->ActiveTexture = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glActiveTexture" );
        if( !crtemu_pc->BindTexture ) crtemu_pc->BindTexture = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glBindTexture" );
        if( !crtemu_pc->TexParameteri ) crtemu_pc->TexParameteri = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, CRTEMU_PC_GLint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glTexParameteri" );
        if( !crtemu_pc->DeleteBuffers ) crtemu_pc->DeleteBuffers = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDeleteBuffers" );
        if( !crtemu_pc->DeleteTextures ) crtemu_pc->DeleteTextures = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLsizei, CRTEMU_PC_GLuint const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDeleteTextures" );
        if( !crtemu_pc->BufferData ) crtemu_pc->BufferData = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLsizeiptr, void const *, CRTEMU_PC_GLenum) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glBufferData" );
        if( !crtemu_pc->UseProgram ) crtemu_pc->UseProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glUseProgram" );
        if( !crtemu_pc->Uniform1i ) crtemu_pc->Uniform1i = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glUniform1i" );
        if( !crtemu_pc->Uniform3f ) crtemu_pc->Uniform3f = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glUniform3f" );
        if( !crtemu_pc->GetUniformLocation ) crtemu_pc->GetUniformLocation = ( CRTEMU_PC_GLint (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLchar const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGetUniformLocation" );
        if( !crtemu_pc->TexImage2D ) crtemu_pc->TexImage2D = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei, CRTEMU_PC_GLint, CRTEMU_PC_GLenum, CRTEMU_PC_GLenum, void const*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glTexImage2D" );
        if( !crtemu_pc->ClearColor ) crtemu_pc->ClearColor = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat, CRTEMU_PC_GLfloat) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glClearColor" );
        if( !crtemu_pc->Clear ) crtemu_pc->Clear = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLbitfield) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glClear" );
        if( !crtemu_pc->DrawArrays ) crtemu_pc->DrawArrays = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLenum, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDrawArrays" );
        if( !crtemu_pc->Viewport ) crtemu_pc->Viewport = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLint, CRTEMU_PC_GLint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glViewport" );
        if( !crtemu_pc->DeleteShader ) crtemu_pc->DeleteShader = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDeleteShader" );
        if( !crtemu_pc->DeleteProgram ) crtemu_pc->DeleteProgram = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glDeleteProgram" );
        #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            if( !crtemu_pc->GetShaderInfoLog ) crtemu_pc->GetShaderInfoLog = ( void (CRTEMU_PC_GLCALLTYPE*) (CRTEMU_PC_GLuint, CRTEMU_PC_GLsizei, CRTEMU_PC_GLsizei*, CRTEMU_PC_GLchar*) ) (uintptr_t) crtemu_pc->wglGetProcAddress( "glGetShaderInfoLog" );
        #endif

    #else

         crtemu_pc->TexParameterfv = glTexParameterfv;
         crtemu_pc->DeleteFramebuffers = glDeleteFramebuffers;
         crtemu_pc->GetIntegerv = glGetIntegerv;
         crtemu_pc->GenFramebuffers = glGenFramebuffers;
         crtemu_pc->BindFramebuffer = glBindFramebuffer;
         crtemu_pc->Uniform1f = glUniform1f;
         crtemu_pc->Uniform2f = glUniform2f;
         crtemu_pc->FramebufferTexture2D = glFramebufferTexture2D;
         crtemu_pc->CreateShader = glCreateShader;
         crtemu_pc->ShaderSource = glShaderSource;
         crtemu_pc->CompileShader = glCompileShader;
         crtemu_pc->GetShaderiv = glGetShaderiv;
         crtemu_pc->CreateProgram = glCreateProgram;
         crtemu_pc->AttachShader = glAttachShader;
         crtemu_pc->BindAttribLocation = glBindAttribLocation;
         crtemu_pc->LinkProgram = glLinkProgram;
         crtemu_pc->GetProgramiv = glGetProgramiv;
         crtemu_pc->GenBuffers = glGenBuffers;
         crtemu_pc->BindBuffer = glBindBuffer;
         crtemu_pc->EnableVertexAttribArray = glEnableVertexAttribArray;
         crtemu_pc->VertexAttribPointer = glVertexAttribPointer;
         crtemu_pc->GenTextures = glGenTextures;
         crtemu_pc->Enable = glEnable;
         crtemu_pc->ActiveTexture = glActiveTexture;
         crtemu_pc->BindTexture = glBindTexture;
         crtemu_pc->TexParameteri = glTexParameteri;
         crtemu_pc->DeleteBuffers = glDeleteBuffers;
         crtemu_pc->DeleteTextures = glDeleteTextures;
         crtemu_pc->BufferData = glBufferData;
         crtemu_pc->UseProgram = glUseProgram;
         crtemu_pc->Uniform1i = glUniform1i;
         crtemu_pc->Uniform3f = glUniform3f;
         crtemu_pc->GetUniformLocation = glGetUniformLocation;
         crtemu_pc->TexImage2D = glTexImage2D;
         crtemu_pc->ClearColor = glClearColor;
         crtemu_pc->Clear = glClear;
         crtemu_pc->DrawArrays = glDrawArrays;
         crtemu_pc->Viewport = glViewport;
         crtemu_pc->DeleteShader = glDeleteShader;
         crtemu_pc->DeleteProgram = glDeleteProgram;
         #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
            crtemu_pc->GetShaderInfoLog = glGetShaderInfoLog;
         #endif
        
    #endif

    // Report error if any gl function was not found.
    if( !crtemu_pc->TexParameterfv ) goto failed;
    if( !crtemu_pc->DeleteFramebuffers ) goto failed;
    if( !crtemu_pc->GetIntegerv ) goto failed;
    if( !crtemu_pc->GenFramebuffers ) goto failed;
    if( !crtemu_pc->BindFramebuffer ) goto failed;
    if( !crtemu_pc->Uniform1f ) goto failed;
    if( !crtemu_pc->Uniform2f ) goto failed;
    if( !crtemu_pc->FramebufferTexture2D ) goto failed;
    if( !crtemu_pc->CreateShader ) goto failed;
    if( !crtemu_pc->ShaderSource ) goto failed;
    if( !crtemu_pc->CompileShader ) goto failed;
    if( !crtemu_pc->GetShaderiv ) goto failed;
    if( !crtemu_pc->CreateProgram ) goto failed;
    if( !crtemu_pc->AttachShader ) goto failed;
    if( !crtemu_pc->BindAttribLocation ) goto failed;
    if( !crtemu_pc->LinkProgram ) goto failed;
    if( !crtemu_pc->GetProgramiv ) goto failed;
    if( !crtemu_pc->GenBuffers ) goto failed;
    if( !crtemu_pc->BindBuffer ) goto failed;
    if( !crtemu_pc->EnableVertexAttribArray ) goto failed;
    if( !crtemu_pc->VertexAttribPointer ) goto failed;
    if( !crtemu_pc->GenTextures ) goto failed;
    if( !crtemu_pc->Enable ) goto failed;
    if( !crtemu_pc->ActiveTexture ) goto failed;
    if( !crtemu_pc->BindTexture ) goto failed;
    if( !crtemu_pc->TexParameteri ) goto failed;
    if( !crtemu_pc->DeleteBuffers ) goto failed;
    if( !crtemu_pc->DeleteTextures ) goto failed;
    if( !crtemu_pc->BufferData ) goto failed;
    if( !crtemu_pc->UseProgram ) goto failed;
    if( !crtemu_pc->Uniform1i ) goto failed;
    if( !crtemu_pc->Uniform3f ) goto failed;
    if( !crtemu_pc->GetUniformLocation ) goto failed;
    if( !crtemu_pc->TexImage2D ) goto failed;
    if( !crtemu_pc->ClearColor ) goto failed;
    if( !crtemu_pc->Clear ) goto failed;
    if( !crtemu_pc->DrawArrays ) goto failed;
    if( !crtemu_pc->Viewport ) goto failed;
    if( !crtemu_pc->DeleteShader ) goto failed;
    if( !crtemu_pc->DeleteProgram ) goto failed;
    #ifdef CRTEMU_PC_REPORT_SHADER_ERRORS
        if( !crtemu_pc->GetShaderInfoLog ) goto failed;
    #endif

    crtemu_pc->crt_shader = crtemu_pc_internal_build_shader( crtemu_pc, vs_source, crt_fs_source );
    if( crtemu_pc->crt_shader == 0 ) goto failed;

    crtemu_pc->blur_shader = crtemu_pc_internal_build_shader( crtemu_pc, vs_source, blur_fs_source );
    if( crtemu_pc->blur_shader == 0 ) goto failed;

    crtemu_pc->accumulate_shader = crtemu_pc_internal_build_shader( crtemu_pc, vs_source, accumulate_fs_source );
    if( crtemu_pc->accumulate_shader == 0 ) goto failed;

    crtemu_pc->blend_shader = crtemu_pc_internal_build_shader( crtemu_pc, vs_source, blend_fs_source );
    if( crtemu_pc->blend_shader == 0 ) goto failed;

    crtemu_pc->copy_shader = crtemu_pc_internal_build_shader( crtemu_pc, vs_source, copy_fs_source );
    if( crtemu_pc->copy_shader == 0 ) goto failed;

    crtemu_pc->GenTextures( 1, &crtemu_pc->accumulatetexture_a );
    crtemu_pc->GenFramebuffers( 1, &crtemu_pc->accumulatebuffer_a );

    crtemu_pc->GenTextures( 1, &crtemu_pc->accumulatetexture_b );
    crtemu_pc->GenFramebuffers( 1, &crtemu_pc->accumulatebuffer_b );

    crtemu_pc->GenTextures( 1, &crtemu_pc->blurtexture_a );
    crtemu_pc->GenFramebuffers( 1, &crtemu_pc->blurbuffer_a );

    crtemu_pc->GenTextures( 1, &crtemu_pc->blurtexture_b );
    crtemu_pc->GenFramebuffers( 1, &crtemu_pc->blurbuffer_b );

    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

    crtemu_pc->GenTextures( 1, &crtemu_pc->frametexture ); 
    #ifndef CRTEMU_PC_WEBGL
        // This enable call is not necessary when using fragment shaders, avoid logged warnings in WebGL
        crtemu_pc->Enable( CRTEMU_PC_GL_TEXTURE_2D ); 
    #endif
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE2 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->frametexture );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );

    crtemu_pc->GenTextures( 1, &crtemu_pc->backbuffer ); 
    #ifndef CRTEMU_PC_WEBGL
        // This enable call is not necessary when using fragment shaders, avoid logged warnings in WebGL
        crtemu_pc->Enable( CRTEMU_PC_GL_TEXTURE_2D ); 
    #endif
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->backbuffer );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_NEAREST );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_NEAREST );

    crtemu_pc->GenBuffers( 1, &crtemu_pc->vertexbuffer );
    crtemu_pc->BindBuffer( CRTEMU_PC_GL_ARRAY_BUFFER, crtemu_pc->vertexbuffer );
    crtemu_pc->EnableVertexAttribArray( 0 );
    crtemu_pc->VertexAttribPointer( 0, 4, CRTEMU_PC_GL_FLOAT, CRTEMU_PC_GL_FALSE, 4 * sizeof( CRTEMU_PC_GLfloat ), 0 );

    #ifdef CRTEMU_PC_WEBGL
        // Avoid WebGL error "TEXTURE_2D at unit 0 is incomplete: Non-power-of-two textures must have a wrap mode of CLAMP_TO_EDGE."
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_a );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_b );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_a );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_b );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->frametexture );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->backbuffer );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    #endif

    return crtemu_pc;

failed:
    if( crtemu_pc->accumulatetexture_a ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->accumulatetexture_a );
    if( crtemu_pc->accumulatebuffer_a ) crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->accumulatebuffer_a );
    if( crtemu_pc->accumulatetexture_b ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->accumulatetexture_b );
    if( crtemu_pc->accumulatebuffer_b ) crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->accumulatebuffer_b );
    if( crtemu_pc->blurtexture_a ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->blurtexture_a );
    if( crtemu_pc->blurbuffer_a ) crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->blurbuffer_a );
    if( crtemu_pc->blurtexture_b ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->blurtexture_b );
    if( crtemu_pc->blurbuffer_b ) crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->blurbuffer_b );
    if( crtemu_pc->frametexture ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->frametexture ); 
    if( crtemu_pc->backbuffer ) crtemu_pc->DeleteTextures( 1, &crtemu_pc->backbuffer ); 
    if( crtemu_pc->vertexbuffer ) crtemu_pc->DeleteBuffers( 1, &crtemu_pc->vertexbuffer );

    #ifndef CRTEMU_PC_SDL
        if( crtemu_pc->gl_dll ) FreeLibrary( crtemu_pc->gl_dll );
    #endif
    CRTEMU_PC_FREE( crtemu_pc->memctx, crtemu_pc );
    return 0;
    }


void crtemu_pc_destroy( crtemu_pc_t* crtemu_pc )
    {
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->accumulatetexture_a );
    crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->accumulatebuffer_a );
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->accumulatetexture_b );
    crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->accumulatebuffer_b );
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->blurtexture_a );
    crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->blurbuffer_a );
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->blurtexture_b );
    crtemu_pc->DeleteFramebuffers( 1, &crtemu_pc->blurbuffer_b );
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->frametexture ); 
    crtemu_pc->DeleteTextures( 1, &crtemu_pc->backbuffer ); 
    crtemu_pc->DeleteBuffers( 1, &crtemu_pc->vertexbuffer );
    #ifndef CRTEMU_PC_SDL
        FreeLibrary( crtemu_pc->gl_dll );
    #endif
    CRTEMU_PC_FREE( crtemu_pc->memctx, crtemu_pc );
    }


void crtemu_pc_config(crtemu_pc_t* crtemu_pc, crtemu_pc_config_t const* config)
    {
    crtemu_pc->config = *config;
    }


void crtemu_pc_frame( crtemu_pc_t* crtemu_pc, CRTEMU_PC_U32* frame_abgr, int frame_width, int frame_height )
    {
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE3 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->frametexture );   
    crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGBA, frame_width, frame_height, 0, CRTEMU_PC_GL_RGBA, CRTEMU_PC_GL_UNSIGNED_BYTE, frame_abgr ); 
    if( frame_abgr )
        crtemu_pc->use_frame = 1.0f;
    else
        crtemu_pc->use_frame = 0.0f;
    }


static void crtemu_pc_internal_blur( crtemu_pc_t* crtemu_pc, CRTEMU_PC_GLuint source, CRTEMU_PC_GLuint blurbuffer_a, CRTEMU_PC_GLuint blurbuffer_b, 
    CRTEMU_PC_GLuint blurtexture_b, float r, int width, int height )
    {
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, blurbuffer_b );
    crtemu_pc->UseProgram( crtemu_pc->blur_shader );
    crtemu_pc->Uniform2f( crtemu_pc->GetUniformLocation( crtemu_pc->blur_shader, "blur" ), r / (float) width, 0 );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->blur_shader, "texture" ), 0 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, source );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, blurbuffer_a );
    crtemu_pc->UseProgram( crtemu_pc->blur_shader );
    crtemu_pc->Uniform2f( crtemu_pc->GetUniformLocation( crtemu_pc->blur_shader, "blur" ), 0, r / (float) height );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->blur_shader, "texture" ), 0 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, blurtexture_b );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );
    }


void crtemu_pc_present( crtemu_pc_t* crtemu_pc, CRTEMU_PC_U64 time_us, CRTEMU_PC_U32 const* pixels_xbgr, int width, int height, 
    CRTEMU_PC_U32 mod_xbgr, CRTEMU_PC_U32 border_xbgr )
    {
    int viewport[ 4 ];
    crtemu_pc->GetIntegerv( CRTEMU_PC_GL_VIEWPORT, viewport );

    if( width != crtemu_pc->last_present_width || height != crtemu_pc->last_present_height )
        {
        crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
 
        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_a );
        crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGB, width, height, 0, CRTEMU_PC_GL_RGB, CRTEMU_PC_GL_UNSIGNED_BYTE, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->accumulatebuffer_a );
        crtemu_pc->FramebufferTexture2D( CRTEMU_PC_GL_FRAMEBUFFER, CRTEMU_PC_GL_COLOR_ATTACHMENT0, CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_a, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_b );
        crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGB, width, height, 0, CRTEMU_PC_GL_RGB, CRTEMU_PC_GL_UNSIGNED_BYTE, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->accumulatebuffer_b );
        crtemu_pc->FramebufferTexture2D( CRTEMU_PC_GL_FRAMEBUFFER, CRTEMU_PC_GL_COLOR_ATTACHMENT0, CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_b, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_a );
        crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGB, width, height, 0, CRTEMU_PC_GL_RGB, CRTEMU_PC_GL_UNSIGNED_BYTE, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->blurbuffer_a );
        crtemu_pc->FramebufferTexture2D( CRTEMU_PC_GL_FRAMEBUFFER, CRTEMU_PC_GL_COLOR_ATTACHMENT0, CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_a, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

        crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_b );
        crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGB, width, height, 0, CRTEMU_PC_GL_RGB, CRTEMU_PC_GL_UNSIGNED_BYTE, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->blurbuffer_b );
        crtemu_pc->FramebufferTexture2D( CRTEMU_PC_GL_FRAMEBUFFER, CRTEMU_PC_GL_COLOR_ATTACHMENT0, CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_b, 0 );
        crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );
        }

    
    crtemu_pc->last_present_width = width;
    crtemu_pc->last_present_height = height;

    CRTEMU_PC_GLfloat vertices[] = 
        { 
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
        };
    crtemu_pc->BufferData( CRTEMU_PC_GL_ARRAY_BUFFER, 4 * 4 * sizeof( CRTEMU_PC_GLfloat ), vertices, CRTEMU_PC_GL_STATIC_DRAW );
    crtemu_pc->BindBuffer( CRTEMU_PC_GL_ARRAY_BUFFER, crtemu_pc->vertexbuffer );

    // Copy to backbuffer
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->backbuffer );
    crtemu_pc->TexImage2D( CRTEMU_PC_GL_TEXTURE_2D, 0, CRTEMU_PC_GL_RGBA, width, height, 0, CRTEMU_PC_GL_RGBA, CRTEMU_PC_GL_UNSIGNED_BYTE, pixels_xbgr ); 
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );

    crtemu_pc->Viewport( 0, 0, width, height );

    // Blur the previous accumulation buffer
    crtemu_pc_internal_blur( crtemu_pc, crtemu_pc->accumulatetexture_b, crtemu_pc->blurbuffer_a, crtemu_pc->blurbuffer_b, crtemu_pc->blurtexture_b, 1.0f, width, height );

    // Update accumulation buffer
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->accumulatebuffer_a );
    crtemu_pc->UseProgram( crtemu_pc->accumulate_shader );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->accumulate_shader, "tex0" ), 0 );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->accumulate_shader, "tex1" ), 1 );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->accumulate_shader, "modulate" ), 1.0f );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->backbuffer );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE1 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_a );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE1 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );
  
    
    // Store a copy of the accumulation buffer
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->accumulatebuffer_b );
    crtemu_pc->UseProgram( crtemu_pc->copy_shader );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->copy_shader, "tex0" ), 0 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_a );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

    // Blend accumulation and backbuffer
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, crtemu_pc->accumulatebuffer_a );
    crtemu_pc->UseProgram( crtemu_pc->blend_shader );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->blend_shader, "tex0" ), 0 );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->blend_shader, "tex1" ), 1 );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->blend_shader, "modulate" ), 1.0f );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->backbuffer );   
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE1 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_b );   
    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE1 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );


    // Add slight blur to backbuffer
    crtemu_pc_internal_blur( crtemu_pc, crtemu_pc->accumulatetexture_a, crtemu_pc->accumulatebuffer_a, crtemu_pc->blurbuffer_b, crtemu_pc->blurtexture_b, /*0.17f*/ 0.0f, width, height );

    // Create fully blurred version of backbuffer
    crtemu_pc_internal_blur( crtemu_pc, crtemu_pc->accumulatetexture_a, crtemu_pc->blurbuffer_a, crtemu_pc->blurbuffer_b, crtemu_pc->blurtexture_b, 1.0f, width, height );


    // Present to screen with CRT shader
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 );

    crtemu_pc->Viewport( viewport[ 0 ], viewport[ 1 ], viewport[ 2 ], viewport[ 3 ] );

    int window_width = viewport[ 2 ] - viewport[ 0 ];
    int window_height = viewport[ 3 ] - viewport[ 1 ];

    int aspect_width = (int)( ( window_height * 4.25f ) / 3 );
    int aspect_height= (int)( ( window_width * 3 ) / 4.25f );
    int target_width, target_height;
    if( aspect_height <= window_height ) 
        {
        target_width = window_width;
        target_height = aspect_height;
        } 
    else
        {
        target_width = aspect_width;
        target_height = window_height;
        }

    float hscale = target_width / (float) width;
    float vscale = target_height / (float) height;

    float hborder = ( window_width - hscale * width ) / 2.0f;
    float vborder = ( window_height - vscale * height ) / 2.0f;
    float x1 = hborder;
    float y1 = vborder;
    float x2 = x1 + hscale * width;
    float y2 = y1 + vscale * height;

    x1 = ( x1 / window_width ) * 2.0f - 1.0f;
    x2 = ( x2 / window_width ) * 2.0f - 1.0f;
    y1 = ( y1 / window_height ) * 2.0f - 1.0f;
    y2 = ( y2 / window_height ) * 2.0f - 1.0f;

    CRTEMU_PC_GLfloat screen_vertices[] = 
        { 
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        };
    screen_vertices[  0 ] = x1;
    screen_vertices[  1 ] = y1;
    screen_vertices[  4 ] = x2;
    screen_vertices[  5 ] = y1;
    screen_vertices[  8 ] = x2;
    screen_vertices[  9 ] = y2;
    screen_vertices[ 12 ] = x1;
    screen_vertices[ 13 ] = y2;

    crtemu_pc->BufferData( CRTEMU_PC_GL_ARRAY_BUFFER, 4 * 4 * sizeof( CRTEMU_PC_GLfloat ), screen_vertices, CRTEMU_PC_GL_STATIC_DRAW );
    crtemu_pc->BindBuffer( CRTEMU_PC_GL_ARRAY_BUFFER, crtemu_pc->vertexbuffer );

    float r = ( ( border_xbgr >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( border_xbgr >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( border_xbgr       ) & 0xff ) / 255.0f;
    crtemu_pc->ClearColor( r, g, b, 1.0f );
    crtemu_pc->Clear( CRTEMU_PC_GL_COLOR_BUFFER_BIT );

    crtemu_pc->UseProgram( crtemu_pc->crt_shader );
    
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "backbuffer" ), 0 );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "blurbuffer" ), 1 );
    crtemu_pc->Uniform1i( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "frametexture" ), 2 );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "use_frame" ), crtemu_pc->use_frame );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "time" ), 1.5f * (CRTEMU_PC_GLfloat)( ( (double) time_us ) / 1000000.0 ) );
    crtemu_pc->Uniform2f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "resolution" ), (float) window_width, (float) window_height );
    crtemu_pc->Uniform2f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "size" ), (float) target_width, (float) target_height );

    float mod_r = ( ( mod_xbgr >> 16 ) & 0xff ) / 255.0f;
    float mod_g = ( ( mod_xbgr >> 8  ) & 0xff ) / 255.0f;
    float mod_b = ( ( mod_xbgr       ) & 0xff ) / 255.0f;
    crtemu_pc->Uniform3f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "modulate" ), mod_r, mod_g, mod_b );

    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_curvature" ), crtemu_pc->config.curvature );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_scanlines" ), crtemu_pc->config.scanlines );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_shadow_mask" ), crtemu_pc->config.shadow_mask );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_separation" ), crtemu_pc->config.separation ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_ghosting" ), crtemu_pc->config.ghosting ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_noise" ), crtemu_pc->config.noise ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_flicker" ), crtemu_pc->config.flicker );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_vignette" ), crtemu_pc->config.vignette );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_distortion" ), crtemu_pc->config.distortion ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_aspect_lock" ), crtemu_pc->config.aspect_lock );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_hpos" ), crtemu_pc->config.hpos );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_vpos" ), crtemu_pc->config.vpos );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_hsize" ), crtemu_pc->config.hsize ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_vsize" ), crtemu_pc->config.vsize ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_contrast" ), crtemu_pc->config.contrast ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_brightness" ), crtemu_pc->config.brightness ); 
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_saturation" ), crtemu_pc->config.saturation );
    crtemu_pc->Uniform1f( crtemu_pc->GetUniformLocation( crtemu_pc->crt_shader, "cfg_degauss" ), crtemu_pc->config.degauss );

    float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->accumulatetexture_a );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    #ifndef CRTEMU_PC_WEBGL
        crtemu_pc->TexParameterfv( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_BORDER_COLOR, color );    
    #endif

    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE1 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->blurtexture_a );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    #ifndef CRTEMU_PC_WEBGL
        crtemu_pc->TexParameterfv( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_BORDER_COLOR, color );    
    #endif

    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE3 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, crtemu_pc->frametexture );   
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MIN_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_MAG_FILTER, CRTEMU_PC_GL_LINEAR );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_S, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    crtemu_pc->TexParameteri( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_WRAP_T, CRTEMU_PC_GL_CLAMP_TO_BORDER );
    #ifndef CRTEMU_PC_WEBGL
        crtemu_pc->TexParameterfv( CRTEMU_PC_GL_TEXTURE_2D, CRTEMU_PC_GL_TEXTURE_BORDER_COLOR, color );    
    #endif

    crtemu_pc->DrawArrays( CRTEMU_PC_GL_TRIANGLE_FAN, 0, 4 );    

    crtemu_pc->ActiveTexture( CRTEMU_PC_GL_TEXTURE0 );
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 0 );   
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 1 );   
    crtemu_pc->BindTexture( CRTEMU_PC_GL_TEXTURE_2D, 2 );   
    crtemu_pc->BindFramebuffer( CRTEMU_PC_GL_FRAMEBUFFER, 0 ); 
    }


void crtemu_pc_coordinates_window_to_bitmap( crtemu_pc_t* crtemu_pc, int width, int height, int* x, int* y )
    {
    CRTEMU_PC_GLint viewport[ 4 ];
    crtemu_pc->GetIntegerv( CRTEMU_PC_GL_VIEWPORT, viewport );

    int window_width = viewport[ 2 ] - viewport[ 0 ];
    int window_height = viewport[ 3 ] - viewport[ 1 ];

    int aspect_width = (int)( ( window_height * 4.25f ) / 3 );
    int aspect_height= (int)( ( window_width * 3 ) / 4.25f );
    int target_width, target_height;
    if( aspect_height <= window_height ) 
        {
        target_width = window_width;
        target_height = aspect_height;
        } 
    else
        {
        target_width = aspect_width;
        target_height = window_height;
        }

    float hscale = target_width / (float) width;
    float vscale = target_height / (float) height;

    float hborder = ( window_width - hscale * width ) / 2.0f;
    float vborder = ( window_height - vscale * height ) / 2.0f;
    
    float xp = ( ( *x - hborder ) / hscale ) / (float) width;
    float yp = ( ( *y - vborder ) / vscale ) / (float) height;

    /* TODO: Common params for shader and this */
    float xc = ( xp - 0.5f ) * 2.0f;
    float yc = ( yp - 0.5f ) * 2.0f;
    xc *= 1.1f; 
    yc *= 1.1f; 
    //xc *= 1.0f + powf( ( fabsf( yc ) / 5.0f ), 2.0f);
    //yc *= 1.0f + powf( ( fabsf( xc ) / 4.0f ), 2.0f);
    float yt = ( yc >= 0.0f ? yc : -yc ) / 5.0f;
    float xt = ( xc >= 0.0f ? xc : -xc ) / 4.0f;
    xc *= 1.0f + ( yt * yt );
    yc *= 1.0f + ( xt * xt );
    xc = ( xc / 2.0f ) + 0.5f;
    yc = ( yc / 2.0f ) + 0.5f;
    xc = xc * 0.92f + 0.04f;
    yc = yc * 0.92f + 0.04f;
    xp = xc * 0.2f + xp * 0.8f; 
    yp = yc * 0.2f + yp * 0.8f; 

    xp = xp * ( 1.0f - 0.04f ) + 0.04f / 2.0f + 0.003f;
    yp = yp * ( 1.0f - 0.04f ) + 0.04f / 2.0f - 0.001f;

    xp = xp * 1.035f - ( crtemu_pc->use_frame == 0.0f ? 0.018f : 0.0125f*0.75f );
    yp = yp * 0.96f + 0.02f;

    xp = xp * 1.2f - 0.1f;
    yp = yp * 1.2f - 0.1f;

    xp *= width;
    yp *= height;
        
    *x = (int) ( xp );
    *y = (int) ( yp );
    }


void crtemu_pc_coordinates_bitmap_to_window( crtemu_pc_t* crtemu_pc, int width, int height, int* x, int* y )
    {
    (void) crtemu_pc, (void) width, (void) height, (void) x, (void) y; // TODO: implement
    }


#endif /* CRTEMU_PC_IMPLEMENTATION */

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2016 Mattias Gustavsson

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
