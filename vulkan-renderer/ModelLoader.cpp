#include "ModelLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <iostream>
#include "Platform.h"
#include "MyMesh.h"
#include "MyMaterial.h"
#include "MyTexture.h"
#include "Drawable.h"
#include "stb_image.h"
#include <memory>
#include "MyImage.h"
#include <stdio.h>
#include "Utils.h"

ModelLoader::ModelLoader(MyScene *scene)
{
    m_pScene = scene;
}


ModelLoader::~ModelLoader()
{
}

bool ModelLoader::load(const char * filepath)
{
    m_baseDirectory = GetFileDirectory(filepath);
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    m_pAiScene = importer.ReadFile(filepath,
        aiProcess_GenNormals |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_RemoveRedundantMaterials);
    // If the import failed, report it
    if (!m_pAiScene)
    {
        return false;
    }

    _parseScene(m_pAiScene);
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
        auto material = _extractMaterial(m_pAiScene->mMeshes[node->mMeshes[i]]->mMaterialIndex);
        std::shared_ptr<Drawable> drawable = std::make_shared<Drawable>();
        drawable->mesh = mesh;
        drawable->material = material;
        drawable->matrix = matrix;
        m_pScene->AddDrawable(drawable);
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
    if (m_meshMap.count(idx) > 0)
    {
        return m_meshMap.at(idx);
    }
    else
    {
        aiMesh *_mesh = m_pAiScene->mMeshes[idx];

        VertexBits vertexBits;
        vertexBits.hasNormal         = _mesh->HasNormals();
        vertexBits.hasTangent        = _mesh->HasTangentsAndBitangents();
        vertexBits.hasTexCoord0      = _mesh->HasTextureCoords(0);
        std::shared_ptr<MyMesh> mesh = std::make_shared<MyMesh>(vertexBits, _mesh->mNumVertices, _mesh->mNumFaces * 3);

        for (size_t i = 0; i < _mesh->mNumVertices; ++i)
        {
            aiVector3D vertex = _mesh->mVertices[i];
            mesh->m_positions[i].x = vertex.x;
            mesh->m_positions[i].y = vertex.y;
            mesh->m_positions[i].z = vertex.z;
        }
        if (vertexBits.hasNormal) 
        {
            for (size_t i = 0; i < _mesh->mNumVertices; ++i)
            {
                aiVector3D normal = _mesh->mNormals[i];
                mesh->m_normals[i].x = normal.x;
                mesh->m_normals[i].y = normal.y;
                mesh->m_normals[i].z = normal.z;
            }
        }
         if (vertexBits.hasTexCoord1)
         {
             for (size_t i = 0; i < _mesh->mNumVertices; ++i)
             {
                 aiVector3D uv = _mesh->mTextureCoords[0][i];
                 mesh->m_texCoords0[i].x = uv.x;
                 mesh->m_texCoords0[i].y = uv.y;
             }
         }
         if (vertexBits.hasTangent)
         {
             for (size_t i = 0; i < _mesh->mNumVertices; ++i)
             {
                 aiVector3D tangent = _mesh->mTangents[i];
                 mesh->m_tangents[i].x = tangent.x;
                 mesh->m_tangents[i].y = tangent.y;
             }
         }
        if(mesh->m_indexType == 1)
        {
            uint8_t *indexPtr = reinterpret_cast<uint8_t*>(mesh->m_indices);
            for (size_t i = 0; i < _mesh->mNumFaces; ++i)
            {
                aiFace face = _mesh->mFaces[i];
                indexPtr[i * 3 + 0] = face.mIndices[0];
                indexPtr[i * 3 + 1] = face.mIndices[1];
                indexPtr[i * 3 + 2] = face.mIndices[2];
            }
        }
        else if (mesh->m_indexType == 2)
        {

            uint16_t *indexPtr = reinterpret_cast<uint16_t*>(mesh->m_indices);
            for (size_t i = 0; i < _mesh->mNumFaces; ++i)
            {
                aiFace face = _mesh->mFaces[i];
                indexPtr[i * 3 + 0] = face.mIndices[0];
                indexPtr[i * 3 + 1] = face.mIndices[1];
                indexPtr[i * 3 + 2] = face.mIndices[2];
            }
        }
        else 
        {
            uint32_t *indexPtr = reinterpret_cast<uint32_t*>(mesh->m_indices);
            for (size_t i = 0; i < _mesh->mNumFaces; ++i)
            {
                aiFace face = _mesh->mFaces[i];
                indexPtr[i * 3 + 0] = face.mIndices[0];
                indexPtr[i * 3 + 1] = face.mIndices[1];
                indexPtr[i * 3 + 2] = face.mIndices[2];
            }
        }

        m_meshMap.insert(std::make_pair(idx, mesh));
        return mesh;
    }
}

