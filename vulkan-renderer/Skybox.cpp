#include "Skybox.h"
#include "MyImage.h"
#include "MyTexture.h"
#include "ResourceManager.h"
#include "MyMesh.h"
#include "Renderable.h"
#include <iostream>
#include "Pipeline.h"
#include "PipelineManager.h"
#include "RenderPass.h"
#include "Context.h"
#include <glm/matrix.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include "SHLight.h"
#include "MyCamera.h"
#include "hdrloader.h"

Skybox::Skybox(ResourceManager *resourceManager, PipelineManager *pipelineManager, VulkanContext *context)
{
    m_pResourceManager = resourceManager;
	m_pPipelineManager = pipelineManager;
    m_pContext = context;
    
    m_pRenderable = std::make_shared<SingleRenderable>();
    m_pRenderable->m_mesh = std::make_shared<MyMesh>();
    m_pRenderable->m_mesh->CreateCube();

    resourceManager->InitVulkanBuffers(m_pRenderable);

}


Skybox::~Skybox()
{

}

bool Skybox::LoadFromCubmapDDS(const char* path, vk::Device device, vk::DescriptorPool &descriptorPool)
{
    m_bCubemap = true;
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
		std::cout << "Header is not DDS." << std::endl;
		return false;
	}

	fread(&header, 1, 124, f);

	std::shared_ptr<MyImage> image = std::make_shared<MyImage>(path);
	vk::Format format;
	// figure out what format to use for what fourCC file type it is
	// block size is about physical chunk storage of compressed data in file (important)
	// FIXME: Still some formats are not supported.
	uint32_t size = 0;
	if (header.ddspf.dwFlags & DDPF_FOURCC) {
		switch (header.ddspf.dwFourCC) {
		case DDS_FORMAT::DXT1: // DXT1
			image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT1;
			image->m_blockSize = 1;
			break;
		case DDS_FORMAT::DXT2:
			image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT2;
			image->m_blockSize = 1;
			break;
		case DDS_FORMAT::DXT3: // DXT3
			image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT3;
			image->m_blockSize = 2;
			break;
		case DDS_FORMAT::DXT4:
			break;
		case DDS_FORMAT::DXT5: // DXT5
			image->m_format = MyImageFormat::MY_IMAGEFORMAT_DXT5;
			image->m_blockSize = 2;
			break;
		case DDS_FORMAT::BC4U:
		case DDS_FORMAT::BC4S:
		case DDS_FORMAT::BC5U:
		case DDS_FORMAT::BC5S:
		case DDS_FORMAT::ETC:
		case DDS_FORMAT::ETC1:
		case DDS_FORMAT::ATC:
		case DDS_FORMAT::ATCA:
		case DDS_FORMAT::ATCI:
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
	image->m_bTransferSrc = true;

	if (header.dwCaps2 & 0x200)
	{
		image->m_layerCount = 6;
	}
	else
	{
		image->m_layerCount = 1;
	}
	m_pTextureEnvMap = std::make_shared<MyTexture>();
	m_pTextureEnvMap->m_pImage = image;
	m_pTextureEnvMap->m_wrapMode[0] = WrapMode::WRAP;
	m_pTextureEnvMap->m_wrapMode[1] = WrapMode::WRAP;
	m_pTextureEnvMap->m_wrapMode[2] = WrapMode::WRAP;

	m_pVulkanTextureEnvMap = m_pResourceManager->CreateCombinedTexture(m_pTextureEnvMap);
	m_pResourceManager->InitVulkanTextureData(m_pTextureEnvMap, m_pVulkanTextureEnvMap);
	fclose(f);

	{
		std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		vk::DescriptorSetLayoutBinding textureBinding(0,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		textureBindings.push_back(textureBinding);

		vk::DescriptorImageInfo imageInfo(m_pVulkanTextureEnvMap->imageSampler,
			m_pVulkanTextureEnvMap->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		imageInfos.push_back(imageInfo);

		vk::DescriptorSetLayout descriptorSetLayout;

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
		descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

		vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &descriptorSetLayout);

		device.allocateDescriptorSets(&allocInfo, &m_dsSkybox);

		vk::WriteDescriptorSet descriptorWrite(m_dsSkybox,
			uint32_t(0),
			uint32_t(0),
			static_cast<uint32_t>(imageInfos.size()),
			vk::DescriptorType::eCombinedImageSampler,
			imageInfos.data(),
			{},
			{});
		device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
		device.destroyDescriptorSetLayout(descriptorSetLayout);
	}


    m_pVulkanTexturePrefilteredEnvMap = generatePrefilteredCubeMap(descriptorPool);
    m_pVulkanTextureIrradianceMap = generateIrradianceMap(descriptorPool);
    m_pVulkanTextureBRDFLUT = generateBRDFLUT(descriptorPool);

	initSHLight();

	m_pSHLight->CreateDescriptorSet(device, descriptorPool);

    m_preFilteredDescriptorSet = m_pResourceManager->CreateTextureDescriptorSet({ m_pVulkanTexturePrefilteredEnvMap, m_pVulkanTextureIrradianceMap, m_pVulkanTextureBRDFLUT });

        {
        std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
        std::vector<vk::DescriptorImageInfo> imageInfos;
        vk::DescriptorSetLayoutBinding textureBinding(0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(m_pVulkanTextureEnvMap->imageSampler,
            m_pVulkanTextureEnvMap->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.push_back(imageInfo);

        vk::DescriptorSetLayout descriptorSetLayout;

        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
        descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

        vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &descriptorSetLayout);

        device.allocateDescriptorSets(&allocInfo, &m_dsSkybox);

        vk::WriteDescriptorSet descriptorWrite(m_dsSkybox,
            uint32_t(0),
            uint32_t(0),
            static_cast<uint32_t>(imageInfos.size()),
            vk::DescriptorType::eCombinedImageSampler,
            imageInfos.data(),
            {},
            {});
        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        device.destroyDescriptorSetLayout(descriptorSetLayout);
    }

}

bool Skybox::DumpCubemapToDDS(const char* path, void* data, DWORD width, DWORD height, DWORD mipmapCount, uint32_t size)
{
	DDS_HEADER header;

	unsigned int blockSize;

	// open the DDS file for binary reading and get file size
	FILE* f;
	if ((f = fopen(path, "wb")) == 0)
		return false;
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// allocate new unsigned char space with 4 (file code) + 124 (header size) bytes
	// read in 128 bytes from the file
	unsigned char* magic = new unsigned char[4];
	//fread(magic, 1, 4, f);
    fwrite("DDS ", 4, 1, f);
	// compare the `DDS ` signature
	//if (memcmp(magic, "DDS ", 4) != 0)
	//{
	//	std::cout << "Header is not DDS." << std::endl;
	//	return false;
	//}
	header.dwSize = 124;
	header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x20000; // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT
	header.dwHeight = height;
	header.dwWidth = width;
	header.dwMipMapCount = mipmapCount;
	header.ddspf.dwSize = 32;
    header.ddspf.dwFlags = 0x4;
    header.dwCaps = 0x8 | 0x400000 | 0x1000;
    header.dwCaps2 = 0x200 | 0x400 | 0x800 | 0x1000 | 0x2000 | 0x4000 | 0x8000;
    header.dwPitchOrLinearSize = 512 * 4 * 4;
    header.dwDepth = 1;
    header.dwReserved1[0] = 0;
    header.dwCaps3 = header.dwCaps4 = header.dwReserved2 = 0;
    //DDS_HEADER_DXT10 fourCC;
    //fourCC.dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    //fourCC.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
    //fourCC.miscFlag = 0x04;
    //fourCC.miscFlags2 = 0x01;
    //fourCC.arraySize = 1;

    //memcpy(&header.ddspf.dwFourCC, &fourCC, sizeof(DWORD));
    header.ddspf.dwFourCC = DDS_FORMAT::DX10;
	header.ddspf.dwRBitMask = 0x00ff0000;
	header.ddspf.dwGBitMask = 0x0000ff00;
	header.ddspf.dwBBitMask = 0x000000ff;
	header.ddspf.dwABitMask = 0xff000000;
    header.ddspf.dwRGBBitCount = 128;

	fwrite(&header, 124, 1, f);

	DDS_HEADER_DXT10 dx10Header;

	dx10Header.dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	dx10Header.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
	dx10Header.miscFlag = 0x04;
	dx10Header.arraySize = 1;
	dx10Header.miscFlags2 = 0x01;
    fwrite(&dx10Header, 1, sizeof(dx10Header), f);

    fwrite(data, size, 1, f);

    fclose(f);

    return true;
}


bool Skybox::LoadFromPanoramaHdr(const char* path, vk::Device device, vk::DescriptorPool& descriptorPool)
{
    m_bCubemap = false;
    HDRLoaderResult res;
    //{
    //public:
    //    int width, height;
    //    // each pixel takes 3 float32, each component can be of any value...
    //    float* cols;
    //};

    HDRLoader::load(path, res);

    std::shared_ptr<MyImage> image = std::make_shared<MyImage>(path);

    image->m_width = res.width;
    image->m_height = res.height;
    // FIXME: RGB32Float is not supported on my machine.
    // Should query available formats on init!!!
    std::vector<float> tempBuffer;
    tempBuffer.resize(4 * res.width * res.height);
    for (int i = 0; i < res.width; i++)
    {
        for (int j = 0; j < res.height; j++)
        {
            tempBuffer[(j * res.width + i) * 4] = res.cols[(j * res.width + i) * 3];
            tempBuffer[(j * res.width + i) * 4 + 1] = res.cols[(j * res.width + i) * 3 + 1];
            tempBuffer[(j * res.width + i) * 4 + 2] = res.cols[(j * res.width + i) * 3 + 2];
            tempBuffer[(j * res.width + i) * 4 + 3] = 1.0f;
        }
    }
    image->m_data = (void*)(tempBuffer.data());

    image->m_mipmapCount = 1;
    image->m_channels = 4;
    image->m_bufferSize = sizeof(float) * 4 * res.width * res.height;
    image->m_bTransferSrc = true;
    image->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;

    image->m_layerCount = 1;
    
    m_pTextureEnvMap = std::make_shared<MyTexture>();
    m_pTextureEnvMap->m_pImage = image;
    m_pTextureEnvMap->m_wrapMode[0] = WrapMode::WRAP;
    m_pTextureEnvMap->m_wrapMode[1] = WrapMode::WRAP;
    m_pTextureEnvMap->m_wrapMode[2] = WrapMode::WRAP;

    m_pVulkanTextureEnvMap = m_pResourceManager->CreateCombinedTexture(m_pTextureEnvMap);
    m_pResourceManager->InitVulkanTextureData(m_pTextureEnvMap, m_pVulkanTextureEnvMap);

    // FIXME: Stupid way to avoid duplicated delete...
    image->m_data = nullptr;

    {
        std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
        std::vector<vk::DescriptorImageInfo> imageInfos;
        vk::DescriptorSetLayoutBinding textureBinding(0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment,
            {});
        textureBindings.push_back(textureBinding);

        vk::DescriptorImageInfo imageInfo(m_pVulkanTextureEnvMap->imageSampler,
            m_pVulkanTextureEnvMap->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);

        imageInfos.push_back(imageInfo);

        vk::DescriptorSetLayout descriptorSetLayout;

        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
        descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

        vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &descriptorSetLayout);

        device.allocateDescriptorSets(&allocInfo, &m_dsSkybox);

        vk::WriteDescriptorSet descriptorWrite(m_dsSkybox,
            uint32_t(0),
            uint32_t(0),
            static_cast<uint32_t>(imageInfos.size()),
            vk::DescriptorType::eCombinedImageSampler,
            imageInfos.data(),
            {},
            {});
        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        device.destroyDescriptorSetLayout(descriptorSetLayout);
    }

    m_pVulkanTexturePrefilteredEnvMap = generatePrefilteredCubeMap(descriptorPool);
    m_pVulkanTextureIrradianceMap = generateIrradianceMap(descriptorPool);
    m_pVulkanTextureBRDFLUT = generateBRDFLUT(descriptorPool);

    initSHLight();

    m_pSHLight->CreateDescriptorSet(device, descriptorPool);

    m_preFilteredDescriptorSet = m_pResourceManager->CreateTextureDescriptorSet({ m_pVulkanTexturePrefilteredEnvMap, m_pVulkanTextureIrradianceMap, m_pVulkanTextureBRDFLUT });
    
	/*{
		std::vector<vk::DescriptorSetLayoutBinding> textureBindings;
		std::vector<vk::DescriptorImageInfo> imageInfos;
		vk::DescriptorSetLayoutBinding textureBinding(0,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			{});
		textureBindings.push_back(textureBinding);

		vk::DescriptorImageInfo imageInfo(m_pVulkanTextureEnvMap->imageSampler,
            m_pVulkanTexturePrefilteredEnvMap->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);

		imageInfos.push_back(imageInfo);

		vk::DescriptorSetLayout descriptorSetLayout;

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t>(textureBindings.size()), textureBindings.data());
		descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

		vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, 1, &descriptorSetLayout);

		device.allocateDescriptorSets(&allocInfo, &m_dsSkybox);

		vk::WriteDescriptorSet descriptorWrite(m_dsSkybox,
			uint32_t(0),
			uint32_t(0),
			static_cast<uint32_t>(imageInfos.size()),
			vk::DescriptorType::eCombinedImageSampler,
			imageInfos.data(),
			{},
			{});
		device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
		device.destroyDescriptorSetLayout(descriptorSetLayout);
	}*/

    return true;
}

void Skybox::Draw(vk::CommandBuffer & commandBuffer, std::shared_ptr<MyCamera> camera, std::shared_ptr<RenderPass> renderPass)
{
	PipelineId skyBoxPipelineId;
	skyBoxPipelineId.type = PipelineType::SKYBOX;
	skyBoxPipelineId.model.primitivePart.info.bits.positionVertexData = 1;
	skyBoxPipelineId.model.primitivePart.info.bits.normalVertexData = 0;
	skyBoxPipelineId.model.primitivePart.info.bits.countTexCoord = 1;
	skyBoxPipelineId.model.primitivePart.info.bits.tangentVertexData = 0;
	skyBoxPipelineId.model.primitivePart.info.bits.countColor = 0;
	std::shared_ptr<Pipeline> pipelineSkybox = m_pPipelineManager->GetPipeline(skyBoxPipelineId, renderPass);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipeline());
	commandBuffer.bindVertexBuffers(0, m_pRenderable->m_vertexBuffers.size(), m_pRenderable->m_vertexBuffers.data(), m_pRenderable->m_vertexBufferOffsets.data());
	commandBuffer.bindIndexBuffer(m_pRenderable->m_indexBuffer, 0, vk::IndexType::eUint16);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipelineLayout(), 0, 1, &camera->m_descriptorSet, 0, nullptr);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineSkybox->GetPipelineLayout(), 1, 1, &m_dsSkybox, 0, nullptr);
	commandBuffer.drawIndexed(m_pRenderable->m_mesh->m_indexNum, 1, 0, 0, 0);
}

