#include "Platform.h"
#include "Utils.h"
#pragma once 

class MyCamera
{
public:
	MyCamera(VmaAllocator *memoryAllocator, bool orthogonal = false);
	~MyCamera();

	void UpdateViewMatrix(float width, float height);
	
	void SetPerspective(float fov, float aspect, float znear, float zfar);

	void UpdateAspectRatio(float aspect);

	void UpdateBBox(BBox &bbox);

	void Translate(float x, float y);

	void Rotate(float x, float y);

    void MoveForward(float dis);

	void CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool);

	void Update(float width, float height);
public:
	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} m_keys;

    VmaAllocator *_memoryAllocator;

	enum CameraType { lookat, firstperson };
	CameraType m_type = CameraType::lookat;

    float m_fov;
    float m_znear, m_zfar;
    float m_aspectRatio;

    BBox  m_bbox;

	glm::vec3 m_rotation;
	glm::vec3 m_eye; // position of camera
    glm::vec3 m_at; // position that the eye lookat
    float m_alpha; // horizontal angle
    float m_beta;  // vertical angle
    float m_radius;
   
    glm::vec3 m_rotationTarget;
    glm::vec3 m_atTarget; 
    float m_alphaTarget; 
    float m_betaTarget;  
    float m_radiusTarget;

	float m_rotationSpeed;
	float m_movementSpeed;
	float m_inertia;

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