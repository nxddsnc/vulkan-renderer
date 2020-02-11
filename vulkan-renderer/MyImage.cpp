#include "MyImage.h"
#include <cstring>
MyImage::MyImage(const char *filename)
{
    std::memcpy(m_fileName, filename, 1024);
    m_data = nullptr;
    m_format = MyImageFormat::MY_IMAGEFORMAT_RGBA8;
    m_mipmapCount = 1;
    m_layerCount = 1;
    m_blockSize = 1;
    m_bCompressed = false;
    m_bFramebuffer = false;
}

MyImage::~MyImage() 
{
    if (m_data) 
    {
        delete [] m_data;
        m_data = nullptr;
    } 
}