#include "MyImage.h"
#include <cstring>
MyImage::MyImage(char *filename)
{
    std::memcpy(m_fileName, filename, 1024);
    m_data = nullptr;
}

MyImage::~MyImage() 
{
    if (m_data) 
    {
        delete [] m_data;
        m_data = nullptr;
    } 
}