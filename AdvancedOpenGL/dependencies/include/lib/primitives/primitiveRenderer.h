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
    unsigned int _VAO;
    unsigned int _VBO;

    PrimitiveRenderer() {};
    PrimitiveRenderer(Transform transform, unsigned int NUM_VERTICES)
    {
        this->_transform = transform;
        this->NUM_VERTICES = NUM_VERTICES;
    };

    // -- since its const, theres no overhead of copying
    template <std::size_t N>
    unsigned int primitiveInitialize(const std::array<float, N> &cubeProperties)
    {
        setupVAO(cubeProperties);

        return this->_VAO;
    }

    virtual void primitiveDraw(Shader *shader)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        shader->use();

        glm::mat4 modelMat = _transform.modelMatx;
        glm::mat3 normalMat = _transform.normalMatx;
        shader->setMat4("model", glm::value_ptr(modelMat));
        shader->setMat3("normalMat", glm::value_ptr(normalMat));

        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

        glDisable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

    virtual void primitiveDraw(Shader *shader, Outline outline)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        shader->use();

        glm::mat4 modelMat = _transform.modelMatx;
        glm::mat3 normalMat = _transform.normalMatx;
        shader->setMat4("model", glm::value_ptr(modelMat));
        shader->setMat3("normalMat", glm::value_ptr(normalMat));

        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

        // disable
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        // draw (outline shader)
        outline.outlineShader->use();
        outline.outlineShader->setVec3("outlineColor", glm::value_ptr(outline.outlineColor));
        outline.outlineShader->setMat4("model", glm::value_ptr(modelMat));
        outline.outlineShader->setMat3("normalMat", glm::value_ptr(normalMat));

        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        shader->use();
    };

private:
    Transform _transform;

    unsigned int NUM_VERTICES;

    template <std::size_t N>
    void setupVAO(const std::array<float, N> &shapeProperties)
    {
        glGenVertexArrays(1, &_VAO);

        glGenBuffers(1, &_VBO);

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