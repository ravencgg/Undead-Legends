
#include "Render.h"
#include "SDL.h"
#include "SDL_syswm.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <string>

#ifndef UL_LINUX
#define UL_WINDOWS 1
#else
#define UL_WINDOWS (!UL_LINUX)
#endif

#if UL_LINUX
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#else
#pragma comment(lib, "opengl32.lib")
#include <Windows.h>
#include <gl/GL.h>
#endif

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_LINK_STATUS                    0x8B82
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_DEPTH_TEST                     0x0B71

#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB         0x8E8C // BC7
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB   0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB   0x8E8E // BC6H
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB 0x8E8F

#define GL_SRC1_COLOR                     0x88F9
#define GL_ONE_MINUS_SRC1_COLOR           0x88FA
#define GL_SRC1_ALPHA                     0x8589
#define GL_ONE_MINUS_SRC1_ALPHA           0x88FB

// BC5 support
#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_ERROR_INVALID_VERSION_ARB      0x2095
#define GL_ERROR_INVALID_PROFILE_ARB      0x2096

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126

// GL_TEXTURE1-31 are GL_TEXTURE0 + index;
#define GL_TEXTURE0                       0x84C0

#define GL_FALSE 0
#define GL_TRUE 1


#define REF(__V) ((void)__V)
#define countof(__arr) (sizeof(__arr) / sizeof(__arr[0]))
template <typename T>
T Min(const T& a, const T& b)
{
    return a <= b ? a : b;
}

template <typename T>
T Max(const T& a, const T& b)
{
    return a > b ? a : b;
}

template <typename T>
void Swap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

enum ColorChannel
{
    ColorChannel_All,

    ColorChannel_Red,
    ColorChannel_Green,
    ColorChannel_Blue,
    ColorChannel_Alpha,

    ColorChannel_Count,
};

enum ShaderType
{
    Shader_None,
    Shader_Sprite,
    Shader_Checkerboard,
    Shader_BitmapFont,
    Shader_SDFFont,
    Shader_Rectangle,

    Shader_Count
};

struct Vector2
{
    float x, y;
};

struct Vector3
{
    float x, y, z;
};

struct Vector4
{
    float x, y, z, w;
};

union Matrix4
{
    Vector4 c[4];
    float   m[4][4];
    float   e[16];
};


struct Vertex
{
    Vector2 position;
    Vector2 uv;
    Vector4 color;
};

enum GLExtension
{
    GLExtension_VSync,
    GLExtension_DebugOutput,
};

enum AddressMode
{
    AddressMode_Wrap,
    AddressMode_Clamp,

    AddressMode_Count
};

enum Sampler
{
    Sampler_Nearest,
    Sampler_Linear,

    Sampler_Count
};

enum BlendMode
{
    BlendMode_Overwrite,
    BlendMode_Blend,
    BlendMode_Additive,

    BlendMode_Count
};

enum DepthMode
{
    DepthMode_Ignored,
    DepthMode_ReadWrite,

    DepthMode_Count
};

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

#if !UL_LINUX
typedef u32 GLuint;
typedef s32 GLsizei;
typedef s32 GLint;
typedef u8  GLchar;
typedef s64 GLsizeiptr;
typedef s64 GLintptr;
static_assert(sizeof(void*) == 8, "Change the size of the preceding typedefs");

struct GLFunction
{
    const char* name;
    void**      address;
};
GLFunction  gl_function_names[100];
size_t      num_gl_function_names;

int RegisterGLFunction(const char* name, void** address)
{
    assert(num_gl_function_names < countof(gl_function_names));
    GLFunction* function = &gl_function_names[num_gl_function_names++];
    function->name = name;
    function->address = address;
    return 0;
}

