#include "MyMesh.h"

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
    if (vertexBits.hasTexCoord1)
    {
        m_texCoords1.resize(vertexSize);
    }
    if (vertexBits.hasTexCoord2)
    {
        m_texCoords2.resize(vertexSize);
    }
    if (vertexBits.hasColor)
    {
        m_colors.resize(vertexSize);
    }

    if (vertexSize < 256)
    {
        m_indexType = 1;
        m_indices = new uint8_t[indexSize];
    }
    else if (vertexSize < 65536)
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

MyMesh::~MyMesh()
{
    delete [] m_indices;
}

int MyMesh::getIndexSize()
{
    return m_indexType;
}
