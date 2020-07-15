#include "Drawable.h"

Drawable::Drawable()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	m_bReady = false;
}

Drawable::~Drawable()
{
}

void Drawable::ComputeBBox()
{
	BBox box = {glm::vec3(INFINITE), -glm::vec3(INFINITE)};

	for (int i = 0; i < m_mesh->m_vertexNum; ++i)
	{
		if (box.min.x > m_mesh->m_positions[i].x)
		{
			box.min.x = m_mesh->m_positions[i].x;
		}
		if (box.min.y > m_mesh->m_positions[i].y)
		{
			box.min.y = m_mesh->m_positions[i].y;
		}
		if (box.min.z > m_mesh->m_positions[i].z)
		{
			box.min.z = m_mesh->m_positions[i].z;
		}

		if (box.max.x < m_mesh->m_positions[i].x)
		{
			box.max.x = m_mesh->m_positions[i].x;
		}
		if (box.max.y < m_mesh->m_positions[i].y)
		{
			box.max.y = m_mesh->m_positions[i].y;
		}
		if (box.max.z < m_mesh->m_positions[i].z)
		{
			box.max.z = m_mesh->m_positions[i].z;
		}
	}

	glm::vec3 boxVerts[8] = { glm::vec3(box.max.x, box.max.y, box.min.z), glm::vec3(box.max.x, box.min.y, box.min.z), 
		glm::vec3(box.min.x, box.max.y, box.min.z), glm::vec3(box.min.x, box.min.y, box.min.z),
		glm::vec3(box.max.x, box.max.y, box.max.z), glm::vec3(box.max.x, box.min.y, box.max.z),
		glm::vec3(box.min.x, box.max.y, box.max.z), glm::vec3(box.min.x, box.min.y, box.max.z) };

	for (int i = 0; i < 8; ++i)
	{
		glm::vec4 vec = m_matrix * glm::vec4(boxVerts[i].x, boxVerts[i].y, boxVerts[i].z, 1);
		if (m_bbox.min.x > vec.x)
		{
			m_bbox.min.x = vec.x;
		}
		if (m_bbox.min.y > vec.y)
		{
			m_bbox.min.y = vec.y;
		}
		if (m_bbox.min.z > vec.z)
		{
			m_bbox.min.z = vec.z;
		}

		if (m_bbox.max.x < vec.x)
		{
			m_bbox.max.x = vec.x;
		}
		if (m_bbox.max.y < vec.y)
		{
			m_bbox.max.y = vec.y;
		}
		if (m_bbox.max.z < vec.z)
		{
			m_bbox.max.z = vec.z;
		}
	}
}
