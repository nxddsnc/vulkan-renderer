#include "ModelLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <iostream>
#include "Platform.h"

ModelLoader::ModelLoader(Scene *scene)
{
    m_scene = scene;
}


ModelLoader::~ModelLoader()
{
}

bool ModelLoader::load(const char * filepath)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    _scene = importer.ReadFile(filepath,
        aiProcess_GenNormals |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_RemoveRedundantMaterials);
    // If the import failed, report it
    if (!_scene)
    {
        return false;
    }

    double scalingFactor;
    _scene->mMetaData->Get("UnitScaleFactor", scalingFactor);
    ai_int upAxis = -1;
    _scene->mMetaData->Get("UpAxis", upAxis);

    _parseScene(_scene);
    return true;
}

void ModelLoader::_parseScene(const aiScene * scene)
{
    aiNode *root = scene->mRootNode;
    glm::mat4 identity = glm::mat4(1.0);
    _extractNode(root, identity);
}

void ModelLoader::_extractNode(aiNode * node, glm::mat4 &transform)
{
    glm::mat4  matrix;
    _extractTransform(matrix, &(node->mTransformation));
    matrix = matrix * transform;

    for (int i = 0; i < node->mNumChildren; ++i)
    {
        aiNode *child = node->mChildren[i];

        for (int j = 0; j < child->mNumMeshes; ++j)
        {
            // extract mesh
            _extractMesh(_scene->mMeshes[child->mMeshes[i]]);
        }

        for (int j = 0; j < child->mNumChildren; ++j)
        {
            _extractNode(child->mChildren[i], matrix);
        }
    }
}

void ModelLoader::_extractTransform(glm::mat4 & transform, void * aiMatrix)
{
    // TODO: Correctness check.
    aiMatrix4x4 *_matrix = (aiMatrix4x4*)aiMatrix;
    transform[0][0] = _matrix->a1; transform[1][0] = _matrix->b1; transform[2][0] = _matrix->c1; transform[3][0] = _matrix->d1;
    transform[0][1] = _matrix->a2; transform[1][1] = _matrix->b2; transform[2][1] = _matrix->c2; transform[3][1] = _matrix->d2;
    transform[0][2] = _matrix->a3; transform[1][2] = _matrix->b3; transform[2][2] = _matrix->c3; transform[3][2] = _matrix->d3;
    transform[0][3] = _matrix->a4; transform[1][3] = _matrix->b4; transform[2][3] = _matrix->c4; transform[3][3] = _matrix->d4;
}

void ModelLoader::_extractMesh(aiMesh * mesh)
{
    _scene->mMaterials[mesh->mMaterialIndex];
  
}


