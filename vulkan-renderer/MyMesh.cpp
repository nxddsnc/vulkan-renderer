#include "MyMesh.h"
#include "assimp/mesh.h"
#include "MyAnimation.h"

MyMesh::MyMesh(VertexBits vertexBits, uint32_t vertexSize, uint32_t indexSize)
{
    m_mode = Mode::Triangles;
    m_vertexNum = vertexSize;
    m_indexNum  = indexSize;
    
    m_vertexBits = vertexBits;
    
    m_positions.resize(vertexSize);
    if (vertexBits.hasNormal)
    {
        m_normals.resize(vertexSize);
    } 
    if (vertexBits.hasTangent) 
    {
        m_tangents.resize(vertexSize);
    }
    if (vertexBits.hasTexCoord0)
    {
        m_texCoords0.resize(vertexSize);
    }
    if (vertexBits.hasTexCoord1)
    {
        m_texCoords1.resize(vertexSize);
    }
    if (vertexBits.hasColor)
    {
        m_colors.resize(vertexSize);
    }
    if (vertexBits.hasBone)
    {
        m_joints.resize(vertexSize);
        m_weights.resize(vertexSize);
    }

    //if (vertexSize < 256)
    //{
    //    m_indexType = 1;
    //    m_indices = new uint8_t[indexSize];
    //}
    //else 
    if (vertexSize < 65536)
    {
        m_indexType = 2;
        m_indices = new uint16_t[indexSize];
    }
    else
    {
        m_indexType = 4;
        m_indices = new uint32_t[indexSize];
    }
}

MyMesh::MyMesh()
{
    
}

MyMesh::~MyMesh()
{
    delete [] m_indices;
}

void MyMesh::InitSkinData()
{
	//for (int i = 0; i < m_bones.size(); ++i)
	for (int i = 0; i < std::min(m_bones.size(), (size_t)4); ++i)
	{
		for (int j = 0; j < m_bones[i]->mNumWeights; ++j)
		{
			aiVertexWeight vertexWeight = m_bones[i]->mWeights[j];
			m_weights[vertexWeight.mVertexId][i] = vertexWeight.mWeight;
			m_joints[vertexWeight.mVertexId][i] = m_boneNodes[i]->jointIndex;
		}
	}
}

