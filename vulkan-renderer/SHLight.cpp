#include "SHLight.h"
#include <glm/gtc/type_ptr.hpp>
#include "ResourceManager.h"
#include "MyTexture.h"

struct FaceAixs
{
	FaceAixs(glm::vec3 _x, glm::vec3 _y, glm::vec3 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	glm::vec3 x;
	glm::vec3 y;
	glm::vec3 z;
};

const float Ylm[9] = { 0.282095, // 00
					   0.488603, 0.488603, 0.488603, // 11, 10, 1-1
					   1.092548, 1.092548, 1.092548, // 21, 2-1, 2-2,
					   0.315392, // 20
					   0.546274 }; // 22

SHLight::SHLight(ResourceManager *resourceManager, std::vector<std::shared_ptr<MyTexture>> textures)
{
    m_pResourceManager = resourceManager;

	std::vector<FaceAixs> directions;
	// +X
	directions.push_back(FaceAixs(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)));
	// -X
	directions.push_back(FaceAixs(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0)));
	// +Y
	directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)));
	// -Y
	directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));
	// +Z
	directions.push_back(FaceAixs(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
	// -Z
	directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)));

	//// +X
	//directions.push_back(FaceAixs(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)));
	//// -X
	//directions.push_back(FaceAixs(glm::vec3(0, 0, -1), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)));
	//// +Y
	//directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
	//// -Y
	//directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
	//// +Z
	//directions.push_back(FaceAixs(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)));
	//// -Z
	//directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 0, -1),  glm::vec3(0, 1, 0)));


	//// +X
	//directions.push_back(FaceAixs(glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));
	//// -X
	//directions.push_back(FaceAixs(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)));
	//// +Y
	//directions.push_back(FaceAixs(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)));
	//// -Y
	//directions.push_back(FaceAixs(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1)));
	//// +Z
	//directions.push_back(FaceAixs(glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)));
	//// -Z
	//directions.push_back(FaceAixs(glm::vec3(0, 0, -1), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));

	int size = textures[0]->m_pImage->m_width;

	// The (u,v) range is [-1,+1], so the distance between each texel is 2/Size.
	float du = 2.0 / size;
	float dv = du;

	// The (u,v) of the first texel is half a texel from the corner (-1,-1).
	float minUV = du * 0.5 - 1.0;

	glm::vec3 l00 = glm::vec3(0), l1_1 = glm::vec3(0), l10 = glm::vec3(0), l11 = glm::vec3(0), l2_2 = glm::vec3(0), l2_1 = glm::vec3(0), l20 = glm::vec3(0), l21 = glm::vec3(0), l22 = glm::vec3(0);

	float totalSolidAngle = 0;
	for (int faceIndex = 0; faceIndex < 6; ++faceIndex)
	{
		float v = minUV;

		FaceAixs axis = directions[faceIndex];
		for (int y = 0; y < size; ++y)
		{
			float u = minUV;
			for (int x = 0; x < size; ++x)
			{
				glm::vec3 worldDirection = axis.x * u + axis.y * v + axis.z;
				worldDirection = glm::normalize(worldDirection);

				float deltaSolidAngle = pow(1.0 + u * u + v * v, -3.0 / 2.0);

				glm::vec3 color = textures[faceIndex]->m_pImage->ReadPixel(x, y);

				l00  += Ylm[0] * color * deltaSolidAngle;

				l1_1 += Ylm[1] * color * deltaSolidAngle * worldDirection.x;
				l10  += Ylm[2] * color * deltaSolidAngle * worldDirection.y;
				l11  += Ylm[3] * color * deltaSolidAngle * worldDirection.z;

				l21  += Ylm[4] * color * deltaSolidAngle * worldDirection.x * worldDirection.z;
				l2_1 += Ylm[5] * color * deltaSolidAngle * worldDirection.y * worldDirection.z;
				l2_2 += Ylm[6] * color * deltaSolidAngle * worldDirection.x * worldDirection.y;

				l20 += Ylm[7] * color * deltaSolidAngle * (3 * worldDirection.z * worldDirection.z - 1);

				l22  += Ylm[8] * color * deltaSolidAngle * (worldDirection.x * worldDirection.x - worldDirection.y * worldDirection.y);

				u += du;

				totalSolidAngle += deltaSolidAngle;
			}
			u = u + dv;
		}
	}

	l00  /= totalSolidAngle;

	l1_1 /= totalSolidAngle;
	l10  /= totalSolidAngle;
	l11  /= totalSolidAngle;

	l21  /= totalSolidAngle;
	l2_1 /= totalSolidAngle;
	l2_2 /= totalSolidAngle;

	l20 /= totalSolidAngle;
	l22  /= totalSolidAngle;

	float c1 = 0.429043, c2 = 0.511664, c3 = 0.743125, c4 = 0.886227, c5 = 0.247708;

	glm::vec3 M00 = l22   * c1, M01 = l2_2 * c1, M02 = l21  * c1, M03 = l11  * c2;
	glm::vec3 M10 = l2_2  * c1, M11 = -l22 * c1, M12 = l2_1 * c1, M13 = l1_1 * c2;
	glm::vec3 M20 = l21   * c1, M21 = l2_1 * c1, M22 = l20  * c3, M23 = l10  * c2;
	glm::vec3 M30 = l11   * c2, M31 = l1_1 * c2, M32 = l10  * c2, M33 = l00 * c4 - l20 * c5;

	float r[16] = { M00.x, M01.x, M02.x, M03.x, M10.x, M11.x, M12.x, M13.x, M20.x, M21.x, M22.x, M23.x, M30.x, M31.x, M32.x, M33.x };

	float g[16] = { M00.y, M01.y, M02.y, M03.y, M10.y, M11.y, M12.y, M13.y, M20.y, M21.y, M22.y, M23.y, M30.y, M31.y, M32.y, M33.y };

	float b[16] = { M00.z, M01.z, M02.z, M03.z, M10.z, M11.z, M12.z, M13.z, M20.z, M21.z, M22.z, M23.z, M30.z, M31.z, M32.z, M33.z };

	//float r[16] = { 0.09009903, -0.047194730000000004, 0.24026408000000002, -0.14838256, -0.047194730000000004, -0.09009903, -0.11155118, 0.19954896, 0.24026408000000002, -0.11155118, -0.1189, -0.17396576000000002, -0.14838256, 0.19954896, -0.17396576000000002, 0.73975261 };

	//float g[16] = { -0.021452150000000003, -0.021452150000000003, 0.09009903, -0.03069984, -0.021452150000000003, 0.021452150000000003, -0.09438946000000001, 0.1790824, 0.09009903, -0.09438946000000001, -0.06688125, -0.09209952, -0.03069984, 0.1790824, -0.09209952, 0.41223360000000003 };

	//float b[16] = { -0.1287129, -0.05148516, 0.060066020000000005, 0.00511664, -0.05148516, 0.1287129, -0.20165021, 0.3069984, 0.060066020000000005, -0.20165021, -0.11146875, -0.13814928, 0.00511664, 0.3069984, -0.13814928, 0.51571878 };


	m_matrixR = glm::make_mat4(r);
	m_matrixG = glm::make_mat4(g);
	m_matrixB = glm::make_mat4(b);

	m_pResourceManager->CreateUniformBuffer(sizeof(m_matrixR) * 3, reinterpret_cast<VkBuffer*>(&m_uniformBuffer), &m_uniformBufferMemory);
}

SHLight::~SHLight()
{
	m_pResourceManager->DestroyUniformBuffer(m_uniformBuffer, m_uniformBufferMemory);
}

void SHLight::CreateDescriptorSet(vk::Device &device, vk::DescriptorPool &descriptorPool)
{
    vk::DescriptorSetLayout descriptorSetLayout;
    // light uniform buffer
    vk::DescriptorSetLayoutBinding lightBinding({ 0,
        vk::DescriptorType::eUniformBuffer,
        1,
        vk::ShaderStageFlagBits::eFragment,
        {} });

    vk::DescriptorSetLayoutCreateInfo layoutInfo({ {},
        1,
        &lightBinding });
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
        sizeof(m_matrixR) * 3
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

void SHLight::UpdateUniformBuffer()
{
	int size = sizeof(m_matrixR) * 3;
	char* data = new char[size];
    memcpy(data, &m_matrixR, sizeof(m_matrixR));
    memcpy(data + sizeof(m_matrixR), &m_matrixG, sizeof(m_matrixG));
    memcpy(data + 2 * sizeof(m_matrixR), &m_matrixB, sizeof(m_matrixB));
	m_pResourceManager->UpdateBuffer(m_uniformBufferMemory, data, size);
}

