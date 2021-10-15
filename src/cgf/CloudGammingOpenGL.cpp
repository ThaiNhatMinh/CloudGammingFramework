#include <string>
#include "CloudGammingOpenGL.hh"
#include <Windows.h>

const int PBO_COUNT = 2;
const int BPP = 4;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned char GLboolean;
typedef int GLsizei;
#ifdef _WIN64
typedef signed long long int khronos_intptr_t;
typedef signed long long int khronos_ssize_t;
#else
typedef signed long int khronos_intptr_t;
typedef signed long int khronos_ssize_t;
#endif
typedef khronos_ssize_t GLsizeiptr;
typedef void (*PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void *(*PFNGLMAPBUFFERPROC)(GLenum target, GLenum access);
typedef GLboolean (*PFNGLUNMAPBUFFERPROC)(GLenum target);
typedef void (*PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef PROC(__stdcall *GL3WglGetProcAddr)(LPCSTR);

PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
GL3WglGetProcAddr wgl_get_proc_address;

extern "C"
{

    extern void glReadBuffer(GLenum src);
    extern void glReadPixels(GLint x,
                             GLint y,
                             GLsizei width,
                             GLsizei height,
                             GLenum format,
                             GLenum type,
                             void *data);
    extern void glBindTexture(GLenum target,
                              GLuint texture);
    extern void glGenTextures(GLsizei n,
                              GLuint *textures);
    extern void glTexParameteri(GLenum target,
                                GLenum pname,
                                GLint param);
    extern void glTexImage2D(GLenum target,
                             GLint level,
                             GLint internalformat,
                             GLsizei width,
                             GLsizei height,
                             GLint border,
                             GLenum format,
                             GLenum type,
                             const void *data);
    extern void glTexSubImage2D(GLenum target,
                                GLint level,
                                GLint xoffset,
                                GLint yoffset,
                                GLsizei width,
                                GLsizei height,
                                GLenum format,
                                GLenum type,
                                const void *pixels);
    extern void glDeleteTextures(GLsizei n,
                                 const GLuint *textures);
};

#define GL_STREAM_READ 0x88E1
#define GL_FRONT 0x0404
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_UNSIGNED_BYTE 0x1401
#define GL_BGRA 0x80E1
#define GL_READ_ONLY 0x88B8
#define GL_STREAM_DRAW 0x88E0
#define GL_TEXTURE_2D 0x0DE1
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_RGBA 0x1908
#define GL_WRITE_ONLY 0x88B9

struct InternalData
{
    GLuint textureId;
    GLuint pboId[PBO_COUNT];
    GLuint width;
    GLuint height;
    GLuint index;
    GLuint frameSize;
    HMODULE libgl;
} data;

void CreateTexture(int width, int height);
void CreatePixelBufferObject(int width, int height, int flags);
void LoadProc();

void cgfCaptureOpenglInit(int width, int height)
{
    LoadProc();
    data.width = width;
    data.height = height;
    data.index = 0;
    CreatePixelBufferObject(width, height, GL_STREAM_READ);
}

void cgfCaptureOpenglFrame(frameProcessFnc fnc)
{
    int nextIndex = 0; // pbo index used for next frame
    data.index = (data.index + 1) % 2;
    nextIndex = (data.index + 1) % 2;
    glReadBuffer(GL_FRONT);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, data.pboId[data.index]);
    glReadPixels(0, 0, data.width, data.height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, data.pboId[nextIndex]);
    GLubyte *src = (GLubyte *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

    if (src)
    {
        fnc(src);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // release pointer to the mapped buffer
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

GLuint cgfOpenglInit(int width, int height)
{
    LoadProc();
    data.width = width;
    data.height = height;
    data.index = 0;
    CreateTexture(width, height);
    CreatePixelBufferObject(width, height, GL_STREAM_DRAW);
    return data.textureId;
}

void cgfUpdateOpenglTexture(const void *pData)
{
    int nextIndex = 0; // pbo index used for next frame
    data.index = (data.index + 1) % 2;
    nextIndex = (data.index + 1) % 2;
    glBindTexture(GL_TEXTURE_2D, data.textureId);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, data.pboId[data.index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.width, data.height, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, data.pboId[nextIndex]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, data.width * data.height * BPP, 0, GL_STREAM_DRAW);
    GLubyte *ptr = (GLubyte *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr)
    {
        // update data directly on the mapped buffer
        std::memcpy(ptr, pData, data.width * data.height * BPP);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void cgfOpenglRelease()
{
    glDeleteTextures(1, &data.textureId);
    glDeleteBuffers(PBO_COUNT, data.pboId);
    FreeModule(data.libgl);
}

void cgfCaptureOpenglRelease()
{
    glDeleteBuffers(PBO_COUNT, data.pboId);
}

void CreateTexture(int width, int height)
{
    GLuint textId;
    glGenTextures(1, &textId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    data.textureId = textId;
}

void CreatePixelBufferObject(int width, int height, int flags)
{
    // create 2 pixel buffer objects, you need to delete them when program exits.
    // glBufferData() with NULL pointer reserves only memory space.
    glGenBuffers(PBO_COUNT, data.pboId);
    for (int i = 0; i < PBO_COUNT; i++)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, data.pboId[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, width * height * BPP, 0, flags);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void LoadProc()
{
    data.libgl = LoadLibraryA("opengl32.dll");
    if (!data.libgl)
        return;
    wgl_get_proc_address = (GL3WglGetProcAddr)GetProcAddress(data.libgl, "wglGetProcAddress");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wgl_get_proc_address("glBindBuffer");
    glMapBuffer = (PFNGLMAPBUFFERPROC)wgl_get_proc_address("glMapBuffer");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wgl_get_proc_address("glUnmapBuffer");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wgl_get_proc_address("glActiveTexture");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wgl_get_proc_address("glGenBuffers");
    glBufferData = (PFNGLBUFFERDATAPROC)wgl_get_proc_address("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wgl_get_proc_address("glDeleteBuffers");
}
