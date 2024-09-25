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

    Transform transform;

    Plane() {};

    Plane(Transform transform) : PrimitiveRenderer(transform, NUM_VERTICES)
    {
        this->transform = transform;
        this->_VAO = VAO; // ! This'll break if initialize() is not called first in main.cpp
    }

    static void initialize()
    {
        if (!initialized)
        {
            Plane instance;
            VAO = instance.primitiveInitialize(PLANE_PROPERTIES);
            initialized = true;
        }
    }

    void Draw(Shader *shader) { primitiveDraw(shader); }

    void Draw(Shader *shader, Outline outline) { primitiveDraw(shader, outline); }

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