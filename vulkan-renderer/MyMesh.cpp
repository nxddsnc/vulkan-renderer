#include "MyMesh.h"

MyMesh::MyMesh(uint32_t vertexSize, uint32_t indexSize)
{
    m_vertexNum = vertexSize;
    m_indexNum  = indexSize;
    m_vertices.resize(vertexSize);
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