void MyMesh::CreateCube()
{
    m_mode = Mode::Triangles;
    m_vertexNum = 24;
    m_indexNum = 36;
    m_vertexBits.hasNormal = false;
    m_vertexBits.hasTangent = false;
    m_vertexBits.hasTexCoord0 = false;
    m_vertexBits.hasTexCoord1 = false;

    m_positions.resize(m_vertexNum);
    m_indexType = 2;
    m_indices = new uint16_t[m_indexNum];
    uint16_t* indices = reinterpret_cast<uint16_t*>(m_indices);

    int vOffset = 0, iOffset = 0;

    // -x 
    m_positions[vOffset].x     = -0.5;   m_positions[vOffset].y     = -0.5;       m_positions[vOffset].z     = -0.5;
    m_positions[vOffset + 1].x = -0.5;   m_positions[vOffset + 1].y =  0.5;       m_positions[vOffset + 1].z = -0.5;
    m_positions[vOffset + 2].x = -0.5;   m_positions[vOffset + 2].y =  0.5;       m_positions[vOffset + 2].z =  0.5;
    m_positions[vOffset + 3].x = -0.5;   m_positions[vOffset + 3].y = -0.5;       m_positions[vOffset + 3].z =  0.5;

    indices[iOffset]     = 0;       indices[iOffset + 1] = 1;         indices[iOffset + 2] = 2;
    indices[iOffset + 3] = 2;       indices[iOffset + 4] = 3;         indices[iOffset + 5] = 0;

    vOffset += 4;
    iOffset += 6;

    // +x 
    m_positions[vOffset].x     = 0.5;   m_positions[vOffset].y     = -0.5;       m_positions[vOffset].z     = -0.5;
    m_positions[vOffset + 1].x = 0.5;   m_positions[vOffset + 1].y =  0.5;       m_positions[vOffset + 1].z = -0.5;
    m_positions[vOffset + 2].x = 0.5;   m_positions[vOffset + 2].y =  0.5;       m_positions[vOffset + 2].z =  0.5;
    m_positions[vOffset + 3].x = 0.5;   m_positions[vOffset + 3].y = -0.5;       m_positions[vOffset + 3].z =  0.5;

    indices[iOffset]     = 0 + vOffset;       indices[iOffset + 1] = 2 + vOffset;         indices[iOffset + 2] = 1 + vOffset;
    indices[iOffset + 3] = 2 + vOffset;       indices[iOffset + 4] = 0 + vOffset;         indices[iOffset + 5] = 3 + vOffset;

    vOffset += 4;
    iOffset += 6;

    // -y
    m_positions[vOffset].x     = -0.5;   m_positions[vOffset].y     = -0.5;       m_positions[vOffset].z     = -0.5;
    m_positions[vOffset + 1].x =  0.5;   m_positions[vOffset + 1].y = -0.5;       m_positions[vOffset + 1].z = -0.5;
    m_positions[vOffset + 2].x =  0.5;   m_positions[vOffset + 2].y = -0.5;       m_positions[vOffset + 2].z =  0.5;
    m_positions[vOffset + 3].x = -0.5;   m_positions[vOffset + 3].y = -0.5;       m_positions[vOffset + 3].z =  0.5;

    indices[iOffset]     = 0 + vOffset;       indices[iOffset + 1] = 2 + vOffset;         indices[iOffset + 2] = 1 + vOffset;
    indices[iOffset + 3] = 2 + vOffset;       indices[iOffset + 4] = 0 + vOffset;         indices[iOffset + 5] = 3 + vOffset;

    vOffset += 4;
    iOffset += 6;

    // +y 
    m_positions[vOffset].x     = -0.5;   m_positions[vOffset].y     = 0.5;       m_positions[vOffset].z     = -0.5;
    m_positions[vOffset + 1].x =  0.5;   m_positions[vOffset + 1].y = 0.5;       m_positions[vOffset + 1].z = -0.5;
    m_positions[vOffset + 2].x =  0.5;   m_positions[vOffset + 2].y = 0.5;       m_positions[vOffset + 2].z = 0.5;
    m_positions[vOffset + 3].x = -0.5;   m_positions[vOffset + 3].y = 0.5;       m_positions[vOffset + 3].z = 0.5;

    indices[iOffset]     = 0 + vOffset;       indices[iOffset + 1] = 1 + vOffset;         indices[iOffset + 2] = 2 + vOffset;
    indices[iOffset + 3] = 2 + vOffset;       indices[iOffset + 4] = 3 + vOffset;         indices[iOffset + 5] = 0 + vOffset;

    vOffset += 4;
    iOffset += 6;

    // -z
    m_positions[vOffset].x     = -0.5;   m_positions[vOffset].y     = -0.5;       m_positions[vOffset].z     = -0.5;
    m_positions[vOffset + 1].x = -0.5;   m_positions[vOffset + 1].y =  0.5;       m_positions[vOffset + 1].z = -0.5;
    m_positions[vOffset + 2].x =  0.5;   m_positions[vOffset + 2].y =  0.5;       m_positions[vOffset + 2].z = -0.5;
    m_positions[vOffset + 3].x =  0.5;   m_positions[vOffset + 3].y = -0.5;       m_positions[vOffset + 3].z = -0.5;

    indices[iOffset]     = 0 + vOffset;       indices[iOffset + 1] = 2 + vOffset;         indices[iOffset + 2] = 1 + vOffset;
    indices[iOffset + 3] = 2 + vOffset;       indices[iOffset + 4] = 0 + vOffset;         indices[iOffset + 5] = 3 + vOffset;

    vOffset += 4;
    iOffset += 6;

    // +z
    m_positions[vOffset].x     = -0.5;   m_positions[vOffset].y     = -0.5;       m_positions[vOffset].z     =  0.5;
    m_positions[vOffset + 1].x = -0.5;   m_positions[vOffset + 1].y =  0.5;       m_positions[vOffset + 1].z =  0.5;
    m_positions[vOffset + 2].x =  0.5;   m_positions[vOffset + 2].y =  0.5;       m_positions[vOffset + 2].z =  0.5;
    m_positions[vOffset + 3].x =  0.5;   m_positions[vOffset + 3].y = -0.5;       m_positions[vOffset + 3].z =  0.5;

    indices[iOffset]     = 0 + vOffset;       indices[iOffset + 1] = 1 + vOffset;         indices[iOffset + 2] = 2 + vOffset;
    indices[iOffset + 3] = 2 + vOffset;       indices[iOffset + 4] = 3 + vOffset;         indices[iOffset + 5] = 0 + vOffset;
}