std::shared_ptr<VulkanTexture> Skybox::generatePrefilteredCubeMap(vk::DescriptorPool &descriptorPool)
{
    uint32_t width = PREFILTERED_IMAGE_WIDTH;
    uint32_t height = PREFILTERED_IMAGE_HEIGHT;
    uint32_t mipmapCount = static_cast<uint32_t>(floor(log2(width)) + +1);
    std::shared_ptr<MyTexture> offscreenTexture = std::make_shared<MyTexture>();
    offscreenTexture->m_pImage = std::make_shared<MyImage>("prefiltered-env-map-colorAttachment");
    offscreenTexture->m_pImage->m_width = width;
    offscreenTexture->m_pImage->m_height = height;
    offscreenTexture->m_pImage->m_channels = 4;
    offscreenTexture->m_pImage->m_bufferSize = width * height * 4 * 4;
    offscreenTexture->m_pImage->m_mipmapCount = 1;
    offscreenTexture->m_pImage->m_layerCount = 1;
    offscreenTexture->m_pImage->m_bFramebuffer = true;
    offscreenTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;

    offscreenTexture->m_wrapMode[0] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[1] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[2] = WrapMode::CLAMP;

    //std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateCombinedTexture(offscreenTexture);
    std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateVulkanTexture(offscreenTexture);

    m_pTexturePrefilteredEnvMap = std::make_shared<MyTexture>();
    m_pTexturePrefilteredEnvMap->m_pImage = std::make_shared<MyImage>("prefiltered-env-map");
    m_pTexturePrefilteredEnvMap->m_pImage->m_width = width;
    m_pTexturePrefilteredEnvMap->m_pImage->m_height = height;
    m_pTexturePrefilteredEnvMap->m_pImage->m_channels = 4;
    m_pTexturePrefilteredEnvMap->m_pImage->m_bufferSize = 0;
    for (int i = 0; i < mipmapCount; ++i)
    {
        m_pTexturePrefilteredEnvMap->m_pImage->m_bufferSize += width * height * 4 * 4 / std::max(1, int((4, i)));
    }
    m_pTexturePrefilteredEnvMap->m_pImage->m_bufferSize *= 6;
    m_pTexturePrefilteredEnvMap->m_pImage->m_mipmapCount = mipmapCount;
    m_pTexturePrefilteredEnvMap->m_pImage->m_layerCount = 6;
    m_pTexturePrefilteredEnvMap->m_pImage->m_bFramebuffer = false;
    m_pTexturePrefilteredEnvMap->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;
    m_pTexturePrefilteredEnvMap->m_wrapMode[0] = WrapMode::CLAMP;
    m_pTexturePrefilteredEnvMap->m_wrapMode[1] = WrapMode::CLAMP;
    m_pTexturePrefilteredEnvMap->m_wrapMode[2] = WrapMode::CLAMP;
    //m_pTexturePrefilteredEnvMap->m_pImage->m_bHostVisible = true;

    m_pVulkanTexturePrefilteredEnvMap = m_pResourceManager->CreateCombinedTexture(m_pTexturePrefilteredEnvMap);

    vk::Device device = m_pContext->GetLogicalDevice();
    RenderPass renderPass(&device);
    vk::AttachmentDescription colorAttachment(
        {},
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    );

    renderPass.AddAttachment(colorAttachment);

    vk::RenderPass vulkanRenderPass = renderPass.Get();

    std::array<vk::ImageView, 1> attachments{};
    attachments[0] = offscreenVulkanTexture->imageView;

    vk::FramebufferCreateInfo createInfo({},
        vulkanRenderPass,
        (uint32_t)attachments.size(),
        attachments.data(),
        width,
        height,
        uint32_t(1));
    vk::Framebuffer framebuffer = device.createFramebuffer(createInfo);

    std::array<vk::ClearValue, 1> clearValues{};
    clearValues[0].color.float32[0] = 0.0;
    clearValues[0].color.float32[1] = 0.0;
    clearValues[0].color.float32[2] = 0.0;
    clearValues[0].color.float32[3] = 1.0f;

    vk::RenderPassBeginInfo renderPassInfo(
        vulkanRenderPass,
        framebuffer,
        vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)),
        (uint32_t)clearValues.size(),
        clearValues.data());

    PipelineId id;
    id.type = PREFILTERED_CUBE_MAP;
    id.model.primitivePart.info.bits.positionVertexData = 1;
    id.model.primitivePart.info.bits.normalVertexData = 0;
    id.model.primitivePart.info.bits.countTexCoord = 0;
	id.model.primitivePart.info.bits.primitiveMode = PrimitiveMode::Triangles;
    Pipeline pipeline(id);
    pipeline.InitPrefilteredCubeMap(m_pContext->GetLogicalDevice(), vulkanRenderPass, m_pTextureEnvMap->m_pImage->m_layerCount > 1);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo({
        m_pContext->GetCommandPool() ,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(1)
    });

    struct PushBlock {
        glm::mat4 mvp;
        float roughness;
        //uint32_t numSamples = 32u;
    } pushBlock;

    std::vector<glm::mat4> matrices = {
        // POSITIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };
   
    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
    vk::CommandBufferBeginInfo beginInfo({
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        nullptr
    });

    commandBuffer.begin(beginInfo);
    
    // transit offscreen image layout
    vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    // transit prefiltered cubemap image layout
    vk::ImageSubresourceRange ssrCubemap(vk::ImageAspectFlagBits::eColor, 0, mipmapCount, 0, 6);
    m_pResourceManager->SetImageLayout(commandBuffer, m_pVulkanTexturePrefilteredEnvMap->image, vk::Format::eR32G32B32A32Sfloat, ssrCubemap,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    // FIXME: ugly way to copy texture from gpu to cpu...
    std::vector<std::vector<std::shared_ptr<MyTexture>>> textures;
    std::vector<std::vector<std::shared_ptr<VulkanTexture>>> vulkanTextures;
    for (int level = 0; level < mipmapCount; ++level)
    {
        textures.push_back(std::vector<std::shared_ptr<MyTexture>>());
        vulkanTextures.push_back(std::vector<std::shared_ptr<VulkanTexture>>());
        float roughness = (float)level / (float)(mipmapCount - 1);
        pushBlock.roughness = roughness;
        for (int i = 0; i < 6; ++i)
        {
            pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[i];

            vk::Viewport viewport(0, 0, width * std::pow(0.5f, level), width * std::pow(0.5f, level), 0, 1);
            vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(viewport.width, viewport.height));
            commandBuffer.setViewport(0, 1, &viewport);
            commandBuffer.setScissor(0, 1, &sissor);

            commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
           
            // skybox command buffer
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());
            commandBuffer.bindVertexBuffers(0, m_pRenderable->m_vertexBuffers.size(), m_pRenderable->m_vertexBuffers.data(), m_pRenderable->m_vertexBufferOffsets.data());
            commandBuffer.bindIndexBuffer(m_pRenderable->m_indexBuffer, 0, vk::IndexType::eUint16);

            commandBuffer.pushConstants(pipeline.GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(pushBlock), reinterpret_cast<void*>(&pushBlock));
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, 1, &m_dsSkybox, 0, nullptr);

            commandBuffer.drawIndexed(m_pRenderable->m_mesh->m_indexNum, 1, 0, 0, 0);

            commandBuffer.endRenderPass();

            /*********************************************************************/
			std::shared_ptr<MyTexture> tempTexture = std::make_shared<MyTexture>();
			char imageName[32];
			sprintf(imageName, "TempImage_%d", i);
			tempTexture->m_pImage = std::make_shared<MyImage>(imageName);
			tempTexture->m_pImage->m_width = viewport.width;
			tempTexture->m_pImage->m_height = viewport.height;
			tempTexture->m_pImage->m_channels = 4;
			tempTexture->m_pImage->m_bufferSize = viewport.width * viewport.height * 4 * 4;
			tempTexture->m_pImage->m_mipmapCount = 1;
			tempTexture->m_pImage->m_layerCount = 1;
			tempTexture->m_pImage->m_bFramebuffer = false;
			tempTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;
			tempTexture->m_pImage->m_bHostVisible = true;

			tempTexture->m_wrapMode[0] = WrapMode::CLAMP;
			tempTexture->m_wrapMode[1] = WrapMode::CLAMP;
			tempTexture->m_wrapMode[2] = WrapMode::CLAMP;


			std::shared_ptr<VulkanTexture> tempVulkanTexture = m_pResourceManager->CreateVulkanTexture(tempTexture);

			textures[level].push_back(tempTexture);
			vulkanTextures[level].push_back(tempVulkanTexture);
            /*********************************************************************/


			//m_pResourceManager->TransferGPUTextureToCPU(offscreenVulkanTexture, cubemapData + offset, viewport.width* viewport.height * 4 * 4);
			//offset += viewport.width* viewport.height * 4 * 4;

            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

            // Copy region for transfer from framebuffer to cube face
            vk::ImageCopy copyRegion(
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, level, i, 1),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(viewport.width, viewport.height, 1));

            commandBuffer.copyImage(offscreenVulkanTexture->image, vk::ImageLayout::eTransferSrcOptimal, 
                m_pVulkanTexturePrefilteredEnvMap->image,
                vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

            vk::ImageSubresourceRange ssrTemp(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			m_pResourceManager->SetImageLayout(commandBuffer, tempVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, ssrTemp,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

			vk::ImageCopy copyRegionTemp(
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
				vk::Offset3D(0, 0, 0),
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
				vk::Offset3D(0, 0, 0),
				vk::Extent3D(viewport.width, viewport.height, 1));
			commandBuffer.copyImage(offscreenVulkanTexture->image, vk::ImageLayout::eTransferSrcOptimal,
                tempVulkanTexture->image,
				vk::ImageLayout::eTransferDstOptimal, 1, &copyRegionTemp);

			m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
				vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    // transit prefiltered cubemap image layout back
    m_pResourceManager->SetImageLayout(commandBuffer, m_pVulkanTexturePrefilteredEnvMap->image, vk::Format::eR32G32B32A32Sfloat, ssrCubemap,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    commandBuffer.end();

    vk::SubmitInfo submitInfo({
        {},
        {},
        {},
        (uint32_t)1,
        &commandBuffer,
        {},
        {}
    });
    m_pContext->GetDeviceQueue().submit((uint32_t)1, &submitInfo, nullptr);
    m_pContext->GetDeviceQueue().waitIdle();
    device.freeCommandBuffers(m_pContext->GetCommandPool(), 1, &commandBuffer);
    device.destroyFramebuffer(framebuffer);

	char* cubemapData = new char[m_pTexturePrefilteredEnvMap->m_pImage->m_bufferSize];
	size_t offset = 0;
    size_t tempWidth, tempHeight = 0;
    for (int i = 0; i < 6; ++i)
    {
        for (int level = 0; level < mipmapCount; ++level)
		{
            tempWidth = width* std::pow(0.5f, level);
            tempHeight = height* std::pow(0.5f, level);
            //m_pResourceManager->TransferGPUTextureToCPU(vulkanTextures[level][i], textures[level][i]);
			m_pResourceManager->TransferGPUTextureToCPU(vulkanTextures[level][i], cubemapData + offset, tempWidth * tempHeight * 4 * 4);
			offset += tempWidth * tempHeight * 4 * 4;
        }
     }
    DumpCubemapToDDS("D:\\Coding\\github\\vulkan-renderer\\vulkan-renderer\\TestModel\\Skybox\\environment.dds", cubemapData, width, height, mipmapCount, m_pTexturePrefilteredEnvMap->m_pImage->m_bufferSize);
    delete [] cubemapData;

    return m_pVulkanTexturePrefilteredEnvMap;
}

std::shared_ptr<VulkanTexture> Skybox::generateIrradianceMap(vk::DescriptorPool &descriptorPool)
{
    uint32_t width = 32;
    uint32_t height = 32;
    uint32_t mipmapCount = static_cast<uint32_t>(floor(log2(width)) + +1);
    std::shared_ptr<MyTexture> offscreenTexture = std::make_shared<MyTexture>();
    offscreenTexture->m_pImage = std::make_shared<MyImage>("prefiltered-env-map-colorAttachment");
    offscreenTexture->m_pImage->m_width = width;
    offscreenTexture->m_pImage->m_height = height;
    offscreenTexture->m_pImage->m_channels = 4;
    offscreenTexture->m_pImage->m_bufferSize = width * height * 4 * 4;
    offscreenTexture->m_pImage->m_mipmapCount = 1;
    offscreenTexture->m_pImage->m_layerCount = 1;
    offscreenTexture->m_pImage->m_bFramebuffer = true;
    offscreenTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;

    offscreenTexture->m_wrapMode[0] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[1] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[2] = WrapMode::CLAMP;

    std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateVulkanTexture(offscreenTexture);


    std::shared_ptr<MyTexture> irradianceMap  = std::make_shared<MyTexture>();
    irradianceMap->m_pImage = std::make_shared<MyImage>("irradiance-map");
    irradianceMap->m_pImage->m_width = width;
    irradianceMap->m_pImage->m_height = height;
    irradianceMap->m_pImage->m_channels = 4;
    irradianceMap->m_pImage->m_bufferSize = 0;
    for (int i = 0; i < mipmapCount; ++i)
    {
        irradianceMap->m_pImage->m_bufferSize += width * height * 4 * 4 / std::max(1, int(width / pow(2, i)));
    }
    irradianceMap->m_pImage->m_bufferSize *= 6;
    irradianceMap->m_pImage->m_mipmapCount = mipmapCount;
    irradianceMap->m_pImage->m_layerCount = 6;
    irradianceMap->m_pImage->m_bFramebuffer = false;
    irradianceMap->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;
    irradianceMap->m_wrapMode[0] = WrapMode::CLAMP;
    irradianceMap->m_wrapMode[1] = WrapMode::CLAMP;
    irradianceMap->m_wrapMode[2] = WrapMode::CLAMP;
    std::shared_ptr<VulkanTexture> vulkanTexture = m_pResourceManager->CreateCombinedTexture(irradianceMap);

    vk::Device device = m_pContext->GetLogicalDevice();
    RenderPass renderPass(&device);
    vk::AttachmentDescription colorAttachment(
    {},
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    );

    renderPass.AddAttachment(colorAttachment);

    vk::RenderPass vulkanRenderPass = renderPass.Get();

    std::array<vk::ImageView, 1> attachments{};
    attachments[0] = offscreenVulkanTexture->imageView;

    vk::FramebufferCreateInfo createInfo({},
        vulkanRenderPass,
        (uint32_t)attachments.size(),
        attachments.data(),
        width,
        height,
        uint32_t(1));
    vk::Framebuffer framebuffer = device.createFramebuffer(createInfo);

    std::array<vk::ClearValue, 1> clearValues{};
    clearValues[0].color.float32[0] = 0.0;
    clearValues[0].color.float32[1] = 0.0;
    clearValues[0].color.float32[2] = 0.0;
    clearValues[0].color.float32[3] = 1.0f;

    vk::RenderPassBeginInfo renderPassInfo(
        vulkanRenderPass,
        framebuffer,
        vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)),
        (uint32_t)clearValues.size(),
        clearValues.data());

    PipelineId id;
    id.type = IRRADIANCE_MAP;
    id.model.primitivePart.info.bits.positionVertexData = 1;
    id.model.primitivePart.info.bits.normalVertexData = 0;
    id.model.primitivePart.info.bits.countTexCoord = 0;
    Pipeline pipeline(id);
    pipeline.InitIrradianceMap(m_pContext->GetLogicalDevice(), vulkanRenderPass, m_pTextureEnvMap->m_pImage->m_layerCount > 1);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo({
        m_pContext->GetCommandPool() ,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(1)
    });

    struct PushBlock {
        glm::mat4 mvp;
        float roughness;
        //uint32_t numSamples = 32u;
    } pushBlock;

    std::vector<glm::mat4> matrices = {
        // POSITIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_X
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Y
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // POSITIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        // NEGATIVE_Z
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
    vk::CommandBufferBeginInfo beginInfo({
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        nullptr
    });

    commandBuffer.begin(beginInfo);

    // transit offscreen image layout
    vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    // transit prefiltered cubemap image layout
    vk::ImageSubresourceRange ssrCubemap(vk::ImageAspectFlagBits::eColor, 0, mipmapCount, 0, 6);
    m_pResourceManager->SetImageLayout(commandBuffer, vulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, ssrCubemap,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    for (int level = 0; level < mipmapCount; ++level)
    {
        float roughness = (float)level / (float)(mipmapCount - 1);
        pushBlock.roughness = roughness;
        for (int i = 0; i < 6; ++i)
        {
            pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[i];

            vk::Viewport viewport(0, 0, width * std::pow(0.5f, level), width * std::pow(0.5f, level), 0, 1);
            vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(viewport.width, viewport.height));
            commandBuffer.setViewport(0, 1, &viewport);
            commandBuffer.setScissor(0, 1, &sissor);

            commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

            // skybox command buffer
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());
            commandBuffer.bindVertexBuffers(0, m_pRenderable->m_vertexBuffers.size(), m_pRenderable->m_vertexBuffers.data(), m_pRenderable->m_vertexBufferOffsets.data());
            commandBuffer.bindIndexBuffer(m_pRenderable->m_indexBuffer, 0, vk::IndexType::eUint16);

            commandBuffer.pushConstants(pipeline.GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(pushBlock), reinterpret_cast<void*>(&pushBlock));
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, 1, &m_dsSkybox, 0, nullptr);

            commandBuffer.drawIndexed(m_pRenderable->m_mesh->m_indexNum, 1, 0, 0, 0);

            commandBuffer.endRenderPass();

            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

            // Copy region for transfer from framebuffer to cube face
            vk::ImageCopy copyRegion(
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, level, i, 1),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(viewport.width, viewport.height, 1));

            commandBuffer.copyImage(offscreenVulkanTexture->image, vk::ImageLayout::eTransferSrcOptimal,
                vulkanTexture->image,
                vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    // transit prefiltered cubemap image layout back
    m_pResourceManager->SetImageLayout(commandBuffer, vulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, ssrCubemap,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    commandBuffer.end();

    vk::SubmitInfo submitInfo({
        {},
        {},
        {},
        (uint32_t)1,
        &commandBuffer,
        {},
        {}
    });
    m_pContext->GetDeviceQueue().submit((uint32_t)1, &submitInfo, nullptr);
    m_pContext->GetDeviceQueue().waitIdle();
    device.freeCommandBuffers(m_pContext->GetCommandPool(), 1, &commandBuffer);
    device.destroyFramebuffer(framebuffer);

    return vulkanTexture;
}

// Generate a BRDF integration map used as a look - up - table(stores roughness / NdotV)
std::shared_ptr<VulkanTexture> Skybox::generateBRDFLUT(vk::DescriptorPool &descriptorPool)
{
    uint32_t width = 512;
    uint32_t height = 512;
    uint32_t mipmapCount = static_cast<uint32_t>(floor(log2(width)) + +1);
    std::shared_ptr<MyTexture> offscreenTexture = std::make_shared<MyTexture>();
    offscreenTexture->m_pImage = std::make_shared<MyImage>("generated-brdf-lut-colorAttachment");
    offscreenTexture->m_pImage->m_width = width;
    offscreenTexture->m_pImage->m_height = height;
    offscreenTexture->m_pImage->m_channels = 4;
    offscreenTexture->m_pImage->m_bufferSize = width * height * 4 * 2;
    offscreenTexture->m_pImage->m_mipmapCount = 1;
    offscreenTexture->m_pImage->m_layerCount = 1;
    offscreenTexture->m_pImage->m_bFramebuffer = true;
    offscreenTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA16_FLOAT;

    offscreenTexture->m_wrapMode[0] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[1] = WrapMode::CLAMP;
    offscreenTexture->m_wrapMode[2] = WrapMode::CLAMP;

    std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateCombinedTexture(offscreenTexture);

    vk::Device device = m_pContext->GetLogicalDevice();
    RenderPass renderPass(&device);
    vk::AttachmentDescription colorAttachment(
        {},
        vk::Format::eR16G16B16A16Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    );

    renderPass.AddAttachment(colorAttachment);

    vk::RenderPass vulkanRenderPass = renderPass.Get();

    std::array<vk::ImageView, 1> attachments{};
    attachments[0] = offscreenVulkanTexture->imageView;

    vk::FramebufferCreateInfo createInfo({},
        vulkanRenderPass,
        (uint32_t)attachments.size(),
        attachments.data(),
        width,
        height,
        uint32_t(1));
    vk::Framebuffer framebuffer = device.createFramebuffer(createInfo);

    PipelineId id;
    id.type = GENERATE_BRDF_LUT;
    id.model.primitivePart.info.bits.positionVertexData = 0;
    id.model.primitivePart.info.bits.normalVertexData = 0;
    id.model.primitivePart.info.bits.countTexCoord = 0;
    Pipeline pipeline(id);
    pipeline.InitGenerateBrdfLut(m_pContext->GetLogicalDevice(), vulkanRenderPass);

    std::array<vk::ClearValue, 1> clearValues{};
    clearValues[0].color.float32[0] = 0.0;
    clearValues[0].color.float32[1] = 0.0;
    clearValues[0].color.float32[2] = 0.0;
    clearValues[0].color.float32[3] = 1.0f;

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo({
        m_pContext->GetCommandPool() ,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(1)
    });
    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
    vk::CommandBufferBeginInfo beginInfo({
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        nullptr
    });

    commandBuffer.begin(beginInfo);

    // transit offscreen image layout
    vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR16G16B16A16Sfloat, srrOffscreen,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    vk::Extent2D brdfLutExtent(512, 512);
    vk::Viewport viewport(0, 0, brdfLutExtent.width, brdfLutExtent.height, 0, 1);
    vk::Rect2D sissor(vk::Offset2D(0, 0), brdfLutExtent);
    commandBuffer.setViewport(0, 1, &viewport);
    commandBuffer.setScissor(0, 1, &sissor);

    vk::RenderPassBeginInfo renderPassInfo(
        vulkanRenderPass,
        framebuffer,
        vk::Rect2D(vk::Offset2D(0, 0), brdfLutExtent),
        (uint32_t)clearValues.size(),
        clearValues.data());
    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());
    commandBuffer.draw(3, 1, 0, 0);
    commandBuffer.endRenderPass();

    m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR16G16B16A16Sfloat, srrOffscreen,
        vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    commandBuffer.end();

    vk::SubmitInfo submitInfo({
        {},
        {},
        {},
        (uint32_t)1,
        &commandBuffer,
        {},
        {}
    });
    m_pContext->GetDeviceQueue().submit((uint32_t)1, &submitInfo, nullptr);
    m_pContext->GetDeviceQueue().waitIdle();
    device.freeCommandBuffers(m_pContext->GetCommandPool(), 1, &commandBuffer);
    device.destroyFramebuffer(framebuffer);

    return offscreenVulkanTexture;
}

void Skybox::initSHLight()
{
    if (m_bCubemap)
    {
	    uint32_t width = m_pTextureEnvMap->m_pImage->m_width;
	    uint32_t height = m_pTextureEnvMap->m_pImage->m_height;
        uint32_t mipmapCount = static_cast<uint32_t>(floor(log2(width)) + +1);

        std::vector<std::shared_ptr<MyTexture>> textures;
        std::vector<std::shared_ptr<VulkanTexture>> vulkanTextures;

        vk::Device device = m_pContext->GetLogicalDevice();

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo({m_pContext->GetCommandPool(),
                                                                 vk::CommandBufferLevel::ePrimary,
                                                                 static_cast<uint32_t>(1)});

        vk::CommandBuffer commandBuffer;
        device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
        vk::CommandBufferBeginInfo beginInfo({vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
                                              nullptr});

        commandBuffer.begin(beginInfo);

        vk::Format format = vk::Format::eR16G16B16A16Sfloat;
        if (m_pTextureEnvMap->m_pImage->m_format == MyImageFormat::MY_IMAGEFORMAT_RGBA32_FLOAT)
        {
            format = vk::Format::eR32G32B32A32Sfloat;
        }
        else if (m_pTextureEnvMap->m_pImage->m_format == MyImageFormat::MY_IMAGEFORMAT_RGB32_FLOAT)
        {
            format = vk::Format::eR32G32B32Sfloat;
        }
        vk::ImageSubresourceRange srrEnvMap(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
        m_pResourceManager->SetImageLayout(commandBuffer, m_pVulkanTextureEnvMap->image, format, srrEnvMap,
                                           vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);

        for (int i = 0; i < 6; ++i)
        {
            std::shared_ptr<MyTexture> offscreenTexture = std::make_shared<MyTexture>();

            char name[32];
            std::sprintf(name, "cubemap-%d", i);
            offscreenTexture->m_pImage = std::make_shared<MyImage>(name);
            offscreenTexture->m_pImage->m_width = width;
            offscreenTexture->m_pImage->m_height = height;
            offscreenTexture->m_pImage->m_channels = 4;
            offscreenTexture->m_pImage->m_bufferSize = width * height * 4 * 4;
            offscreenTexture->m_pImage->m_mipmapCount = 1;
            offscreenTexture->m_pImage->m_layerCount = 1;
            offscreenTexture->m_pImage->m_format = m_pTextureEnvMap->m_pImage->m_format;
            offscreenTexture->m_pImage->m_bHostVisible = true;
            offscreenTexture->m_pImage->m_bTransferSrc = true;

            offscreenTexture->m_wrapMode[0] = WrapMode::CLAMP;
            offscreenTexture->m_wrapMode[1] = WrapMode::CLAMP;
            offscreenTexture->m_wrapMode[2] = WrapMode::CLAMP;

            std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateVulkanTexture(offscreenTexture);
            // transit offscreen image layout
            vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, format, srrOffscreen,
                                               vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

            // Copy region for transfer from framebuffer to cube face
            vk::ImageCopy copyRegion(
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, i, 1),
                vk::Offset3D(0, 0, 0),
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(width, height, 1));

            commandBuffer.copyImage(m_pVulkanTextureEnvMap->image, vk::ImageLayout::eTransferSrcOptimal,
                                    offscreenVulkanTexture->image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

            // m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR16G16B16A16Sfloat, srrOffscreen,
            //	vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal);

            textures.push_back(offscreenTexture);
            vulkanTextures.push_back(offscreenVulkanTexture);
        }

        m_pResourceManager->SetImageLayout(commandBuffer, m_pVulkanTextureEnvMap->image, format, srrEnvMap,
                                           vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        commandBuffer.end();

        vk::SubmitInfo submitInfo({{},
                                   {},
                                   {},
                                   (uint32_t)1,
                                   &commandBuffer,
                                   {},
                                   {}});
        m_pContext->GetDeviceQueue().submit((uint32_t)1, &submitInfo, nullptr);
        m_pContext->GetDeviceQueue().waitIdle();
        device.freeCommandBuffers(m_pContext->GetCommandPool(), 1, &commandBuffer);

        for (int i = 0; i < textures.size(); ++i)
        {
            m_pResourceManager->TransferGPUTextureToCPU(vulkanTextures[i], textures[i]);

            char name[32];
            sprintf(name, "cubemap-%d.hdr", i);
            // textures[i]->m_pImage->DumpImageHDR(name);
        }

        std::vector<std::shared_ptr<MyTexture>> _textures;
        _textures.push_back(textures[0]); // +X
        _textures.push_back(textures[1]); // -X
        _textures.push_back(textures[4]); // +Z
        _textures.push_back(textures[5]); // -Z
        _textures.push_back(textures[2]); // +Y
        _textures.push_back(textures[3]); // -Y

        m_pSHLight = std::make_shared<SHLight>(m_pResourceManager, textures);
    }
    else
    {
        uint32_t width = PREFILTERED_IMAGE_WIDTH;
        uint32_t height = PREFILTERED_IMAGE_HEIGHT;
        uint32_t mipmapCount = static_cast<uint32_t>(floor(log2(width)) + +1);

        vk::Device device = m_pContext->GetLogicalDevice();
        RenderPass renderPass(&device);
        vk::AttachmentDescription colorAttachment(
            {},
            vk::Format::eR32G32B32A32Sfloat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal);

        renderPass.AddAttachment(colorAttachment);

        vk::RenderPass vulkanRenderPass = renderPass.Get();

        struct PushBlock
        {
            glm::mat4 mvp;
        } pushBlock;

        std::vector<glm::mat4> matrices = {
            // POSITIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

    

        // transit offscreen image layout
        //vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        //m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR16G16B16A16Sfloat, srrOffscreen,
        //                                   vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);


        std::vector<std::shared_ptr<MyTexture>> _textures;
        std::vector<std::shared_ptr<MyTexture>> textures;
        std::vector<std::shared_ptr<VulkanTexture>> vulkanTextures;
        for (int i = 0; i < 6; ++i)
        {
            vk::CommandBufferAllocateInfo commandBufferAllocateInfo({ m_pContext->GetCommandPool(),
                                                                    vk::CommandBufferLevel::ePrimary,
                                                                    static_cast<uint32_t>(1) });

            vk::CommandBuffer commandBuffer;
            device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
            vk::CommandBufferBeginInfo beginInfo({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
                                                  nullptr });

            commandBuffer.begin(beginInfo);

            std::shared_ptr<MyTexture> offscreenTexture = std::make_shared<MyTexture>();
            offscreenTexture->m_pImage = std::make_shared<MyImage>("env-map-blit-colorAttachment");
            offscreenTexture->m_pImage->m_width = width;
            offscreenTexture->m_pImage->m_height = height;
            offscreenTexture->m_pImage->m_channels = 4;
            offscreenTexture->m_pImage->m_bufferSize = width * height * 4 * 4;
            offscreenTexture->m_pImage->m_mipmapCount = 1;
            offscreenTexture->m_pImage->m_layerCount = 1;
            offscreenTexture->m_pImage->m_bFramebuffer = true;
            offscreenTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;

            offscreenTexture->m_wrapMode[0] = WrapMode::CLAMP;
            offscreenTexture->m_wrapMode[1] = WrapMode::CLAMP;
            offscreenTexture->m_wrapMode[2] = WrapMode::CLAMP;

    
            std::shared_ptr<VulkanTexture> offscreenVulkanTexture = m_pResourceManager->CreateVulkanTexture(offscreenTexture);

            std::shared_ptr<MyTexture> tempTexture = std::make_shared<MyTexture>();
			char imageName[32];
			sprintf(imageName, "TempImageForSH_%d", i);
            tempTexture->m_pImage = std::make_shared<MyImage>(imageName);
            tempTexture->m_pImage->m_width = width;
            tempTexture->m_pImage->m_height = height;
            tempTexture->m_pImage->m_channels = 4;
            tempTexture->m_pImage->m_bufferSize = width * height * 4 * 4;
            tempTexture->m_pImage->m_mipmapCount = 1;
            tempTexture->m_pImage->m_layerCount = 1;
            tempTexture->m_pImage->m_bFramebuffer = false;
            tempTexture->m_pImage->m_format = MY_IMAGEFORMAT_RGBA32_FLOAT;
            tempTexture->m_pImage->m_bHostVisible = true;

            tempTexture->m_wrapMode[0] = WrapMode::CLAMP;
            tempTexture->m_wrapMode[1] = WrapMode::CLAMP;
            tempTexture->m_wrapMode[2] = WrapMode::CLAMP;


            textures.push_back(tempTexture);

            std::shared_ptr<VulkanTexture> tempVulkanTexture = m_pResourceManager->CreateVulkanTexture(tempTexture);

            vulkanTextures.push_back(tempVulkanTexture);


            vk::ImageSubresourceRange srrOffscreen(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
            m_pResourceManager->SetImageLayout(commandBuffer, tempVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);


            std::array<vk::ImageView, 1> attachments{};
            attachments[0] = offscreenVulkanTexture->imageView;

            vk::FramebufferCreateInfo createInfo({},
                vulkanRenderPass,
                (uint32_t)attachments.size(),
                attachments.data(),
                width,
                height,
                uint32_t(1));
            vk::Framebuffer framebuffer = device.createFramebuffer(createInfo);

            std::array<vk::ClearValue, 1> clearValues{};
            clearValues[0].color.float32[0] = 0.0;
            clearValues[0].color.float32[1] = 0.0;
            clearValues[0].color.float32[2] = 0.0;
            clearValues[0].color.float32[3] = 1.0f;

            vk::RenderPassBeginInfo renderPassInfo(
                vulkanRenderPass,
                framebuffer,
                vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)),
                (uint32_t)clearValues.size(),
                clearValues.data());

            PipelineId id;
            id.type = BLIT_ENV_MAP;
            id.model.primitivePart.info.bits.positionVertexData = 1;
            id.model.primitivePart.info.bits.normalVertexData = 0;
            id.model.primitivePart.info.bits.countTexCoord = 0;
            id.model.primitivePart.info.bits.primitiveMode = PrimitiveMode::Triangles;
            Pipeline pipeline(id);
            pipeline.InitBlitEnvMap(m_pContext->GetLogicalDevice(), vulkanRenderPass);


            pushBlock.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[i];

            vk::Viewport viewport(0, 0, width, height, 0, 1);
            vk::Rect2D sissor(vk::Offset2D(0, 0), vk::Extent2D(viewport.width, viewport.height));
            commandBuffer.setViewport(0, 1, &viewport);
            commandBuffer.setScissor(0, 1, &sissor);

            commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

            // skybox command buffer
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipeline());
            commandBuffer.bindVertexBuffers(0, m_pRenderable->m_vertexBuffers.size(), m_pRenderable->m_vertexBuffers.data(), m_pRenderable->m_vertexBufferOffsets.data());
            commandBuffer.bindIndexBuffer(m_pRenderable->m_indexBuffer, 0, vk::IndexType::eUint16);

            commandBuffer.pushConstants(pipeline.GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(pushBlock), reinterpret_cast<void *>(&pushBlock));
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineLayout(), 0, 1, &m_dsSkybox, 0, nullptr);

            commandBuffer.drawIndexed(m_pRenderable->m_mesh->m_indexNum, 1, 0, 0, 0);

            commandBuffer.endRenderPass();


            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

            // Copy region for transfer from framebuffer to cube face
            vk::ImageCopy copyRegion(
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(viewport.width, viewport.height, 1));

            commandBuffer.copyImage(offscreenVulkanTexture->image, vk::ImageLayout::eTransferSrcOptimal,
                tempVulkanTexture->image,
                vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

            m_pResourceManager->SetImageLayout(commandBuffer, offscreenVulkanTexture->image, vk::Format::eR32G32B32A32Sfloat, srrOffscreen,
                vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal);


            commandBuffer.end();

            vk::SubmitInfo submitInfo({ {},
                                       {},
                                       {},
                                       (uint32_t)1,
                                       &commandBuffer,
                                       {},
                                       {} });
            m_pContext->GetDeviceQueue().submit((uint32_t)1, &submitInfo, nullptr);
            m_pContext->GetDeviceQueue().waitIdle();
            device.freeCommandBuffers(m_pContext->GetCommandPool(), 1, &commandBuffer);
            device.destroyFramebuffer(framebuffer);
        }

        for (int i = 0; i < textures.size(); ++i)
        {
            m_pResourceManager->TransferGPUTextureToCPU(vulkanTextures[i], textures[i]);

            char name[32];
            sprintf(name, "cubemap-%d.hdr", i);
             textures[i]->m_pImage->DumpImageHDR(name);
        }

        _textures.push_back(textures[0]); // +X
        _textures.push_back(textures[1]); // -X
        _textures.push_back(textures[4]); // +Z
        _textures.push_back(textures[5]); // -Z
        _textures.push_back(textures[2]); // +Y
        _textures.push_back(textures[3]); // -Y

        m_pSHLight = std::make_shared<SHLight>(m_pResourceManager, textures);
    }
}
