#pragma once

#include "glad/glad.h"
#include "glfw3.h"

#include "shader.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <vector>
#include <string>

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
    unsigned int id = 0;
    std::string type;
    std::string path;
};

class Mesh
{
public:

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Positions.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normals.
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Texture coords.
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // Tangent.
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        // Bitangent.
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        // Ids.
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // Weights.
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
    }

    void Draw(Shader& shader)
    {
        // Bind appropriate textures.
        unsigned int diffuseNumber = 1;
        unsigned int specularNumber = 1;
        unsigned int normalNumber = 1;
        unsigned int heightNumber = 1;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            // Activate proper texture unit before binding.
            glActiveTexture(GL_TEXTURE0 + i);

            // Retrieve texture number (the N in diffuse_textureN).

            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
            {
                number = std::to_string(diffuseNumber++);
            }
            else if (name == "texture_specular")
            {
                number = std::to_string(specularNumber++); // transfer unsigned int to string
            }
            else if (name == "texture_normal")
            {
                number = std::to_string(normalNumber++); // transfer unsigned int to string
            }
            else if (name == "texture_height")
            {
                number = std::to_string(heightNumber++); // transfer unsigned int to string
            }

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // Draw mesh.
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (unsigned int)(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:

    unsigned int VBO, EBO;
};