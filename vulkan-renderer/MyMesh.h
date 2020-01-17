#include "Platform.h"
#include <vector>
#pragma once

struct VertexBits
{
    VertexBits()  : hasNormal(true), hasTangent(false), hasTexCoord1(false),  hasTexCoord2(false), hasColor(false) {}
    bool hasNormal    : 1;
    bool hasTangent   : 1;
    bool hasTexCoord1 : 1;
    bool hasTexCoord2 : 1;
    bool hasColor     : 1;
};

enum class Mode : uint8_t {
    Points = 0,        ///< Each vertex defines a separate point
    Lines = 1,         ///< The first two vertices define the first segment, with subsequent pairs of vertices each defining one more segment
    LineStrip = 3,     ///< The first vertex specifies the first segment’s start point while the second vertex specifies the first segment’s endpoint and the second segment’s start point
    Triangles = 4,     ///<
    TriangleStrip = 5, ///<
    TriangleFan = 6    ///<
};

class MyMesh
{
public:
    MyMesh(VertexBits vertexBits, uint32_t vertexSize, uint32_t indexSize);
    ~MyMesh();

    int getIndexSize();
    int                     m_indexType;
    VertexBits              m_vertexBits;
    std::vector<glm::vec3>  m_positions;
    std::vector<glm::vec3>  m_normals;
    std::vector<glm::vec3>  m_tangents;
    std::vector<glm::vec2>  m_texCoords1;
    std::vector<glm::vec2>  m_texCoords2;
    std::vector<glm::vec3>  m_colors;
    void                  * m_indices;
    uint32_t                m_vertexNum;
    uint32_t                m_indexNum;

    Mode                    m_mode;
};

