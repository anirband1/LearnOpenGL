#ifndef PRIMITIVE_RENDERER_H
#define PRIMITIVE_RENDERER_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/stb_image.h>
#include <lib/structs/transform.h>
#include <lib/structs/outline.h>
#include <lib/shader_s.h>

class PrimitiveRenderer
{
protected:
    Transform transform;

    unsigned int _VAO;
    unsigned int _VBO;

    unsigned int NUM_VERTICES;

    PrimitiveRenderer() {};

    PrimitiveRenderer(Transform transform)
    {
        this->transform = transform;
    }

    // -- since its const, theres no overhead of copying
    template <std::size_t N>
    unsigned int primitiveInitialize(const std::array<float, N> &cubeProperties, unsigned int NUM_VERTICES)
    {
        this->NUM_VERTICES = NUM_VERTICES;

        setupVAO(cubeProperties);

        return this->_VAO;
    }

    void primitiveDraw(Shader *shader) {};
    void primitiveDraw(Shader *shader, Outline outline) {};

private:
    template <std::size_t N>
    void setupVAO(const std::array<float, N> &shapeProperties)
    {
        glGenVertexArrays(1, &_VAO);

        glGenBuffers(1, &_VBO);

        // cube
        glBindVertexArray(_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(shapeProperties), shapeProperties.data(), GL_STATIC_DRAW); // ! This * should be ** but make float an array

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0); // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float))); // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float))); // texCoord
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

#endif