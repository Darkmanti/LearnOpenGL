#pragma once

#include "glad/glad.h"
#include "glfw3.h"

#include "shader.h"
#include "mesh.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <string>
#include <map>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Model
{
public:

    std::vector<Texture> LoadedTextures;
    std::vector<Mesh> Meshes;
    std::string Directory;

    Model(std::string path)
    {
        LoadModel(path);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < Meshes.size(); i++)
        {
            Meshes[i].Draw(shader);
        }
    }

private:

    unsigned int TextureFromFile(const char* path, const std::string& directory)
    {
        std::string filename = std::string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

        if (data)
        {
            GLenum format{};
            if (nrComponents == 1)
            {
                format = GL_RED;
            }
            else if (nrComponents == 3)
            {
                format = GL_RGB;
            }
            else if (nrComponents == 4)
            {
                format = GL_RGBA;
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            wprintf(L"Texture failed to load at path: %hs\n", path);
            stbi_image_free(data);
        }

        return textureID;
    }

    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool alreadyExist = false;

            for (unsigned int j = 0; j < LoadedTextures.size(); j++)
            {
                if (std::strcmp(LoadedTextures[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(LoadedTextures[j]);
                    alreadyExist = true;
                    break;
                }
            }

            // If texture hasn't been loaded already, load it.
            if (!alreadyExist)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), Directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                LoadedTextures.push_back(texture);
            }
        }
        return textures;
    }

    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            // Process vertex positions, normals and texture coordinates.

            Vertex vertex{};
            glm::vec3 vector{};

            // Positions.
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normals.
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // Texture coordinates.
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec{};

                // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).

                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // Tangent.
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                // Bitangent.
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // Process indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Process material.
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        // height: texture_heightN

        // 1. Diffuse maps.
        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 2. Specular maps.
        std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 3. Normal maps.
        std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 4. Height maps.
        std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // Return result.
        return Mesh(vertices, indices, textures);
    }

    void ProcessNode(aiNode* node, const aiScene* scene)
    {
        // Process all the node's meshes (if any).
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            Meshes.push_back(ProcessMesh(mesh, scene));
        }

        // Then do the same for each of its children.
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    void LoadModel(std::string path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        // Check for errors.
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            wprintf(L"ERROR::ASSIMP::%hs\n", importer.GetErrorString());
            return;
        }

        Directory = path.substr(0, path.find_last_of('/'));

        ProcessNode(scene->mRootNode, scene);
    }
};