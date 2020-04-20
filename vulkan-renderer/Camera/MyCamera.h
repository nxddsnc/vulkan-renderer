#include "Platform.h"
#include "Utils.h"
#pragma once 

class MyCamera
{
public:
	MyCamera(VmaAllocator *memoryAllocator, bool orthogonal = false);
	~MyCamera();

	void UpdateViewMatrix();
	
	void SetPerspective(float fov, float aspect, float znear, float zfar);

	void UpdateAspectRatio(float aspect);

	void SetPosition(glm::vec3 position);

	void LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

	void SetRotation(glm::vec3 rotation);

	void Update(BBox &bbox);

	void SetTranslation(glm::vec3 translation);

	void Translate(glm::vec3 delta);

	void Rotate(glm::vec3 delta);

	void CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool);

	void UpdateUniformBuffer();
public:
	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} m_keys;

    float m_fov;
    float m_znear, m_zfar;
    VmaAllocator *_memoryAllocator;

	enum CameraType { lookat, firstperson };
	CameraType m_type = CameraType::lookat;


	glm::vec3 m_rotation;
	glm::vec3 m_position;

	float m_rotationSpeed;
	float m_movementSpeed;

	bool m_updated;

	bool m_bOrtho;

	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} m_matrices;

	vk::Buffer                  m_uniformBuffer;
	VmaAllocation               m_uniformBufferMemory;
	vk::DescriptorSet           m_descriptorSet;

	bool moving();

};