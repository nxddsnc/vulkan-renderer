#include "Platform.h"
#include "MyCamera.h"

MyCamera::MyCamera(VmaAllocator * memoryAllocator, bool orthogonal)
{
	m_rotationSpeed = 1.0f;
	m_movementSpeed = 1.0f;
	m_updated = false;
	m_bOrtho = orthogonal;

	_memoryAllocator = memoryAllocator;

	vk::BufferCreateInfo createInfo({ {},
		vk::DeviceSize(sizeof(m_matrices) + sizeof(m_position)),
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

void MyCamera::UpdateViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	transM = glm::translate(glm::mat4(1.0f), m_position);

	if (m_type == CameraType::firstperson)
	{
		m_matrices.view = rotM * transM;
	}
	else
	{
		m_matrices.view = transM * rotM;
	}

	m_updated = true;
}

void MyCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	m_fov   = fov;
	m_znear = znear;
	m_zfar  = zfar;


	m_matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void MyCamera::UpdateAspectRatio(float aspect)
{
	m_matrices.perspective = glm::perspective(glm::radians(m_fov), aspect, m_znear, m_zfar);
}

void MyCamera::SetPosition(glm::vec3 position)
{
	m_position = position;

	UpdateViewMatrix();
}

void MyCamera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
	m_matrices.view = glm::lookAtRH(eye, center, up);
}

void MyCamera::SetRotation(glm::vec3 rotation)
{
	m_rotation = rotation;
	UpdateViewMatrix();
}

void MyCamera::Update(BBox &bbox)
{
	glm::vec3 boxVerts[8] = { glm::vec3(bbox.max.x, bbox.max.y, bbox.min.z), glm::vec3(bbox.max.x, bbox.min.y, bbox.min.z),
		glm::vec3(bbox.min.x, bbox.max.y, bbox.min.z), glm::vec3(bbox.min.x, bbox.min.y, bbox.min.z),
		glm::vec3(bbox.max.x, bbox.max.y, bbox.max.z), glm::vec3(bbox.max.x, bbox.min.y, bbox.max.z),
		glm::vec3(bbox.min.x, bbox.max.y, bbox.max.z), glm::vec3(bbox.min.x, bbox.min.y, bbox.max.z) };

	BBox newBox;
	newBox.min = glm::vec3(INFINITE, INFINITE, INFINITE);
	newBox.max = -glm::vec3(INFINITE, INFINITE, INFINITE);
	for (int i = 0; i < 8; ++i)
	{
		glm::vec4 vec = m_matrices.view * glm::vec4(boxVerts[i].x, boxVerts[i].y, boxVerts[i].z, 1);
		if (newBox.min.x > vec.x)
		{
			newBox.min.x = vec.x;
		}
		if (newBox.min.y > vec.y)
		{
			newBox.min.y = vec.y;
		}
		if (newBox.min.z > vec.z)
		{
			newBox.min.z = vec.z;
		}

		if (newBox.max.x < vec.x)
		{
			newBox.max.x = vec.x;
		}
		if (newBox.max.y < vec.y)
		{
			newBox.max.y = vec.y;
		}
		if (newBox.max.z < vec.z)
		{
			newBox.max.z = vec.z;
		}
	}

	float left = -(newBox.max.x - newBox.min.x) / 2;
	float right = -left;
	float top = (newBox.max.y - newBox.min.y) / 2;
	float bottom = -(newBox.max.y - newBox.min.y) / 2;

	m_znear = std::max(newBox.max.z, 0.1f);
	m_zfar = -newBox.min.z;

	m_matrices.perspective = glm::ortho(left, right, bottom, top, m_znear, m_zfar);

	//m_matrices.perspective = glm::perspective(glm::radians(45.0f), 1.6f, 0.1f, 194.0f);
}

void MyCamera::SetTranslation(glm::vec3 translation)
{
	m_position = translation;
	UpdateViewMatrix();
}

void MyCamera::Translate(glm::vec3 delta)
{
	m_position += delta;
	UpdateViewMatrix();
}

void MyCamera::Rotate(glm::vec3 delta)
{
	m_rotation += delta;
	UpdateViewMatrix();
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

void MyCamera::UpdateUniformBuffer()
{
	glm::vec3 cameraPos;
	glm::mat4 mat = glm::inverse(m_matrices.view);
	cameraPos.x = mat[3][0];
	cameraPos.y = mat[3][1];
	cameraPos.z = mat[3][2];

	//if (m_bOrtho)
	//{
	//	//m_matrices.perspective = glm::ortho(-1, 1, -1, 1, znear, zfar);
	//}

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


