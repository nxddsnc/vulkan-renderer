#include "Platform.h"
#include "MyCamera.h"
#include <cmath>
#define PI 3.14159
#define EPS 0.000001f
MyCamera::MyCamera(VmaAllocator * memoryAllocator, bool orthogonal)
{
	m_rotationSpeed = 0.001f;
	m_movementSpeed = 2.0f;
    
    m_alpha = 0.0f;
    m_beta  = 0.0f;
    m_alphaTarget = 0.0f;
    m_betaTarget = 0.0f;

	m_znear = 0.01f;

    m_eye = glm::vec3(0.0, 0.0, 1.0);
    m_at  = glm::vec3(0.0, 0.0, 0.0);
	
    m_radiusTarget = 1.0;
    m_radius = 1.0;

    m_fov = 45.0;
    m_aspectRatio = 1.0;

    m_updated = false;

	m_bOrtho = orthogonal;

	_memoryAllocator = memoryAllocator;

	vk::BufferCreateInfo createInfo({ {},
        vk::DeviceSize(sizeof(m_matrices) + sizeof(m_eye)),
		vk::BufferUsageFlagBits::eUniformBuffer,
		{},
		{},
		{} });
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VkBufferCreateInfo &vkCreateInfo = createInfo;
	vmaCreateBuffer(*_memoryAllocator, &vkCreateInfo, &allocInfo, reinterpret_cast<VkBuffer*>(&m_uniformBuffer), &m_uniformBufferMemory, nullptr);
}

MyCamera::~MyCamera()
{
	vmaDestroyBuffer(*_memoryAllocator, m_uniformBuffer, m_uniformBufferMemory);
}

void MyCamera::UpdateViewMatrix(float width, float height)
{
    glm::vec3 diffAt = m_atTarget - m_at;
    float diffRadius = m_radiusTarget - m_radius;
    float diffAlpha = m_alphaTarget - m_alpha;
    float diffBeta = m_betaTarget - m_beta;

    if (diffAt.length() > EPS || diffRadius > EPS || diffAlpha > EPS || diffBeta > EPS)
    {
        m_at += diffAt * 0.1f;
        m_radius += diffRadius * 0.1f;
        m_alpha += diffAlpha * 0.1f;
        m_beta += diffBeta * 0.1f;

        float x = std::cos(m_beta) * std::sin(m_alpha);
        float y = std::cos(m_beta) * std::cos(m_alpha);
        float z = std::sin(m_beta);

        m_eye = m_at + glm::vec3(x, y, z) * m_radius;
    }

    m_matrices.view = glm::lookAtRH(m_eye, m_at, glm::vec3(0, 0, 1));


    float length = glm::length(m_bbox.max - m_bbox.min);
    float farPlane = glm::length(m_eye - (m_bbox.max + m_bbox.min) * 0.5f) + glm::length(m_bbox.max - m_bbox.min) / 2;

	m_zfar = farPlane;

    SetPerspective(m_fov, width / height, m_znear, m_zfar);

	m_updated = true;
}

void MyCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	m_fov   = fov;
	m_znear = znear;
	m_zfar  = zfar;

	if (m_bOrtho)
	{
		BBox box(m_bbox);
		ApplyMatrixToBBox(m_matrices.view, box);
		m_matrices.perspective = glm::ortho(box.min.x, box.max.x, box.min.y, box.max.y, m_znear, m_zfar);
	}
	else
	{
		m_matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}
}

void MyCamera::UpdateAspectRatio(float aspect)
{
	m_matrices.perspective = glm::perspective(glm::radians(m_fov), aspect, m_znear, m_zfar);
}

void MyCamera::UpdateBBox(BBox &bbox)
{
    m_atTarget = (bbox.max + bbox.min) * 0.5f;
    m_radiusTarget   = glm::length(bbox.max - bbox.min);
    m_bbox = bbox;
}

void MyCamera::Translate(float x, float y)
{
    glm::mat3 rotationMatrix = glm::mat3(m_matrices.view);
    glm::mat3 invert = glm::inverse(rotationMatrix);
    glm::vec3 dir = glm::normalize(invert * glm::vec3(x, -y, 0));
    m_atTarget  += dir * m_movementSpeed;
}

void MyCamera::Rotate(float x, float y)
{
    m_alphaTarget += x * m_rotationSpeed;
    m_betaTarget  += y * m_rotationSpeed;
    if (m_betaTarget > PI * 0.5)
    {
        m_betaTarget = 0.499 * PI;
    }
    if (m_betaTarget < - PI * 0.5)
    {
        m_betaTarget = - 0.499 * PI;
    }
}

void MyCamera::MoveForward(float dis)
{
    m_atTarget += glm::normalize(m_at - m_eye) * dis * m_movementSpeed;
}

void MyCamera::CreateDescriptorSet(vk::Device & device, vk::DescriptorPool & descriptorPool)
{
	vk::DescriptorSetLayout descriptorSetLayout;
	// camera uniform buffer
	vk::DescriptorSetLayoutBinding cameraBinding({ 0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		{} });

	vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
		1,
		&cameraBinding });
	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

	vk::DescriptorSetAllocateInfo allocInfo({
		descriptorPool,
		1,
		&descriptorSetLayout
	});
	device.allocateDescriptorSets(&allocInfo, &m_descriptorSet);

	vk::DescriptorBufferInfo bufferInfo({
		m_uniformBuffer,
		0,
		sizeof(m_matrices) + sizeof(glm::vec3)
	});

	vk::WriteDescriptorSet descriptorWrite({
		m_descriptorSet,
		0,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		{},
		&bufferInfo,
		{}
	});
	device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

	device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void MyCamera::Update(float width, float height)
{
    UpdateViewMatrix(width, height);

	glm::vec3 cameraPos;
	glm::mat4 mat = glm::inverse(m_matrices.view);
	cameraPos.x = mat[3][0];
	cameraPos.y = mat[3][1];
	cameraPos.z = mat[3][2];

	m_matrices.perspective[1][1] *= -1;
	void* data;
	vmaMapMemory(*_memoryAllocator, m_uniformBufferMemory, &data);
	memcpy(data, &m_matrices, sizeof(m_matrices));
	memcpy(reinterpret_cast<char*>(data) + sizeof(m_matrices), &cameraPos, sizeof(cameraPos));
	vmaUnmapMemory(*_memoryAllocator, m_uniformBufferMemory);
	m_matrices.perspective[1][1] *= -1;
}

bool MyCamera::moving()
{
	return m_keys.left || m_keys.right || m_keys.up || m_keys.down;
}


