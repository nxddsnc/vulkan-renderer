#include "Platform.h"
#include <unordered_map>

struct aiNode;
struct MyNode
{
	aiNode							   * node;
	std::shared_ptr<MyNode>				 parent;
	std::vector<std::shared_ptr<MyNode>> children;
	int									 jointIndex;
    glm::mat4                            jointMatrix;
	glm::mat4							 inverseTransformMatrix = glm::mat4(1.0);
};

struct NodePose
{
    glm::vec3 keyPosition;
    glm::vec3 keyScaling;
    glm::quat keyRotation;
};

struct KeyFrame
{
	double time;
    std::unordered_map<std::shared_ptr<MyNode>, std::shared_ptr<NodePose>> nodePose;
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
    vk::DescriptorSet                       m_descriptorSet;
    std::vector<std::shared_ptr<KeyFrame>>				m_keyFrames;
private:
	std::vector<std::shared_ptr<MyNode>>    m_boneNodes;
	double									m_duration;
	double									m_currentTime;
    std::unordered_map<std::shared_ptr<MyNode>, glm::mat4> m_nodePoses;

    std::shared_ptr<KeyFrame>               m_preFrame;
    std::shared_ptr<KeyFrame>               m_nextFrame;

	vk::Buffer                              m_uniformBuffer;
	VmaAllocation                           m_uniformBufferMemory;
private:
	std::shared_ptr<MyNode>			        m_pRoot;

	std::vector<glm::mat4>					m_JointMatrices;

	VmaAllocator						*   m_pMemoryAllocator;

	int										m_speed;
};