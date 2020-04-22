#include "Platform.h"
#include <vector>
#pragma once

struct aiNode;
struct VertexBits
{
    VertexBits()  : hasNormal(true), hasTangent(false), hasTexCoord0(false),  hasTexCoord1(false), hasColor(false), hasBone(false) {}
    bool hasNormal    : 1;
    bool hasTangent   : 1;
    bool hasTexCoord0 : 1;
    bool hasTexCoord1 : 1;
    bool hasColor     : 1;
	bool hasBone      : 1;
};

enum class Mode : uint8_t {
    Points = 0,        ///< Each vertex defines a separate point
    LineList = 1,         ///< The first two vertices define the first segment, with subsequent pairs of vertices each defining one more segment
    LineStrip = 3,     ///< The first vertex specifies the first segment’s start point while the second vertex specifies the first segment’s endpoint and the second segment’s start point
    Triangles = 4,     ///<
    TriangleStrip = 5, ///<
    TriangleFan = 6    ///<
};

struct BoneNode
{
	struct VertexWeight
	{
		unsigned int vertexId;
		float weight;
	};
	struct KeyVector
	{
		glm::vec3 value;
		double time;
	};
	struct KeyQuat
	{
		glm::quat value;
		double time;
	};
	std::string			      name;
	std::shared_ptr<BoneNode> parent;
	aiNode*					  root;
	std::vector<std::shared_ptr<BoneNode>> children;
	std::vector<VertexWeight> vertexWeights;
	std::vector<KeyVector>    keyPositions;
	std::vector<KeyVector>    keyScalings;
	std::vector<KeyQuat>      keyRotations;
};

struct MyAnimation
{
	std::shared_ptr<BoneNode> root;
	double					  duration;
	double					  currentTime;
};

class MyMesh
{
public:
	MyMesh(VertexBits vertexBits, uint32_t vertexSize, uint32_t indexSize);
	MyMesh();
	~MyMesh();

	void CreateCube();
	void CreateAixs();
	int getIndexSize();
	int                     m_indexType;
	VertexBits              m_vertexBits;
	std::vector<glm::vec3>  m_positions;
	std::vector<glm::vec3>  m_normals;
	std::vector<glm::vec3>  m_tangents;
	std::vector<glm::vec3>  m_texCoords0;
	std::vector<glm::vec2>  m_texCoords1;
	std::vector<glm::vec3>  m_colors;
	void                  * m_indices;
	uint32_t                m_vertexNum;
	uint32_t                m_indexNum;

	Mode                    m_mode;
};

