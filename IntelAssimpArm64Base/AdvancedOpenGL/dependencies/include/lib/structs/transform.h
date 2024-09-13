#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform
{
public:
    glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
    // glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);

    glm::mat4 GetModelMat()
    {
        glm::mat4 transformMat = glm::mat4(1.0f);

        // // ! lmao this is so scuffed
        // float theta = glm::length(rotation);
        // glm::vec3 axis = glm::normalize(rotation);

        transformMat = glm::translate(transformMat, position);
        transformMat = glm::scale(transformMat, scale);
        // transformMat = glm::rotate(transformMat, theta, axis);

        this->transformMat = transformMat;

        return transformMat;
    }

    glm::mat3 GetNormalMat()
    {
        glm::mat3 normalMat = glm::mat3(1.0);
        normalMat = glm::mat3(glm::transpose(glm::inverse(transformMat)));
        return normalMat;
    }

private:
    glm::mat4 transformMat;
};

#endif