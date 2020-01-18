#include <stdint.h>
#pragma once

class MyImage
{
public:
    MyImage(char *filename);
    ~MyImage();

public:
    char          m_fileName[1024];
    void        * m_data;
    uint32_t      m_width;
    uint32_t      m_height;
    uint8_t       m_channels;
};