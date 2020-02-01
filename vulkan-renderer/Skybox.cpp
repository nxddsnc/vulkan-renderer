#include "Skybox.h"
#include "MyImage.h"
#include "MyTexture.h"
#include "ResourceManager.h"
#include "MyMesh.h"
#include "Drawable.h"
#include <iostream>

Skybox::Skybox(ResourceManager *resourceManager)
{
    m_pResourceManager = resourceManager;
    
    std::shared_ptr<MyMesh> mesh;
    mesh->CreateCube();
    vk::DeviceSize size = mesh->m_vertexNum * sizeof(mesh->m_positions[0]);

    vk::Buffer positionBuffer;
    VmaAllocation positionBufferMemory;
    vk::DeviceSize positionBufferOffset;
    resourceManager->CreateVertexBuffer(size, reinterpret_cast<void*>(mesh->m_positions.data()), positionBuffer, positionBufferMemory, positionBufferOffset);
    m_vertexBuffers.push_back(positionBuffer);
    m_vertexBufferMemorys.push_back(positionBufferMemory);
    m_vertexBufferOffsets.push_back(positionBufferOffset);

    size = mesh->m_vertexNum * sizeof(mesh->m_texCoords0[0]);
    vk::Buffer uvBuffer;
    VmaAllocation uvBufferMemory;
    vk::DeviceSize uvBufferOffset;
    resourceManager->CreateVertexBuffer(size, reinterpret_cast<void*>(mesh->m_texCoords0.data()), uvBuffer, uvBufferMemory, uvBufferOffset);
    m_vertexBuffers.push_back(uvBuffer);
    m_vertexBufferMemorys.push_back(uvBufferMemory);
    m_vertexBufferOffsets.push_back(uvBufferOffset);

    resourceManager->CreateIndexBuffer(mesh, m_indexBuffer, m_indexBufferMemory);

    
}


Skybox::~Skybox()
{
}

void Skybox::LoadFromFile(std::string filename)
{
    loadDDS(filename.c_str());
    //MyImage image(filename.c_str());
    //
    //image.width = static_cast<uint32_t>(texCube.extent().x);
    //height = static_cast<uint32_t>(texCube.extent().y);
    //mipLevels = static_cast<uint32_t>(texCube.levels());
    
}

bool Skybox::loadDDS(const char* path)
{
    // lay out variables to be used
    DDS_HEADER header;

    unsigned int blockSize;

    // open the DDS file for binary reading and get file size
    FILE* f;
    if ((f = fopen(path, "rb")) == 0)
        return false;
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // allocate new unsigned char space with 4 (file code) + 124 (header size) bytes
    // read in 128 bytes from the file
    unsigned char* magic = new unsigned char[4];
    fread(magic, 1, 4, f);
    // compare the `DDS ` signature
    if (memcmp(magic, "DDS ", 4) != 0)
    {
        std::cout  << "Header is not DDS." << std::endl;
        return false;
    }

    fread(&header, 1, 124, f);
 
    std::shared_ptr<MyImage> image = std::make_shared<MyImage>(path);
    vk::Format format;
    // figure out what format to use for what fourCC file type it is
    // block size is about physical chunk storage of compressed data in file (important)
    uint32_t size = 0;
    if (header.ddspf.dwFlags & DDPF_FOURCC) {
        switch (header.ddspf.dwFourCC) {
        case DDS_FORMAT::DXT1: // DXT1
            image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT1;
            image->m_blockSize = 1;
            break;
        case DDS_FORMAT::DXT3: // DXT3
            image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT3;
            image->m_blockSize = 2;
            break;
        case DDS_FORMAT::DXT5: // DXT5
            image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT5;
            image->m_blockSize = 2;
            break;
        case DDS_FORMAT::DX10: // DX10
            DDS_HEADER_DXT10 dx10Header;
            fread(&dx10Header, 1, sizeof(dx10Header), f);
            switch (dx10Header.dxgiFormat)
            {
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                image->m_format = MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT;
                image->m_blockSize = 2;
                break;
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                image->m_format = MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT;
                image->m_blockSize = 4;
                break;
            //case DXGI_FORMAT_BC1_UNORM:
            //    image->m_format = ImageFormat::RGBA16_FLOAT;
            //    break;
            //case DXGI_FORMAT_BC2_UNORM:
            //    format = vk::Format::eBc2UnormBlock;
            //    break;
            }
            break;
        default:
            image->m_format = MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT;
            image->m_blockSize = 2;
            break;
        }

        if (header.ddspf.dwFourCC == DDS_FORMAT::DX10)
        {
            size = file_size - 128 - sizeof(DDS_HEADER_DXT10);
            image->m_data = new unsigned char[size];
        }
        else
        {
            size = file_size - 128;
            image->m_data = new unsigned char[size];
        }
    }
    else // BC4U/BC4S/ATI2/BC55/R8G8_B8G8/G8R8_G8B8/UYVY-packed/YUY2-packed unsupported
    {
        return false;
    }

    // read rest of file
    fread(image->m_data, 1, size, f);

    image->m_width = header.dwWidth;
    image->m_height = header.dwHeight;
    image->m_mipmapCount = header.dwMipMapCount;
    image->m_channels = 4;
    image->m_bufferSize = size;
    
    if (header.dwCaps2 & 0x200)
    {
        image->m_layerCount = 6;
    }
    else
    {
        image->m_layerCount = 1;
    }
    m_pTexture = std::make_shared<MyTexture>();
    m_pTexture->m_pImage = image;
    m_pTexture->m_wrapMode[0] = WrapMode::WRAP;
    m_pTexture->m_wrapMode[1] = WrapMode::WRAP;
    m_pTexture->m_wrapMode[2] = WrapMode::WRAP;

    m_pResourceManager->CreateCombinedTexture(m_pTexture);

    fclose(f);
}

