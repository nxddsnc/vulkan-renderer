#include "Platform.h"
struct aiNode;
struct MyNode
{
	struct KeyVector
	{
		glm::vec3 value;
		double time;
	};
	struct KeyQuat
	{
		glm::quat value;
		double time;
	};
	aiNode							   * node;
	std::shared_ptr<MyNode>				 parent;
	std::vector<std::shared_ptr<MyNode>> children;
	int									 jointIndex;
	std::vector<KeyVector>				 keyPositions;
	std::vector<KeyVector>				 keyScalings;
	std::vector<KeyQuat>				 keyRotations;
	glm::mat4							 jointMatrix = glm::mat4(1.0);
	glm::mat4							 inverseTransformMatrix = glm::mat4(1.0);
};

struct KeyFrame
{
	double time;
	glm::vec3 keyPosition;
	glm::vec3 keyScaling;
	glm::quat keyRotation;
};

class MyAnimation
{
public:
	MyAnimation(double duration);
	~MyAnimation();

	void SetRoot(std::shared_ptr<MyNode> root);
	void Update();
	void InitUniformBuffer(VmaAllocator * memoryAllocator);
	void UpdateUniformbuffer();
	void CreateDescriptorSet(vk::Device & device, vk::DescriptorPool & descriptorPool);
private:
	void _traverseNodes(std::shared_ptr<MyNode> parent);
	void _traverseUpdate(std::shared_ptr<MyNode> parent);
	glm::mat4 _getCurrentMatrix(std::shared_ptr<MyNode> node);

public:
        vk::DescriptorSet           m_descriptorSet;

private:
	std::vector<std::shared_ptr<MyNode>>    m_boneNodes;
	double									m_duration;
	double									m_currentTime;

	vk::Buffer                  m_uniformBuffer;
	VmaAllocation               m_uniformBufferMemory;
private:
	std::shared_ptr<MyNode>			        m_pRoot;

	std::vector<glm::mat4>					m_JointMatrices;

	VmaAllocator						*   m_pMemoryAllocator;

	int										m_speed;

	std::vector<KeyFrame>					m_keyFrames;
};