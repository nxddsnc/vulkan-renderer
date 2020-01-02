#include "Scene.h"
#pragma once

struct aiScene;
struct aiNode;
struct aiMesh;

class ModelLoader
{
public:
    ModelLoader(Scene *scene);
    ~ModelLoader();

    bool load(const char* filepath);
private:
    void _parseScene(const aiScene *scene);
    void _extractNode(aiNode *node, glm::mat4 &transform);
    void _extractTransform(glm::mat4 &transform, void *aiMatrix);
    void _extractMesh(aiMesh *mesh);
private:
    aiScene  *_scene;
    Scene    *m_scene;
};