void MyMesh::CreateAixs()
{
    m_mode = Mode::LineList;
    m_vertexNum = 8;
    m_indexNum = 6;
    m_vertexBits.hasNormal    = false;
    m_vertexBits.hasTangent   = false;
    m_vertexBits.hasColor     = true;
    m_vertexBits.hasTexCoord0 = false;
    m_vertexBits.hasTexCoord1 = false;

    m_positions.resize(m_vertexNum);
    m_colors.resize(m_vertexNum);
    m_indexType = 2;
    m_indices = new uint16_t[m_indexNum];
    uint16_t* indices = reinterpret_cast<uint16_t*>(m_indices);

    int vOffset = 0, iOffset = 0;

    // x
    m_positions[vOffset].x     = 0;   m_positions[vOffset].y     = 0;   m_positions[vOffset].z     = 0;
    m_positions[vOffset + 1].x = 1;   m_positions[vOffset + 1].y = 0;   m_positions[vOffset + 1].z = 0;

    m_colors[vOffset].x     = 1.0;    m_colors[vOffset].y     = 0;      m_colors[vOffset].z     = 0;
    m_colors[vOffset + 1].x = 1.0;    m_colors[vOffset + 1].y = 0;      m_colors[vOffset + 1].z = 0; 

    indices[iOffset] = iOffset + 0;   indices[iOffset + 1] = iOffset + 1;

    vOffset += 2;
    iOffset += 2;

    // y
    m_positions[vOffset].x     = 0;   m_positions[vOffset].y     = 0;   m_positions[vOffset].z     = 0;
    m_positions[vOffset + 1].x = 0;   m_positions[vOffset + 1].y = 1;   m_positions[vOffset + 1].z = 0;

    m_colors[vOffset].x     = 0.0;    m_colors[vOffset].y     = 1;      m_colors[vOffset].z     = 0;
    m_colors[vOffset + 1].x = 0.0;    m_colors[vOffset + 1].y = 1;      m_colors[vOffset + 1].z = 0;

    indices[iOffset] = iOffset + 0;   indices[iOffset + 1] = iOffset + 1;

    vOffset += 2;
    iOffset += 2;
    // z
    m_positions[vOffset].x     = 0;   m_positions[vOffset].y     = 0;   m_positions[vOffset].z     = 0;
    m_positions[vOffset + 1].x = 0;   m_positions[vOffset + 1].y = 0;   m_positions[vOffset + 1].z = 1;

    m_colors[vOffset].x     = 0.0;    m_colors[vOffset].y     = 0;      m_colors[vOffset].z     = 1;
    m_colors[vOffset + 1].x = 0.0;    m_colors[vOffset + 1].y = 0;      m_colors[vOffset + 1].z = 1;

    indices[iOffset] = iOffset + 0;   indices[iOffset + 1] = iOffset + 1;
}

int MyMesh::getIndexSize()
{
    return m_indexType;
}
