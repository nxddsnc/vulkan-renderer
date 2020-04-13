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


