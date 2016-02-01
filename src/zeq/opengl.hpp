
#ifndef _ZEQ_OPENGL_HPP_
#define _ZEQ_OPENGL_HPP_

#include "define.hpp"
#include <FreeImage.h>

#define GL_TEXTURE_MAX_ANISOTROPY   0x84fe

#ifdef ZEQ_WINDOWS
#define GL_BGRA 0x80e1
#define GL_INTERNAL_PIXEL_FORMAT GL_BGRA
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88e4
#define GL_DYNAMIC_DRAW 0x88e8
#endif

#define GL_INTERNAL_PIXEL_FORMAT GL_BGRA

class OpenGL
{
private:
    bool extensionsLoaded;

    typedef void (__stdcall *GenerateMipmap)(int);
    GenerateMipmap glGenerateMipmap;

    // VBO
    typedef void (__stdcall *GenBuffers)(uint32_t, uint32_t*);
    GenBuffers glGenBuffers;

    typedef void (__stdcall *BindBuffer)(int, uint32_t);
    BindBuffer glBindBuffer;

    typedef void (__stdcall *BufferData)(int, int, const void*, int);
    BufferData glBufferData;

    typedef void (__stdcall *DeleteBuffers)(uint32_t, const uint32_t*);
    DeleteBuffers glDeleteBuffers;

private:
    static bool mipmapFlag();

public:
    OpenGL() : extensionsLoaded(false) { }
    
    static void     loadExtensions();
    static uint32_t loadTexture(byte* data, int width, int height);
    
    static uint32_t generateVBO(void* data, uint32_t len, bool isDynamic = false);
    static void     bindVBO(uint32_t vbo);
    static void     deleteVBO(uint32_t vbo);
};

#endif//_ZEQ_OPENGL_HPP_
