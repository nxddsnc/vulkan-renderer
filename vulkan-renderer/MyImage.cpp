#include "MyImage.h"
#include <cstring>
#include "stb_image_write.h"
#include "Utils.h"

MyImage::MyImage(const char *filename)
{
    std::memcpy(m_fileName, filename, 1024);
    m_data = nullptr;
    m_format = MyImageFormat::MY_IMAGEFORMAT_RGBA8;
    m_mipmapCount = 1;
    m_layerCount = 1;
    m_blockSize = 1;

	// TODO: Move the following variables to MyTexture?
    m_bCompressed = false;
    m_bFramebuffer = false;
	m_bHostVisible = false;
	m_bTransferSrc = false;
}

MyImage::MyImage(const char * name, int width, int height, MyImageFormat format, bool bFramebuffer)
{
	std::memcpy(m_fileName, name, 1024);
	m_data = nullptr;
	m_width = width;
	m_height = height;
	m_format = format;
	m_mipmapCount = 1;
	m_layerCount = 1;

	switch (format)
	{
	case MY_IMAGEFORMAT_RGBA8:
		m_blockSize = 1;
		m_channels = 4;
		break;
	case MY_IMAGEFORMAT_RGBA16_FLOAT:
		m_blockSize = 2;
		m_channels = 4;
		break;
	case MY_IMAGEFORMAT_RGBA32_FLOAT:
		m_blockSize = 4;
		m_channels = 4;
		break;
	case MY_IMAGEFORMAT_DXT1:
	case MY_IMAGEFORMAT_DXT2:
	case MY_IMAGEFORMAT_DXT3:
	case MY_IMAGEFORMAT_DXT5:
		break;
	case MY_IMAGEFORMAT_D24S8_UINT:
		m_blockSize = 4;
		m_channels = 1;
		break;
	}

	m_bFramebuffer = bFramebuffer;
	m_bCompressed = false;
	m_bHostVisible = false;
	m_bTransferSrc = false;
}

MyImage::~MyImage() 
{
    if (m_data) 
    {
        delete [] m_data;
        m_data = nullptr;
    } 
}

glm::vec3 MyImage::ReadPixel(int x, int y)
{
	glm::vec3 res = glm::vec3();
	if (MY_IMAGEFORMAT_RGBA16_FLOAT == m_format)
	{
		uint16_t *imageData = reinterpret_cast<uint16_t*>(m_data);
		int offset = (y * m_width + x) * m_channels;
		float32(&(res.x), imageData[offset]);
		float32(&(res.y), imageData[offset + 1]);
		float32(&(res.z), imageData[offset + 2]);
	}

	//res.x = pow(res.x, 2.2);
	//res.y = pow(res.y, 2.2);
	//res.z = pow(res.z, 2.2);
	return res;
}

void MyImage::DumpImageHDR(char * filename)
{
	if (MY_IMAGEFORMAT_RGBA16_FLOAT == m_format)
	{
		float *data = new float[m_width * m_height * m_channels];

		uint16_t *src = reinterpret_cast<uint16_t*>(m_data);

		int size = m_width * m_height * m_channels;
		for (int i = 0; i < size; ++i)
		{
			float32(&(data[i]), src[i]);
		}
		stbi_write_hdr(filename, m_width, m_height, m_channels, reinterpret_cast<const float*>(data));
	}
	else if (MY_IMAGEFORMAT_RGBA32_FLOAT == m_format)
	{
		stbi_write_hdr(filename, m_width, m_height, m_channels, reinterpret_cast<const float*>(m_data));
	}
	else
	{
		std::printf("error:  not implemented!!");
	}

}


