#include "ModelLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <iostream>
#include "Platform.h"
#include "MyMesh.h"
#include "MyMaterial.h"
#include "MyTexture.h"
#include "MyNode.h"

ModelLoader::ModelLoader(MyScene *scene)
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

    _parseScene(_scene);
    return true;
}

void ModelLoader::_parseScene(const aiScene * scene)
{
    aiNode *root = scene->mRootNode;
    glm::mat4 identity = glm::mat4(1.0);
    _extractNode(root, identity);
}

void ModelLoader::_extractNode(aiNode * node, glm::mat4 &parentTransform)
{
    glm::mat4  matrix;
    _extractTransform(matrix, &(node->mTransformation));
    matrix = matrix * parentTransform;

    for (int i = 0; i < node->mNumMeshes; ++i)
    {
        // extract mesh
        auto mesh = _extractMesh(node->mMeshes[i]);
        std::shared_ptr<RenderNode> myNode = std::make_shared<RenderNode>();
        myNode->mesh = mesh;
        myNode->matrix = matrix;
        m_scene->AddNode(myNode);
    }

    for (int i = 0; i < node->mNumChildren; ++i)
    {
        aiNode *child = node->mChildren[i];

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

std::shared_ptr<MyMesh> ModelLoader::_extractMesh(unsigned int idx)
{
    if (_meshMap.count(idx) > 0)
    {
        return _meshMap.at(idx);
    }
    else
    {
        std::shared_ptr<MyMesh> mesh = std::make_shared<MyMesh>();
        
        aiMesh *_mesh = _scene->mMeshes[idx];

        bool hasNormal = _mesh->HasNormals();
        bool hasUV = _mesh->HasTextureCoords(0);
       
        mesh->m_vertices.resize(_mesh->mNumVertices);
        mesh->m_indices.resize(_mesh->mNumFaces * 3);
        
        for (size_t i = 0; i < _mesh->mNumVertices; ++i)
        {
            aiVector3D vertex = _mesh->mVertices[i];
            aiVector3D normal = _mesh->mNormals[i];
            mesh->m_vertices[i].pos.x = vertex.x;
            mesh->m_vertices[i].pos.y = vertex.y;
            mesh->m_vertices[i].pos.z = vertex.z;
        
            mesh->m_vertices[i].normal.x = normal.x;
            mesh->m_vertices[i].normal.y = normal.y;
            mesh->m_vertices[i].normal.z = normal.z;
        
            if (hasUV)
            {
                aiVector3D uv = _mesh->mTextureCoords[0][i];
                mesh->m_vertices[i].texCoord.x = uv.x;
                mesh->m_vertices[i].texCoord.y = uv.y;
            }
            else
            {
                // TODO: conditional compile.
                mesh->m_vertices[i].texCoord.x = 0.0;
                mesh->m_vertices[i].texCoord.y = 0.0;
            }
        }

        for (size_t i = 0; i < _mesh->mNumFaces; ++i)
        {
            aiFace face = _mesh->mFaces[i];
            mesh->m_indices[i * 3 + 0] = face.mIndices[0];
            mesh->m_indices[i * 3 + 1] = face.mIndices[1];
            mesh->m_indices[i * 3 + 2] = face.mIndices[2];
        }

        _meshMap.insert(std::make_pair(idx, mesh));
        return mesh;
    }
}

std::shared_ptr<MyMaterial> ModelLoader::_extractMaterial(unsigned int idx)
{
    if (_materialMap.count(idx) > 0)
    {
        return _materialMap.at(idx);
    }
    else
    {
        std::shared_ptr<MyMaterial> material = std::make_shared<MyMaterial>();

        /*aiMaterial *material = _scene->mMaterials[mesh->mMaterialIndex];
        aiString materialName;
        material->Get(AI_MATKEY_NAME, materialName);
        sprintf_s(myMaterial->name, "%s_%d", materialName.data, m_materialMap.size());

        printf("%s\n", myMaterial->name);
        aiColor3D diffuse;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_OPACITY, myMaterial->tr);
        myMaterial->kd[0] = diffuse.r;
        myMaterial->kd[1] = diffuse.g;
        myMaterial->kd[2] = diffuse.b;
*/
        _materialMap.insert(std::make_pair(idx, material));

        return material;
    }

}


