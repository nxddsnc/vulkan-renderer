#include "MyAnimation.h"
#include "MyMesh.h"
#include <glm/gtx/quaternion.hpp>

MyAnimation::MyAnimation(double duration)
{
	m_currentTime = 0;
	m_duration = duration;
	m_speed = 1;
}

MyAnimation::~MyAnimation()
{
}

void MyAnimation::SetRoot(std::shared_ptr<MyNode> root)
{
	m_pRoot = root;

	_traverseNodes(root);

	assert(m_boneNodes.size() <= 64);
	m_JointMatrices.resize(64);
}

void MyAnimation::Update()
{
	m_currentTime += 1.0 * m_speed;
	_traverseUpdate(m_pRoot);
	for (int i = 0; i < m_boneNodes.size(); ++i)
	{
		m_JointMatrices[i] = m_boneNodes[i]->jointMatrix * m_boneNodes[i]->inverseTransformMatrix;
	}
}

void MyAnimation::UpdateUniformbuffer()
{
	void* data;
	vmaMapMemory(*m_pMemoryAllocator, m_uniformBufferMemory, &data);
	memcpy(data, m_JointMatrices.data(), sizeof(glm::mat4) * m_JointMatrices.size());
	vmaUnmapMemory(*m_pMemoryAllocator, m_uniformBufferMemory);
}

void MyAnimation::CreateDescriptorSet(vk::Device & device, vk::DescriptorPool & descriptorPool)
{
	vk::DescriptorSetLayout descriptorSetLayout;
	// camera uniform buffer
	vk::DescriptorSetLayoutBinding uniformBinding({ 0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		{} });

	vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
		1,
		&uniformBinding });
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
		sizeof(glm::mat4) * m_JointMatrices.size()
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

void MyAnimation::InitUniformBuffer(VmaAllocator * memoryAllocator)
{
	m_pMemoryAllocator = memoryAllocator;

	vk::BufferCreateInfo createInfo({ {},
		vk::DeviceSize(sizeof(glm::mat4) * m_JointMatrices.size()),
		vk::BufferUsageFlagBits::eUniformBuffer,
		{},
		{},
		{} });
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	VkBufferCreateInfo &vkCreateInfo = createInfo;
	vmaCreateBuffer(*memoryAllocator, &vkCreateInfo, &allocInfo, reinterpret_cast<VkBuffer*>(&m_uniformBuffer), &m_uniformBufferMemory, nullptr);
}

void MyAnimation::_traverseNodes(std::shared_ptr<MyNode> parent)
{
	parent->jointIndex = m_boneNodes.size();
	m_boneNodes.push_back(parent);
	for (auto child : parent->children)
	{
		_traverseNodes(child);
	}
}

void MyAnimation::_traverseUpdate(std::shared_ptr<MyNode> node)
{
	node->jointMatrix = _getCurrentMatrix(node);
	if (node->parent)
	{
		node->jointMatrix = node->parent->jointMatrix * node->jointMatrix;
	}
	for (int i = 0; i < node->children.size(); ++i)
	{
		_traverseUpdate(node->children[i]);
	}
}

glm::mat4 MyAnimation::_getCurrentMatrix(std::shared_ptr<MyNode> node)
{
	glm::vec3 pre(0), next(0);
	glm::vec3 currentPos(0);
	glm::vec3 currentScaling(1);
	glm::quat currentRotation(0, 0, 0, 0);
	float ratio;
	if (node->keyPositions.size() != 0)
	{
		pre = node->keyPositions[0].value;
		for (int i = 1; i < node->keyPositions.size() - 1; i++)
		{
			if (m_currentTime > node->keyPositions[i].time && m_currentTime <= node->keyPositions[i + 1].time)
			{
				pre = node->keyPositions[i].value;
				next = node->keyPositions[i + 1].value;
				ratio = (m_currentTime - node->keyPositions[i].time) / (node->keyPositions[i + 1].time - node->keyPositions[i].time);
				currentPos = pre * (1 - ratio) + next * ratio;
				break;
			}
		}
	}

	if (node->keyScalings.size() != 0)
	{
		for (int i = 1; i < node->keyScalings.size() - 1; i++)
		{
			pre = node->keyScalings[0].value;
			if (m_currentTime > node->keyScalings[i].time && m_currentTime <= node->keyScalings[i + 1].time)
			{
				pre = node->keyScalings[i].value;
				next = node->keyScalings[i + 1].value;
				ratio = (m_currentTime - node->keyScalings[i].time) / (node->keyScalings[i + 1].time - node->keyScalings[i].time);
				currentScaling = pre * (1 - ratio) + next * ratio;
				break;
			}
		}
	}

	if (node->keyRotations.size() != 0)
	{
		for (int i = 1; i < node->keyRotations.size() - 1; i++)
		{
			if (m_currentTime > node->keyRotations[i].time && m_currentTime <= node->keyRotations[i + 1].time)
			{
				ratio = (m_currentTime - node->keyRotations[i].time) / (node->keyRotations[i + 1].time - node->keyRotations[i].time);
				currentRotation = glm::slerp(node->keyRotations[i].value, node->keyRotations[i + 1].value, ratio);
				break;
			}
		}
	}

    return glm::translate(glm::toMat4(currentRotation) * glm::scale(glm::mat4(1.0), currentScaling), currentPos);
}