std::shared_ptr<MyMaterial> ModelLoader::_extractMaterial(unsigned int idx)
{
    if (m_materialMap.count(idx) > 0)
    {
        return m_materialMap.at(idx);
    }
    else
    {
        aiMaterial *material = m_pAiScene->mMaterials[idx];
        aiString materialName;
        material->Get(AI_MATKEY_NAME, materialName);
        std::shared_ptr<MyMaterial> myMaterial = std::make_shared<MyMaterial>(materialName.data);

        aiColor3D diffuse;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_OPACITY, myMaterial->m_opacity);
        myMaterial->m_diffuse[0] = diffuse.r;
        myMaterial->m_diffuse[1] = diffuse.g;
        myMaterial->m_diffuse[2] = diffuse.b;

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString texturePath;
            int textureMapMode[3] = { aiTextureMapMode_Wrap };
            aiReturn res = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL,
                reinterpret_cast<aiTextureMapMode*>(textureMapMode));
            myMaterial->m_pDiffuseMap = _extractTexture(texturePath.data, textureMapMode);
        }
        if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString texturePath;
            int textureMapMode[3];
            aiReturn res = material->GetTexture(aiTextureType_NORMALS, 0, &texturePath, NULL, NULL, NULL, NULL,
                reinterpret_cast<aiTextureMapMode*>(textureMapMode));
            myMaterial->m_pNormalMap = _extractTexture(texturePath.data, textureMapMode);
        }
        //TODO: load other types of texture later.

        m_materialMap.insert(std::make_pair(idx, myMaterial));

        return myMaterial;
    }
}

std::shared_ptr<MyTexture> ModelLoader::_extractTexture(char *texturePath, int textureWrapMode[3])
{
    TextureKey textureKey;
    std::strcpy(textureKey.fileName, texturePath);
    if (m_textureMap.count(textureKey) > 0)
    {
        return m_textureMap.at(textureKey);
    }
    else
    {
        std::shared_ptr<MyTexture> myTexture = std::make_shared<MyTexture>();
        myTexture->m_wrapMode[0] = static_cast<WrapMode>(textureWrapMode[0]);
        myTexture->m_wrapMode[1] = static_cast<WrapMode>(textureWrapMode[1]);
        myTexture->m_wrapMode[2] = static_cast<WrapMode>(textureWrapMode[2]);

        myTexture->m_pImage = _extractImage(texturePath);

        m_textureMap.insert(std::make_pair(textureKey, myTexture));
       
        return myTexture;
    }
}

std::shared_ptr<MyImage> ModelLoader::_extractImage(char * filename)
{
    if (m_imageMap.count(filename) > 0)
    {
        return m_imageMap.at(filename);
    }
    else
    {
        const aiTexture *texture = m_pAiScene->GetEmbeddedTexture(filename);
        std::shared_ptr<MyImage> image = std::make_shared<MyImage>(filename);
        int width, height, components;
        unsigned char *imageData = nullptr;
        if (texture != NULL) {
            //returned pointer is not null, read texture from memory
            if (texture->mHeight == 0)
            {
                image->m_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth,
                    &width, &height, &components, 0);
            }
            else
            {
                image->m_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth * texture->mHeight,
                    &width, &height, &components, 0);
            }
        }
        else
        {
            char filePath[1024];
            std::sprintf(filePath, "%s/%s", m_baseDirectory.c_str(), filename);
            FILE *file = std::fopen(filePath, "rb");
            if (file == nullptr)
            {
                return nullptr;
            }
            image->m_data = stbi_load_from_file(file, &width, &height, &components, 0);
        }
        image->m_width = width;
        image->m_height = height;
        image->m_channels = components;

        m_imageMap.insert(std::make_pair(filename, image));

        return image;
    }
}


