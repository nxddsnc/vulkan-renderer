#include "Scene.h"
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

class ModelLoader
{
public:
    ModelLoader(MyScene *scene);
    ~ModelLoader();

    bool load(const char* filepath);
private:
    void _parseScene(const aiScene *scene);
    void _extractNode(aiNode *node, glm::mat4 &transform);
    void _extractTransform(glm::mat4 &transform, void *aiMatrix);
    std::shared_ptr<MyMesh> _extractMesh(unsigned int idx);
    std::shared_ptr<MyMaterial> _extractMaterial(unsigned int idx);
private:
    const aiScene     *_scene;
    MyScene           *m_scene;


    std::unordered_map<unsigned int, std::shared_ptr<MyMesh>> _meshMap;
    std::unordered_map<unsigned int, std::shared_ptr<MyMaterial>> _materialMap;
    std::unordered_map<unsigned int, std::shared_ptr<MyTexture>> _textureMap;
};

