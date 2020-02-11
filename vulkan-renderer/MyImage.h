#include <stdint.h>
#pragma once

enum MyImageFormat
{
    MY_IMAGEFORMAT_RGBA8,
    MY_IMAGEFORMAT_RGBA16_FLOAT,
    MY_IMAGEFORMAT_RGBA32_FLOAT,
    MY_IMAGEFORMAT_DXT1,
    MY_IMAGEFORMAT_DXT3,
    MY_IMAGEFORMAT_DXT5
};
class MyImage
{
public:
    MyImage(const char *filename);
    ~MyImage();

public:
    char          m_fileName[1024];
    void        * m_data;
    MyImageFormat   m_format;
    uint32_t      m_width;
    uint32_t      m_height;
    uint8_t       m_channels;
    uint32_t      m_layerCount;
    uint32_t      m_mipmapCount;
    uint32_t      m_blockSize;
    uint32_t      m_bufferSize;
    bool          m_bCompressed;
    bool          m_bFramebuffer;
};