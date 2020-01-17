#include "Platform.h"
#pragma once 

class VulkanCamera
{
private:
	float fov;
	float znear, zfar;
    VmaAllocator *_memoryAllocator;
	void updateViewMatrix()
	{
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		transM = glm::translate(glm::mat4(1.0f), position);

		if (type == CameraType::firstperson)
		{
			matrices.view = rotM * transM;
		}
		else
		{
			matrices.view = transM * rotM;
		}

		updated = true;
	};
public:
	VulkanCamera(VmaAllocator *memoryAllocator) 
	{
        _memoryAllocator = memoryAllocator;

		vk::BufferCreateInfo createInfo({{},
                                         vk::DeviceSize(sizeof(matrices)),
                                         vk::BufferUsageFlagBits::eUniformBuffer,
                                         {},
                                         {},
                                         {}});
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        VkBufferCreateInfo &vkCreateInfo = createInfo;
		vmaCreateBuffer(*_memoryAllocator, &vkCreateInfo, &allocInfo, reinterpret_cast<VkBuffer*>(&uniformBuffer), &uniformBufferMemory, nullptr);
	};
	~VulkanCamera()
	{
		vmaDestroyBuffer(*_memoryAllocator, uniformBuffer, uniformBufferMemory);
	};
	enum CameraType { lookat, firstperson };
	CameraType type = CameraType::lookat;

	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;

	bool updated = false;

	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;

	vk::Buffer					uniformBuffer;
	VmaAllocation				uniformBufferMemory;
    vk::DescriptorSet           descriptorSet;

	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} keys;

	bool moving()
	{
		return keys.left || keys.right || keys.up || keys.down;
	}

	float getNearClip() { 
		return znear;
	}

	float getFarClip() {
		return zfar;
	}

	void setPerspective(float fov, float aspect, float znear, float zfar)
	{
		this->fov = fov;
		this->znear = znear;
		this->zfar = zfar;
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	};

	void updateAspectRatio(float aspect)
	{
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}

	void setPosition(glm::vec3 position)
	{
		this->position = position;
		updateViewMatrix();
	}

	void setRotation(glm::vec3 rotation)
	{
		this->rotation = rotation;
		updateViewMatrix();
	};

	void rotate(glm::vec3 delta)
	{
		this->rotation += delta;
		updateViewMatrix();
	}

	void setTranslation(glm::vec3 translation)
	{
		this->position = translation;
		updateViewMatrix();
	};

	void translate(glm::vec3 delta)
	{
		this->position += delta;
		updateViewMatrix();
	}

    void createDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool)
    {
        vk::DescriptorSetLayout descriptorSetLayout;
        // camera uniform buffer
        vk::DescriptorSetLayoutBinding cameraBinding({ 0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex,
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
        device.allocateDescriptorSets(&allocInfo, &descriptorSet);

        vk::DescriptorBufferInfo bufferInfo({
            uniformBuffer,
            0,
            sizeof(matrices)
        });

        vk::WriteDescriptorSet descriptorWrite({
            descriptorSet,
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            {},
            &bufferInfo,
            {}
        });
        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }

	void UpdateUniformBuffer() 
	{
    	matrices.perspective[1][1] *= -1;;
		void* data;
		vmaMapMemory(*_memoryAllocator, uniformBufferMemory, &data);
		memcpy(data, &matrices, sizeof(matrices));
    	vmaUnmapMemory(*_memoryAllocator, uniformBufferMemory);
	}
	
	void update(float deltaTime)
	{
		updated = false;
		if (type == CameraType::firstperson)
		{
			if (moving())
			{
				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
				camFront.y = sin(glm::radians(rotation.x));
				camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * movementSpeed;

				if (keys.up)
					position += camFront * moveSpeed;
				if (keys.down)
					position -= camFront * moveSpeed;
				if (keys.left)
					position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				if (keys.right)
					position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

				updateViewMatrix();
			}
		}
	};

	// Update camera passing separate axis data (gamepad)
	// Returns true if view or position has been changed
	bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
	{
		bool retVal = false;

		if (type == CameraType::firstperson)
		{
			// Use the common console thumbstick layout		
			// Left = view, right = move

			const float deadZone = 0.0015f;
			const float range = 1.0f - deadZone;

			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			float moveSpeed = deltaTime * movementSpeed * 2.0f;
			float rotSpeed = deltaTime * rotationSpeed * 50.0f;
			 
			// Move
			if (fabsf(axisLeft.y) > deadZone)
			{
				float pos = (fabsf(axisLeft.y) - deadZone) / range;
				position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				retVal = true;
			}
			if (fabsf(axisLeft.x) > deadZone)
			{
				float pos = (fabsf(axisLeft.x) - deadZone) / range;
				position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				retVal = true;
			}

			// Rotate
			if (fabsf(axisRight.x) > deadZone)
			{
				float pos = (fabsf(axisRight.x) - deadZone) / range;
				rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
				retVal = true;
			}
			if (fabsf(axisRight.y) > deadZone)
			{
				float pos = (fabsf(axisRight.y) - deadZone) / range;
				rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
				retVal = true;
			}
		}
		else
		{
			// todo: move code from example base class for look-at
		}

		if (retVal)
		{
			updateViewMatrix();
		}

		return retVal;
	}

};