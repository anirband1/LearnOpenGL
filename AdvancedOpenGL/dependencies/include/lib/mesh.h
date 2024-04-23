#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <lib/constants.h>
#include <lib/shader_s.h>
#include <lib/outline.h>

#include <string>
#include <vector>

using namespace std;

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    // albedo00 -> albedo texture for material 0;
    void Draw(Shader *shader)
    {
        // ? maybe put shader.use() for safety?

        unsigned int diffuseNR = 0;
        unsigned int specularNR = 0;
        unsigned int normalNR = 0;

        for (int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture before binding

            string index;
            string type = textures[i].type;

            if (type == "albedo")
                index = to_string(diffuseNR++);
            else if (type == "specular")
                index = to_string(specularNR++);
            else if (type == "normal")
                index = to_string(normalNR++);

            shader->setInt("textureMaterials[" + index + "]." + type, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id); // + possible optimization, put this part in setup, instead of draw
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

    void DrawWithOutline(Shader *shader, Outline outline)
    {
        // init
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        // draw (normal shader)
        shader->use();
        Draw(shader);

        // disable
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        // draw (outline shader)
        outline.outlineShader->use();
        outline.outlineShader->setVec3("outlineColor", glm::value_ptr(outline.outlineColor));

        Transform scaledTranform = outline.transform;
        scaledTranform.scale += outline.outlineThickness;

        glm::mat4 modelMat = scaledTranform.GetModelMat();
        outline.outlineShader->setMat4("model", glm::value_ptr(modelMat));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

        // defaults
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        shader->use();
    }

private:
    unsigned int VAO, VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);                                                            // * contains type Vertex
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // * directly pass pointer since structs stored continguous

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position)); // * "offsetof(Vertex, Position)" evaluates to 0
        glEnableVertexAttribArray(0);

        // vertex normals
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        // vertex textures
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
};

#endif
