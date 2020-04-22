#include "MyScene.h"
#include <unordered_map>
#include <memory.h>
#pragma once

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

class MyMesh;
class MyMaterial;
class MyTexture;
class MyImage;

struct TextureKey
{
    int  wrapMode[3];
    char fileName[1024];

    bool operator==(TextureKey const &textureKey) const 
    {
        return std::strcmp(fileName, textureKey.fileName) == 0 &&
                        wrapMode[0] == textureKey.wrapMode[0] &&
                        wrapMode[1] == textureKey.wrapMode[1] && 
                        wrapMode[2] == textureKey.wrapMode[2];
    }

    bool operator!=(TextureKey const &textureKey) const 
    {
        return std::strcmp(fileName, textureKey.fileName) != 0 ||
                        wrapMode[0] != textureKey.wrapMode[0] ||
                        wrapMode[1] != textureKey.wrapMode[1] ||
                        wrapMode[2] != textureKey.wrapMode[2];
    }
};
template<> struct std::hash<TextureKey> {
    size_t operator()(const TextureKey& textureKey) const {
        // TODO: use more appropriate hash value
        return std::hash<uint32_t>()(std::strlen(textureKey.fileName) + 
                                    (textureKey.wrapMode[0] + textureKey.wrapMode[1] << 2 + textureKey.wrapMode[2] << 4) << 10);
    }
};

class ModelLoader
{
public:
    ModelLoader(std::shared_ptr<MyScene> scene);
    ~ModelLoader();

    bool load(const char* filepath);
private:
    void _parseScene(const aiScene *scene);
    void _extractNode(aiNode *node, glm::mat4 &transform);
    void _extractTransform(glm::mat4 &transform, void *aiMatrix);
	std::shared_ptr<BoneNode> _traverseBuildSkeleton(aiNode* parent);
	void _extractSkeletonAnimations();
    std::shared_ptr<MyMesh> _extractMesh(unsigned int idx);
    std::shared_ptr<MyMaterial> _extractMaterial(unsigned int idx);
    std::shared_ptr<MyTexture> _extractTexture(char *texturePath, int textureWrapMode[3]);
    std::shared_ptr<MyImage> _extractImage(char *filePath);
private:
    const aiScene            *m_pAiScene;
	std::shared_ptr<MyScene>  m_pScene;
    bool                      m_flipYZ;

    std::string        m_baseDirectory;

    std::unordered_map<unsigned int, std::shared_ptr<MyMesh>>       m_meshMap;
    std::unordered_map<unsigned int, std::shared_ptr<MyMaterial>>   m_materialMap;
    std::unordered_map<TextureKey, std::shared_ptr<MyTexture>>     m_textureMap;
    std::unordered_map<std::string, std::shared_ptr<MyImage>>       m_imageMap;

	std::unordered_map<aiNode*, std::shared_ptr<BoneNode>> m_nodeBoneMap;
	std::unordered_map<std::string, std::shared_ptr<BoneNode>> m_nameBoneMap;

};

