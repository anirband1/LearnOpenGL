#ifndef OUTLINE_H
#define OUTLINE_H

#include <glm/glm.hpp>

#include <lib/constants.h>
#include <lib/shader_s.h>
#include <lib/structs/transform.h>

struct Outline
{
    glm::vec3 outlineColor; // (1.0, 0.25, 1.0)
    float outlineThickness; // 0.1f
    Shader *outlineShader;
    Transform transform;
};

#endif