#define GL_FUNCTION(ret, name, ...)                                 \
typedef ret (*fn_ ##name) (__VA_ARGS__);                            \
static fn_ ##name name;                                             \
static int dummy_ ##name = RegisterGLFunction(#name, (void**)&name)


typedef void (*GLDebugProcARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

//          Return      Name                        Arguments
GL_FUNCTION(GLuint,     glCreateShader,             GLuint type);
GL_FUNCTION(GLuint,     glShaderSource,             GLuint shader, GLsizei count, const char *const* string, const GLint *length);
GL_FUNCTION(void,       glCompileShader,            GLuint shader);
GL_FUNCTION(GLuint,     glCreateProgram,            void);
GL_FUNCTION(void,       glDeleteProgram,            GLuint program);
GL_FUNCTION(void,       glAttachShader,             GLuint program, GLuint shader);
GL_FUNCTION(void,       glLinkProgram,              GLuint program);
GL_FUNCTION(void,       glGetProgramiv,             GLuint program, GLenum pname, GLint *params);
GL_FUNCTION(void,       glGetProgramInfoLog,        GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_FUNCTION(void,       glDetachShader,             GLuint program, GLuint shader);
GL_FUNCTION(void,       glDeleteShader,             GLuint shader);
GL_FUNCTION(void,       glGenBuffers,               GLsizei n, GLuint *buffers);
GL_FUNCTION(void,       glGenVertexArrays,          GLsizei n, GLuint *arrays);
GL_FUNCTION(void,       glEnableVertexArrayAttrib,  GLuint vaobj, GLuint index);
GL_FUNCTION(void,       glBindBuffer,               GLenum target, GLuint buffer);
GL_FUNCTION(void,       glVertexAttribPointer,      GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_FUNCTION(void,       glBufferData,               GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_FUNCTION(void,       glBindVertexArray,          GLuint array);
GL_FUNCTION(void,       glGenFramebuffers,          GLsizei n, GLuint *framebuffers);
GL_FUNCTION(void,       glUseProgram,               GLuint program);
GL_FUNCTION(GLint,      glGetUniformLocation,       GLuint program, const char *name);
GL_FUNCTION(void,       glBufferSubData,            GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_FUNCTION(void,       glUniform1f,                GLint location, GLfloat v0);
GL_FUNCTION(void,       glUniform2f,                GLint location, GLfloat v0, GLfloat v1);
GL_FUNCTION(void,       glUniform3f,                GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_FUNCTION(void,       glUniform4f,                GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_FUNCTION(void,       glDeleteBuffers,            GLsizei n, const GLuint *buffers);
GL_FUNCTION(void,       glDeleteVertexArrays,       GLsizei n, const GLuint *arrays);
GL_FUNCTION(void,       glEnableVertexAttribArray,  GLuint index);
GL_FUNCTION(void,       glUniformMatrix4fv,         GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_FUNCTION(void,       glActiveTexture,            GLenum texture);
GL_FUNCTION(void,       glCompressedTexImage2D,     GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_FUNCTION(void,       glCompressedTexImage3D,     GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
GL_FUNCTION(const u8*,  glGetStringi,               GLenum name, GLuint index);
GL_FUNCTION(void,       glGetShaderInfoLog,         GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_FUNCTION(void,       glGetShaderiv,              GLuint shader, GLenum pname, GLint *params);
GL_FUNCTION(void,       glUniform1i,                GLint location, GLint v0);
GL_FUNCTION(void,       glTexSubImage3D,            GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
GL_FUNCTION(void,       glCompressedTexSubImage2D,  GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_FUNCTION(void,       glCompressedTexSubImage3D,  GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);
GL_FUNCTION(void,       glTexStorage2D,             GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_FUNCTION(void,       glTexStorage3D,             GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GL_FUNCTION(void,       glDrawElementsBaseVertex,   GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

GL_FUNCTION(void,       glDebugMessageCallbackARB,  GLDebugProcARB callback, const void* userParam);

GL_FUNCTION(HGLRC,      wglCreateContextAttribsARB, HDC hDC, HGLRC hShareContext, const int *attribList);
GL_FUNCTION(BOOL,       wglSwapIntervalEXT,         int interval);
GL_FUNCTION(int,        wglGetSwapIntervalEXT,      void);

#if UL_LINUX
static void* LoadGLFunctionPointer(const char *name)
{
    void *result = nullptr;
    result = (void *)SDL_GL_GetProcAddress(name);
}
#else
static HMODULE opengl32_module;
static void* LoadGLFunctionPointer(const char *name)
{
    void *result = nullptr;
    result = (void *)wglGetProcAddress(name);
    if (result == 0 || (result == (void*)0x1) || (result == (void*)0x2) || (result == (void*)0x3) || (result == (void*)-1))
    {
        if (opengl32_module == 0)
            opengl32_module = LoadLibraryA("opengl32.dll");
        if (!opengl32_module)
            return nullptr;
        result = (void *)GetProcAddress(opengl32_module, name);
    }
    return result;
}
#endif

bool LoadGLFunctionPointers()
{
    assert(*gl_function_names[0].address == nullptr);

    for (size_t i = 0; i < num_gl_function_names; ++i)
    {
        GLFunction* function = gl_function_names + i;
        *function->address = LoadGLFunctionPointer(function->name);
        if (*function->address == nullptr)
            return false;
    }
    return true;
}
#endif

#define _STRING(v) #v
#define STRING(v) _STRING(v)

static const char* s_vert_shader = R"TERM(

#line )TERM" STRING(__LINE__) R"TERM(
layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec4 v_color;

out vec2 f_position;
out vec2 f_uv;
out vec4 f_color;

uniform mat4 u_ortho_mat;

void main()
{
    vec4 pos = u_ortho_mat * vec4(v_position.xy, 0, 1);
    gl_Position = pos;
    f_position = pos.xy;
    f_color = v_color;
    f_uv = v_uv;
}

)TERM";

static const char* s_frag_shader = R"TERM(

#line )TERM" STRING(__LINE__) R"TERM(
in vec2 f_position;
in vec2 f_uv;
in vec4 f_color;

out vec4 color;

uniform sampler2D diffuse;

void main()
{
    color = texture(diffuse, f_uv) * f_color;
}

)TERM";

static const char* s_frag_font_shader = R"TERM(

#line )TERM" STRING(__LINE__) R"TERM(
in vec2 f_uv;
in vec4 f_color;
in vec2 f_position;

layout (location = 0, index = 0) out vec4 out_color;

uniform sampler2D diffuse;
uniform vec2 u_screen_size;

float SampleSDF(sampler2D sampler, vec2 uv)
{
// http://metalbyexample.com/rendering-text-in-metal-with-signed-distance-fields/
// This version uses dFdx, dFdy to prevent blurriness when zooming in on a font
// by only ever having one pixel wide antialiasing.

    float edge = 128.0 / 255.0;// 180.0 / 255.0; // I believe that this needs to be matched to the sdf_onedge_value from glass_font.cpp
    float d = texture(sampler, uv).r;
    float distance = 0.7 * length(vec2(dFdx(d), dFdy(d)));
    float alpha = smoothstep(edge - distance, edge + distance, d);
    return alpha;
}

void main()
{
#if BITMAP_FONT
    float d = texture(diffuse, f_uv).r;
    out_color = d * f_color;
#elif SDF_FONT
    out_color = f_color;

    float sdf_value = SampleSDF(diffuse, f_uv);

    vec2 shadow_uv = f_uv;
    vec2 size = textureSize(diffuse, 0);
    shadow_uv.x -= 2.0 / size.x;
    shadow_uv.y -= 2.0 / size.y;
    float shadow_power = min(SampleSDF(diffuse, shadow_uv), f_color.a);

    vec3 shadow_color = vec3(0);

    out_color.rgb = mix(shadow_color, out_color.rgb, sdf_value);
    sdf_value = max(shadow_power, sdf_value);

    out_color.a *= sdf_value;
#else
#error Unkown font shader type!
#endif
}

)TERM";

static const char* s_frag_checkerboard_shader = R"TERM(
#line )TERM" STRING(__LINE__) R"TERM(
in vec2 f_uv;
in vec4 f_color;
in vec2 f_position;

out vec4 color;
uniform vec2 u_screen_size;

void main()
{
    const float size = 50.0f;
    vec2 pos = mod(f_position * u_screen_size, size * 2.0);

    if ((pos.x < size && pos.y < size) || (pos.x > size && pos.y > size))
        color = vec4(1);
    else
        color = vec4(vec3(0.75f), 1);
}

)TERM";

static const char* s_frag_rectangle_shader = R"TERM(
#line )TERM" STRING(__LINE__) R"TERM(
in vec2 f_uv;
in vec4 f_color;
in vec2 f_position;

out vec4 color;
uniform vec2 u_screen_size;

uniform vec4  u_interior_color;
uniform vec4  u_border_color;
uniform vec2  u_rect_pos;  // center of the rectangle in pixels
uniform vec2  u_rect_size; // size of the rectangle in pixels
uniform float u_line_width;
uniform float u_rounding;

// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float rounded_box_sdf(vec2 center, vec2 size, vec4 radius)
{
    radius.xy = (center.x > 0.0) ? radius.xy : radius.zw;
    radius.x  = (center.y > 0.0) ? radius.x  : radius.y;

    vec2 q = abs(center) - size + vec2(radius.x);
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0))) - radius.x;
}

void main()
{
    // The pixel space scale of the rectangle.
    vec2 bottom_left = u_rect_pos - u_rect_size * 0.5;
    vec2 top_right = bottom_left + u_rect_size;
    vec2 size = (top_right - bottom_left);
    size += u_line_width * 2.0;

    // The pixel space location of the rectangle.
    vec2 location = u_rect_pos;

    // How soft the edges should be (in pixels). Higher values could be used to simulate a drop shadow.
    float edge_softness  = 2.0f;

    // The radius of the corners (in pixels) clockwise starting in the top left.
    //vec4 radius = vec4(1.0f, 10.0f, 40.0f, 80.0f);
    vec4 radius = vec4(u_rounding);

    // Calculate distance to edge.
    vec2 pixel_pos = (f_position + vec2(1.0)) * (u_screen_size / 2.0);
    float dist = rounded_box_sdf(pixel_pos - location, size / 2.0f, radius);

    // Smooth the result (free antialiasing).
    float alpha =  1.0f - smoothstep(0.0f, edge_softness, dist);

    float border_thickness = u_line_width;
    float border_softness  = 2.0f;
    float border_alpha = 1.0f - smoothstep(border_thickness - border_softness, border_thickness, abs(dist));

    vec4 exterior_color = vec4(0.0);
    color = mix(exterior_color, mix(u_interior_color, u_border_color, border_alpha), alpha);
}


)TERM";

static const char* s_frag_solid_shader = R"TERM(

#line )TERM" STRING(__LINE__) R"TERM(
in vec2 f_uv;
in vec4 f_color;
in vec2 f_position;

out vec4 color;

void main()
{
    color = f_color;
}

)TERM";

static void SetDepthMode(DepthMode mode);

struct glShaderConstants
{
    Matrix4 c_mvp;
    Vector2 c_screen_size;

    // Rectangle drawing stuff
    Vector4  cr_interior_color;
    Vector4  cr_border_color;
    Vector2  cr_rect_pos;  // center of the rectangle in pixels
    Vector2  cr_rect_size; // size of the rectangle in pixels
    float    cr_line_width;
    float    cr_rounding;
};


struct Shader
{
    u32 handle;

    s32 u_ortho_loc;
    s32 u_screen_size_loc;

    // Rectangles:
    s32 u_interior_color;
    s32 u_border_color;
    s32 u_rect_pos;
    s32 u_rect_size;
    s32 u_line_width;
    s32 u_rounding;
};

struct GPUBuffer
{
    u32 gl_target;
    u32 handle;
    s32 gpu_size;
    void* memory;
    s32 size;
    s32 used;
};

struct R_Texture
{
    u32 handle;
    u32 target;

    u32 width;
    u32 height;
    R_PixelFormat format;

    Vector4     mod_color;
    BlendMode   blend_mode;

    // Uploading
    SDL_Rect      locked_region;
    void*         pixels;
};

struct OpenGL
{
    void*       gl_context;
    void*       dc;
    SDL_Window* window;

    u32         the_vao;

    // The texture that handles the rectangle fill operations.
    R_Texture*  white_texture;

    ShaderType  active_shader;
    Shader      shaders[Shader_Count];

    GPUBuffer   vertex_buffer;
    GPUBuffer   index_buffer;

    // The size of the draw region for the current frame
    s32         frame_width;
    s32         frame_height;

    u32         supported_extensions;

    std::vector<R_Texture*> defer_free_textures;

    glShaderConstants constants;

// Transient:
    bool        post_present;
    Matrix4     current_ortho_matrix;
    Vector4     current_render_color;
};
OpenGL* s_gl;

static void ReleaseDeferredResources()
{
    for (R_Texture* texture : s_gl->defer_free_textures)
    {
        free(texture->pixels);
        texture->pixels = nullptr;
        glDeleteTextures(1, &texture->handle);
        delete texture;
    }
    s_gl->defer_free_textures.clear();
}

static const char* GLErrorString(s32 code)
{
    switch(code)
    {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    //case GL_ERROR_INVALID_VERSION_ARB: return "GL_ERROR_INVALID_VERSION_ARB";
    //case GL_ERROR_INVALID_PROFILE_ARB: return "GL_ERROR_INVALID_PROFILE_ARB";
    }
    return "Unknown";
}

static bool _CheckGLError(const char* file, int line)
{
    s32 error = 0;
    bool found_error = false;
    while ((error = glGetError()) != 0)
    {
        fprintf(stderr, "%s(%d): GLError %d: %s\n", file, line, error, GLErrorString(error));
        found_error = true;
    }
    return found_error;
}

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

struct ShaderDefine
{
    const char* name;
    const char* value;
};

static GLuint CompileShader(const char* shader_code, GLenum shader_type, ShaderDefine* defines, u32 num_defines)
{
    const char* shader_sources[18] = {};
    u32 num_sources = 0;
    char define_string[16][256];
    char version_string[256];
    snprintf(version_string, countof(version_string), "#version 410\n");
    shader_sources[num_sources++] = version_string;

    for (u32 i = 0; i < num_defines; ++i)
    {
        //sprintf(define_string[i], sizeof(define_string[i]), "#define %s %s");
        snprintf(define_string[i], countof(define_string[i]), "#define %s %s\n", defines[i].name, defines[i].value);
        shader_sources[num_sources++] = define_string[i];
    }
    shader_sources[num_sources++] = shader_code;

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, num_sources, shader_sources, 0);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar info[4096];
        GLsizei len;
        glGetShaderInfoLog(shader, Min(log_length, (GLint)countof(info)), &len, info);
        fprintf(stderr, "Shader compilation error: %s\n", info);
    }

    return shader;
}

static u32 LoadShaderWithDefines(const char* vert_shader, const char* frag_shader, ShaderDefine* vert_defines, u32 num_vert_defines, ShaderDefine* frag_defines, u32 num_frag_defines)
{
    GLuint vert_loc = CompileShader(vert_shader, GL_VERTEX_SHADER, vert_defines, num_vert_defines);
    GLuint frag_loc = CompileShader(frag_shader, GL_FRAGMENT_SHADER, frag_defines, num_frag_defines);

    GLuint program = glCreateProgram();

    glAttachShader(program, vert_loc);
    glAttachShader(program, frag_loc);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        GLchar info[4096];
        info[0] = 0;
        GLsizei len;
        glGetProgramInfoLog(program, countof(info), &len, info);
        fprintf(stderr, "Shader linking error %d: %s\n", (int)len, info);
    }

    glDetachShader(program, vert_loc);
    glDetachShader(program, frag_loc);
    glDeleteShader(vert_loc);
    glDeleteShader(frag_loc);

    return program;
}

u32 LoadShader(const char* vert_shader, const char* frag_shader)
{
    return LoadShaderWithDefines(vert_shader, frag_shader, 0, 0, 0, 0);
}

static void LoadShaderUniforms(Shader* shader)
{
    if (shader && shader->handle)
    {
        shader->u_ortho_loc         = glGetUniformLocation(shader->handle, "u_ortho_mat");
        shader->u_screen_size_loc   = glGetUniformLocation(shader->handle, "u_screen_size");
        shader->u_interior_color    = glGetUniformLocation(shader->handle, "u_interior_color");
        shader->u_border_color      = glGetUniformLocation(shader->handle, "u_border_color");
        shader->u_rect_pos          = glGetUniformLocation(shader->handle, "u_rect_pos");
        shader->u_rect_size         = glGetUniformLocation(shader->handle, "u_rect_size");
        shader->u_line_width        = glGetUniformLocation(shader->handle, "u_line_width");
        shader->u_rounding          = glGetUniformLocation(shader->handle, "u_rounding");
    }
}

void LoadSpriteShader()
{
    Shader* shader = &s_gl->shaders[Shader_Sprite];
    shader->handle = LoadShader(s_vert_shader, s_frag_shader);
    LoadShaderUniforms(shader);
}

void LoadCheckerboardShader()
{
    Shader* shader = &s_gl->shaders[Shader_Checkerboard];
    shader->handle = LoadShader(s_vert_shader, s_frag_checkerboard_shader);
    LoadShaderUniforms(shader);
}

void LoadRectangleShader()
{
    Shader* shader = &s_gl->shaders[Shader_Rectangle];
    shader->handle = LoadShader(s_vert_shader, s_frag_rectangle_shader);
    LoadShaderUniforms(shader);
}

void LoadFontShader()
{

    {
        Shader* shader = &s_gl->shaders[Shader_SDFFont];
        ShaderDefine defines[] = {
            { "BITMAP_FONT",   "0" },
            { "SDF_FONT",      "1" },
        };
        shader->handle = LoadShaderWithDefines(s_vert_shader, s_frag_font_shader, 0, 0, defines, countof(defines));
        LoadShaderUniforms(shader);
    }

    {
        Shader* shader = &s_gl->shaders[Shader_BitmapFont];
        ShaderDefine defines[] = {
            { "BITMAP_FONT",   "1" },
            { "SDF_FONT",      "0" },
        };
        shader->handle = LoadShaderWithDefines(s_vert_shader, s_frag_font_shader, 0, 0, defines, countof(defines));
        LoadShaderUniforms(shader);
    }

}

static void CheckForExtension(const std::vector<std::string>& extensions, GLExtension extension, const std::string& extension_string)
{
    for (auto str : extensions)
    {
        if (str == extension_string)
        {
            s_gl->supported_extensions |= (1 << extension);
            break;
        }
    }
}

static bool ExtensionIsSupported(GLExtension extension)
{
    return !!(s_gl->supported_extensions & (1 << extension));
}

static bool PixelFormatToGLFormats(R_PixelFormat format, GLenum* gl_format, GLenum* gl_internal_format)
{
    *gl_format = format == R_PixelFormat_R ? GL_RED : GL_RGBA;
    switch (format)
    {
    case R_PixelFormat_RGBA: *gl_internal_format = GL_RGBA8; break;
    case R_PixelFormat_R:    *gl_internal_format = GL_RED;   break;

    default:
        assert(false);
        return false;
    }

    return true;
}

static int PixelSize(R_PixelFormat format)
{
    int result = 1;

    switch(format)
    {
    case R_PixelFormat_RGBA: 
        result = 4;
        break;
    case R_PixelFormat_R:
        result = 1;
        break;

    case R_PixelFormat_Invalid: break;
    default:
        assert(false);
    }
    return result;
}

static void GLDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    REF(source);
    REF(type);
    REF(id);
    REF(severity);
    REF(length);
    REF(userParam);
    fprintf(stderr, "Debug message: %s\n", message);
}

void* R_CreateRenderer(SDL_Window* window)
{
    assert(s_gl == nullptr);
    OpenGL* result = new OpenGL();
    s_gl = result;
    result->window = window;
    result->current_render_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    result->post_present = true;

#if UL_WINDOWS
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
        32,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24, //24,                  //Number of bits for the depthbuffer
        0,                        //Number of bits for the stencilbuffer
        0,                        //Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    HDC dc = GetDC(hwnd);
    result->dc = dc;
    int pixel_format = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pixel_format, &pfd);

    HGLRC temp_context = wglCreateContext(dc);
    wglMakeCurrent(dc, temp_context);

    if (!LoadGLFunctionPointers())
    {
        fprintf(stderr, "Failed to load gl function pointers\n");
    }

    int attrib_list[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
#if _DEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, //WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0, 0
    };

    result->gl_context = wglCreateContextAttribsARB(dc, 0, attrib_list);
    wglMakeCurrent(dc, (HGLRC)result->gl_context);
    wglDeleteContext(temp_context);

    glDebugMessageCallbackARB(GLDebugOutput, nullptr);
#else
    result->gl_context = SDL_GL_CreateContext(window);
#endif

    result->vertex_buffer = {};
    result->index_buffer = {};

    {
        // LoadShaders
        LoadSpriteShader();
        LoadCheckerboardShader();
        LoadFontShader();
        LoadRectangleShader();
    }

    {
        u32 default_color = 0xFFFFFFFF;
        s_gl->white_texture = R_CreateTexture(R_PixelFormat_RGBA, 1, 1, 1);

        void* pixel;
        int pitch;
        if (R_LockTexture(s_gl->white_texture, 0, &pixel, &pitch))
        {
            memcpy(pixel, &default_color, sizeof(default_color));
            R_UnlockTexture(s_gl->white_texture);
        }
    }

    {
        u32 vao;
        glGenVertexArrays(1, &vao);
        result->the_vao = vao;
        // Always bound with 3 enabled vertex arrays for the Vertex type
        glBindVertexArray(vao);
        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);
        glEnableVertexArrayAttrib(vao, 2);
    }

    glEnable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);

    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    s32 num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    std::vector<std::string> extensions;
    extensions.reserve(num_extensions);
    for (s32 i = 0; i < num_extensions; ++i)
    {
        extensions.push_back(std::string((const char*)glGetStringi(GL_EXTENSIONS, i)));
    }
    CheckForExtension(extensions, GLExtension_VSync, "WGL_EXT_swap_control");
    CheckForExtension(extensions, GLExtension_DebugOutput, "GL_ARB_debug_output");

    assert(ExtensionIsSupported(GLExtension_DebugOutput));

#if UL_WINDOWS
    if (ExtensionIsSupported(GLExtension_VSync))
    {
        wglSwapIntervalEXT(1);
    }
#endif

    SetDepthMode(DepthMode_Ignored);
    return result;
}

void R_DestroyRenderer()
{
    if (s_gl)
    {
#if UL_WINDOWS
        wglMakeCurrent((HDC)s_gl->dc, NULL);
        wglDeleteContext((HGLRC)s_gl->gl_context);
#endif
        free(s_gl->vertex_buffer.memory);
        free(s_gl->index_buffer.memory);

        R_DestroyTexture(s_gl->white_texture);

        ReleaseDeferredResources();
        s_gl->defer_free_textures.clear();

        delete s_gl;
        s_gl = nullptr;
    }
}

R_Texture* R_CreateTexture(R_PixelFormat format, int access, int width, int height)
{
    REF(access);
    GLenum gl_format, internal_format;
    if (!PixelFormatToGLFormats(format, &gl_format, &internal_format))
    {
        return nullptr;
    }

    R_Texture* result = new R_Texture();
    result->width = width;
    result->height = height;
    result->target = GL_TEXTURE_2D;
    result->format = format;
    result->mod_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    result->blend_mode = BlendMode_Overwrite;
    glGenTextures(1, &result->handle);
    glBindTexture(result->target, result->handle);
    glTexImage2D(result->target, 0, internal_format, width, height, 0, gl_format, GL_UNSIGNED_BYTE, nullptr);
    return result;
}

void R_DestroyTexture(R_Texture* texture)
{
    if (!texture)
    {
        return;
    }

    s_gl->defer_free_textures.push_back(texture);
}

bool R_LockTexture(R_Texture* texture, SDL_Rect* region, void** pixels, int* pitch)
{
    int size = texture->width * texture->height * PixelSize(texture->format);
    assert(texture->pixels == nullptr);
    if (texture->pixels)
    {
        return false;
    }
    texture->pixels = malloc(size);

    if (region == nullptr)
    {
        texture->locked_region.x = 0;
        texture->locked_region.y = 0;
        texture->locked_region.w = texture->width;
        texture->locked_region.h = texture->height;
    }
    else
    {
        texture->locked_region = *region;
    }

    *pixels = texture->pixels;
    if (pitch)
    {
        *pitch = texture->width * PixelSize(texture->format);
    }

    return true;
}

void R_UnlockTexture(R_Texture* texture)
{
    if (!texture->pixels)
    {
        return;
    }

    GLenum format, internal_format;
    if (PixelFormatToGLFormats(texture->format, &format, &internal_format))
    {
        SDL_Rect r = texture->locked_region;

        glBindTexture(texture->target, texture->handle);
        glTexSubImage2D(texture->target, 0, r.x, r.y, r.w, r.h, format, GL_UNSIGNED_BYTE, texture->pixels);
        CheckGLError();
    }
    else
    {
        fprintf(stderr, "Tried to create a texture, but failed to determine pixel format\n");
    }

    free(texture->pixels);
    texture->pixels = nullptr;
}

void R_QueryTexture(R_Texture* texture, uint32_t* format, int* access, int* width, int* height)
{
    if (!texture) return;
    if (format) *format = texture->format;
    if (access) *access = 1;
    if (width)  *width  = texture->width;
    if (height) *height = texture->height;
}

void R_SetTextureColorMod(R_Texture* texture, u8 red, u8 green, u8 blue)
{
    texture->mod_color.x = red   / 255.0f;
    texture->mod_color.y = green / 255.0f;
    texture->mod_color.z = blue  / 255.0f;
    texture->mod_color.w = 1.0f;
}

void R_SetTextureBlendMode(R_Texture* texture, SDL_BlendMode mode)
{
    switch(mode)
    {
    case SDL_BLENDMODE_NONE:
        texture->blend_mode = BlendMode_Overwrite;
        break;
    case SDL_BLENDMODE_BLEND:
        texture->blend_mode = BlendMode_Blend;
        break;
    case SDL_BLENDMODE_ADD:
        texture->blend_mode = BlendMode_Additive;
        break;
    default:
        assert(false);
    }
}

static void SetDepthMode(DepthMode mode)
{
    switch (mode)
    {
    case DepthMode_Ignored:
        glDisable(GL_DEPTH_TEST);
        break;
    case DepthMode_ReadWrite:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        break;
    default:
        assert(0);
    }
}

static void SetBlendMode(BlendMode mode)
{
    switch (mode)
    {
    case BlendMode_Overwrite:
        glBlendFunc(GL_ONE, GL_ZERO);
        break;
    case BlendMode_Blend:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case BlendMode_Additive:
        glBlendFunc(GL_ONE, GL_ONE);
        break;
    default:
        assert(false);
    }
}

static void SetSampler(u32 target, Sampler sampler)
{
    GLenum gl_samplers[] = {
        GL_NEAREST, // Sampler_Nearest,
        GL_LINEAR,  // Sampler_Linear,
    };
    static_assert(countof(gl_samplers) == Sampler_Count, "");

    GLenum* min_samplers = gl_samplers;
    GLenum* mag_samplers = gl_samplers;

    assert(sampler >= 0 && sampler < Sampler_Count);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_samplers[sampler]);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_samplers[sampler]);
    CheckGLError();
}

