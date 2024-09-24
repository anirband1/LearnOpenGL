#ifndef PLANE_H
#define PLANE_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/stb_image.h>
#include <lib/structs/transform.h>
#include <lib/shader_s.h>

#include <lib/primitives/primitiveRenderer.h>

class Plane : public PrimitiveRenderer
{
public:
    const static int NUM_VERTICES = 6;

    // declaration of static
    static unsigned int VAO;
    static unsigned int VBO;

    static bool initialized;

    // Transform transform;

    Plane() {};

    Plane(Transform transform) : PrimitiveRenderer(transform)
    {
        this->transform = transform;
    }

    static void initialize()
    {
        if (!initialized)
        {
            Plane instance;
            VAO = instance.primitiveInitialize(PLANE_PROPERTIES, NUM_VERTICES);
            initialized = true;
        }
    }

    void Draw(Shader *shader)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        shader->use();

        glm::mat4 modelMat = transform.modelMatx;
        glm::mat3 normalMat = transform.normalMatx;
        shader->setMat4("model", glm::value_ptr(modelMat));
        shader->setMat3("normalMat", glm::value_ptr(normalMat));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

        glDisable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

    void Draw(Shader *shader, Outline outline)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        shader->use();

        glm::mat4 modelMat = transform.modelMatx;
        glm::mat3 normalMat = transform.normalMatx;
        shader->setMat4("model", glm::value_ptr(modelMat));
        shader->setMat3("normalMat", glm::value_ptr(normalMat));

        glBindVertexArray(VAO);
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

        glBindVertexArray(VAO);
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
    }

private:
    static constexpr std::array<float, 48> PLANE_PROPERTIES = {
        // positions          // normals        // texture coords
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   //
        0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  //
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   //
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
        -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  //
    };
};

// definition of static
unsigned int Plane::VAO = 0;
unsigned int Plane::VBO = 0;
bool Plane::initialized = false;

#endif