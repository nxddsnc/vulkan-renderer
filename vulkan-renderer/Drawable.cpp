#include "Drawable.h"

SingleDrawable::SingleDrawable()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	m_type = SINGLE_DRAWABLE;
	m_bReady = false;
}

SingleDrawable::~SingleDrawable()
{
}

void SingleDrawable::ComputeBBox()
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

// TODO: Not efficient way to initialize variables.
InstanceDrawable::InstanceDrawable()
{
	m_bbox.min = glm::vec3(INFINITE);
	m_bbox.max = -glm::vec3(INFINITE);
	m_type = INSTANCE_DRAWABLE;
	m_bReady = false;
}

InstanceDrawable::InstanceDrawable(std::shared_ptr<SingleDrawable> d)
{
	m_mesh = d->m_mesh;
	m_material = d->m_material;
	m_type = INSTANCE_DRAWABLE;
	m_matricies.push_back(d->m_matrix);
	m_normalMatrices.push_back(d->m_normalMatrix);
	m_bReady = false;
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols.emplace_back(std::vector<glm::vec4>());
	m_matrixCols[0].emplace_back(glm::vec4(d->m_matrix[0][0], d->m_matrix[1][0], d->m_matrix[2][0], d->m_matrix[3][0]));
	m_matrixCols[1].emplace_back(glm::vec4(d->m_matrix[0][1], d->m_matrix[1][1], d->m_matrix[2][1], d->m_matrix[3][1]));
	m_matrixCols[2].emplace_back(glm::vec4(d->m_matrix[0][2], d->m_matrix[1][2], d->m_matrix[2][2], d->m_matrix[3][2]));
}

InstanceDrawable::~InstanceDrawable()
{
}

void InstanceDrawable::AddDrawable(std::shared_ptr<SingleDrawable> d)
{
	m_matricies.push_back(d->m_matrix);
	// TODO: not cache friendly.
	m_matrixCols[0].push_back(glm::vec4(d->m_matrix[0][0], d->m_matrix[1][0], d->m_matrix[2][0], d->m_matrix[3][0]));
	m_matrixCols[1].push_back(glm::vec4(d->m_matrix[0][1], d->m_matrix[1][1], d->m_matrix[2][1], d->m_matrix[3][1]));
	m_matrixCols[2].push_back(glm::vec4(d->m_matrix[0][2], d->m_matrix[1][2], d->m_matrix[2][2], d->m_matrix[3][2]));
	m_normalMatrices.push_back(d->m_normalMatrix);
}

void InstanceDrawable::ComputeBBox()
{
	BBox box = { glm::vec3(INFINITE), -glm::vec3(INFINITE) };

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

	for (int i = 0; i < m_matricies.size(); ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			glm::vec4 vec = m_matricies[i] * glm::vec4(boxVerts[j].x, boxVerts[j].y, boxVerts[j].z, 1);
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
}

int Drawable::GetHash()
{
	// TODO: use real hash function
	return (int)m_mesh.get() + (int)m_material.get();
}