static void SetAddressMode(u32 target, AddressMode mode)
{
    GLenum gl_address_modes[] = {
        GL_REPEAT,        // AddressMode_Wrap,
        GL_CLAMP_TO_EDGE, // AddressMode_Clamp,
    };
    static_assert(countof(gl_address_modes) == AddressMode_Count, "");
    assert(mode >= 0 && mode < AddressMode_Count);

    CheckGLError();
    // Requires that the texture is bound!
    glTexParameteri(target, GL_TEXTURE_WRAP_S, gl_address_modes[mode]);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, gl_address_modes[mode]);
    CheckGLError();
}

static void SetShader(ShaderType shader)
{
    OpenGL* gl = s_gl;
    if (gl->active_shader == shader)
        return;

    gl->active_shader = shader;
    glUseProgram(gl->shaders[shader].handle);
}

static Shader* GetActiveShader()
{
    return &s_gl->shaders[s_gl->active_shader];
}

static void MatrixIdentity(Matrix4* m)
{
    m->c[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
    m->c[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
    m->c[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
    m->c[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
}

static void MakeOrthoMatrix(Matrix4* m, float left, float right, float bottom, float top)
{
	MatrixIdentity(m);
	m->m[0][0] = 2.0f / (right - left);
	m->m[1][1] = 2.0f / (top - bottom);
	m->m[2][2] = -1.0f;
	m->m[3][0] = -(right + left) / (right - left);
	m->m[3][1] = -(top + bottom) / (top - bottom);
}

static void SetDefaultMatrix()
{
    if (s_gl->active_shader)
    {
        Matrix4 ortho_mat;
        MakeOrthoMatrix(&ortho_mat, 0, (float)s_gl->frame_width, (float)s_gl->frame_height, 0);

        Shader* shader = GetActiveShader();
        glUniformMatrix4fv(shader->u_ortho_loc, 1, GL_FALSE, ortho_mat.e);
    }
}

static void ClearBuffer(GPUBuffer* buffer)
{
    buffer->used = 0;
}

// NOTE: using R_RenderClear as a BeginFrame function, which won't be correct when there
// are multiple render targets
void R_RenderClear()
{
    if (!s_gl || !s_gl->window)
        return;

    OpenGL* gl = s_gl;
    if (gl->post_present)
    {
        // Do this on the first clear of a frame. Not correct overall, but correct
        // while transitioning APIs from SDL to opengl
        gl->post_present = false;

        SDL_Window* window = gl->window;
        SDL_GL_GetDrawableSize(window, &gl->frame_width, &gl->frame_height);

        MakeOrthoMatrix(&gl->constants.c_mvp, 0, (float)gl->frame_width, (float)gl->frame_height, 0);
        s32 width = gl->frame_width;
        s32 height = gl->frame_height;
        gl->constants.c_screen_size.x = float(width);
        gl->constants.c_screen_size.y = float(height);
        glViewport(0, 0, width, height);

        for (u32 shader_index = 0; shader_index < Shader_Count; ++shader_index)
        {
            SetShader(ShaderType(shader_index));
            SetDefaultMatrix();
        }

        ClearBuffer(&gl->vertex_buffer);
        ClearBuffer(&gl->index_buffer);
    }

    Vector4 clear_color = gl->current_render_color;
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void R_RenderPresent()
{
    s_gl->post_present = true;

#if UL_WINDOWS
    SwapBuffers((HDC)s_gl->dc);
#else
    SDL_GL_SwapWindow(s_gl->window);
#endif
    ReleaseDeferredResources();
}

void R_SetRenderDrawColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    s_gl->current_render_color.x = red / 255.0f;
    s_gl->current_render_color.y = green / 255.0f;
    s_gl->current_render_color.z = blue / 255.0f;
    s_gl->current_render_color.w = alpha / 255.0f;
}

void R_SetRenderDrawBlendMode(SDL_BlendMode mode)
{
    R_SetTextureBlendMode(s_gl->white_texture, mode);
}

void R_RenderCopy(R_Texture* texture, SDL_Rect* source, SDL_Rect* dest)
{
    R_RenderCopyEx(texture, source, dest, 0, NULL, SDL_FLIP_NONE);
}

static void BindBuffer(GPUBuffer* buffer)
{
    if (buffer->memory == 0)
    {
        assert(false);
        return;
    }

    if (buffer->handle == 0)
    {
        glGenBuffers(1, &buffer->handle);
    }

    glBindBuffer(buffer->gl_target, buffer->handle);
    if (buffer->gpu_size < buffer->size)
    {
        buffer->gpu_size = buffer->size;
        glBufferData(buffer->gl_target, buffer->gpu_size, 0, GL_DYNAMIC_DRAW);
    }

    glBufferSubData(buffer->gl_target, 0, buffer->used, buffer->memory);

    if (buffer->gl_target == GL_ARRAY_BUFFER)
    {
        glVertexAttribPointer(0, sizeof(Vertex::position) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glVertexAttribPointer(1, sizeof(Vertex::uv)       / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glVertexAttribPointer(2, sizeof(Vertex::color)    / sizeof(float), GL_FLOAT, GL_TRUE,  sizeof(Vertex), (void*)offsetof(Vertex, color));
    }
}

// TODO: Not this:
static void PushVertices(Vertex* vertices, u32 count)
{
    GPUBuffer* buffer = &s_gl->vertex_buffer;
    assert(buffer->gl_target != GL_ELEMENT_ARRAY_BUFFER);
    buffer->gl_target = GL_ARRAY_BUFFER;

    if (buffer->memory == 0)
    {
        buffer->size = 256;
        buffer->memory = malloc(buffer->size);
        buffer->used = 0;
    }

    u32 alloc_size = sizeof(Vertex) * count;
    buffer->used = 0;
    s32 new_used = buffer->used + alloc_size;

    while (new_used > buffer->size)
    {
        buffer->size *= 2;
        fprintf(stderr, "Resizing buffer to %d\n", buffer->size);
        buffer->memory = realloc(buffer->memory, buffer->size);
    }

    Vertex* result = (Vertex*)((u8*)buffer->memory + buffer->used);
    buffer->used = new_used;
    memcpy(result, vertices, sizeof(Vertex) * count);
    BindBuffer(buffer);
    ClearBuffer(buffer);
}

#if 0
static void PushIndices(u16* indices, u32 count)
{
    GPUBuffer* buffer = &s_gl->index_buffer;
    assert(buffer->gl_target != GL_ARRAY_BUFFER);
    buffer->gl_target = GL_ELEMENT_ARRAY_BUFFER;

    if (buffer->memory == 0)
    {
        buffer->size = 1024 * 1024 * 1;
        buffer->memory = malloc(buffer->size);
        buffer->used = 0;
    }

    s32 alloc_size = sizeof(u16) * count;
    s32 new_used = buffer->used + alloc_size;

    while (new_used > buffer->size)
    {
        buffer->size *= 2;
        buffer->memory = realloc(buffer->memory, buffer->size);
    }

    u16* result = (u16*)((u8*)buffer->memory + buffer->used);
    buffer->used = new_used;
    memcpy(result, indices, sizeof(indices[0]) * count);
    BindBuffer(buffer);
    ClearBuffer(buffer);
}
#endif

static void SetUniform(s32 location, Matrix4 value)
{
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
    }
}

#if 0
static void SetUniform(s32 location, s32 value)
{
    if (location != -1)
    {
        glUniform1i(location, value);
    }
}
#endif

static void SetUniform(s32 location, float value)
{
    if (location != -1)
    {
        glUniform1f(location, value);
    }
}

static void SetUniform(s32 location, Vector2 value)
{
    if (location != -1)
    {
        glUniform2f(location, value.x, value.y);
    }
}

static void SetUniform(s32 location, Vector4 value)
{
    if (location != -1)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}


static void UpdateShaderUniforms()
{
    Shader* shader = GetActiveShader();

    SetUniform(shader->u_ortho_loc,         s_gl->constants.c_mvp);
    SetUniform(shader->u_screen_size_loc,   s_gl->constants.c_screen_size);

    SetUniform(shader->u_interior_color,    s_gl->constants.cr_interior_color);
    SetUniform(shader->u_border_color,      s_gl->constants.cr_border_color);
    SetUniform(shader->u_rect_pos,          s_gl->constants.cr_rect_pos);
    SetUniform(shader->u_rect_size,         s_gl->constants.cr_rect_size);
    SetUniform(shader->u_line_width,        s_gl->constants.cr_line_width);
    SetUniform(shader->u_rounding,          s_gl->constants.cr_rounding);
}

static void RenderImpl(R_Texture* texture, SDL_Rect* source, SDL_Rect* dest, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    s32 width = texture ? texture->width : 1;
    s32 height = texture ? texture->height : 1;
    SDL_Rect s, d;
    if (source)
    {
        s = *source;
    }
    else
    {
        s.x = 0;
        s.y = 0;
        s.w = width;
        s.h = height;
    }

    if (dest)
    {
        d = *dest;
    }
    else
    {
        d.x = 0;
        d.y = 0;
        d.w = s_gl->frame_width;
        d.h = s_gl->frame_height;
    }

    Vector2 uv_min = { (float)s.x / width, (float)s.y  / height };
    Vector2 uv_max = uv_min;
    uv_max.x += (float)s.w / width;
    uv_max.y += (float)s.h / height;

    //   0     1
    //
    //
    //   2     3

    Vector2 positions[4] = {
        { (float)d.x,       (float)d.y       },
        { (float)d.x + d.w, (float)d.y       },
        { (float)d.x,       (float)d.y + d.h },
        { (float)d.x + d.w, (float)d.y + d.h },
    };
    if (angle != 0)
    {
        SDL_Point c;
        if (center)
        {
            c = *center;
        }
        else
        {
            c = { d.w / 2, d.h / 2 };
        }

        Vector2 rot = { d.x + (float)c.x, d.y + (float)c.y };
        float cos_theta = cosf((float)angle / 180 * 3.14159f);
        float sin_theta = sinf((float)angle / 180 * 3.14159f);
        for (Vector2& p : positions)
        {
            p.x -= rot.x;
            p.y -= rot.y;

            float new_x = p.x * cos_theta + p.y * -sin_theta;
            float new_y = p.x * sin_theta + p.y * cos_theta;
            p.x = new_x;
            p.y = new_y;

            p.x += rot.x;
            p.y += rot.y;
        }
    }

    if (flip & SDL_FLIP_HORIZONTAL)
    {
        Swap(uv_min.x, uv_max.x);
    }
    if (flip & SDL_FLIP_VERTICAL)
    {
        Swap(uv_min.y, uv_max.y);
    }

    R_Texture* t = texture;
    Vector4 color;
    if (t)
    {
        color = t->mod_color;
    }
    else
    {
        t = s_gl->white_texture;
        color = s_gl->current_render_color;
    }
    glBindTexture(t->target, t->handle);
    SetSampler(t->target, Sampler_Linear);
    SetAddressMode(t->target, AddressMode_Clamp);
    SetBlendMode(t->blend_mode);

    Vertex vertices[4]; 
    vertices[0].position = positions[0];
    vertices[0].uv       = uv_min;
    vertices[0].color    = color;

    vertices[1].position = positions[1];
    vertices[1].uv       = { uv_max.x, uv_min.y };
    vertices[1].color    = color;

    vertices[2].position = positions[2];
    vertices[2].uv       = { uv_min.x, uv_max.y };
    vertices[2].color    = color;

    vertices[3].position = positions[3];
    vertices[3].uv       = uv_max;
    vertices[3].color    = color;

    PushVertices(vertices, countof(vertices));
    SetShader(Shader_Sprite);
    UpdateShaderUniforms();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, countof(vertices));
}

void R_RenderCopyEx(R_Texture* texture, SDL_Rect* source, SDL_Rect* dest, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    if (texture == nullptr)
    {
        return;
    }
    RenderImpl(texture, source, dest, angle, center, flip);
}

void R_RenderDrawLine(int x1, int y1, int x2, int y2)
{
    Vector4 color = s_gl->current_render_color;
    Vertex vertices[2]; 
    vertices[0].position = { (float)x1, (float)y1 };
    vertices[0].uv       = {};
    vertices[0].color    = color;

    vertices[1].position = { (float)x2, (float)y2 };
    vertices[1].uv       = {};
    vertices[1].color    = color;

    PushVertices(vertices, countof(vertices));
    SetShader(Shader_Sprite);
    UpdateShaderUniforms();
    glBindTexture(s_gl->white_texture->target, s_gl->white_texture->handle);
    SetBlendMode(s_gl->white_texture->blend_mode);
    glDrawArrays(GL_LINES, 0, countof(vertices));
}

void R_RenderFillRect(SDL_Rect* rect)
{
    RenderImpl(nullptr, 0, rect, 0, 0, SDL_FLIP_NONE);
}

void R_RenderDrawRect(SDL_Rect* rect)
{
    R_RenderDrawLine(rect->x,           rect->y,           rect->x + rect->w, rect->y);
    R_RenderDrawLine(rect->x + rect->w, rect->y,           rect->x + rect->w, rect->y + rect->h);
    R_RenderDrawLine(rect->x + rect->w, rect->y + rect->h, rect->x,           rect->y + rect->h);
    R_RenderDrawLine(rect->x,           rect->y + rect->h, rect->x,           rect->y - 1);
}

// Enable this to request discrete graphics
#if 1
extern "C" {
#ifdef _MSC_VER
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
#else
    //__attribute__((dllexport)) u32 NvOptimusEnablement = 0x00000001;
    //__attribute__((dllexport)) int AmdPowerXpressRequestHighPerformance = 0x00000001;
    __attribute__((visibility("default"))) u32 NvOptimusEnablement = 0x00000001;
    __attribute__((visibility("default"))) int AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif
}
#endif
