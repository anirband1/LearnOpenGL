#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform
{
public:
    glm::mat4 modelMatx;
    glm::mat3 normalMatx;

    Transform()
    {
        setPosition(glm::vec3(0.0, 0.0, 0.0));
        setScale(glm::vec3(1.0, 1.0, 1.0));
    }

    Transform(glm::vec3 position, glm::vec3 scale)
    {
        setPosition(position);
        setScale(scale);
    }

    // -- getter-setter
    void setPosition(glm::vec3 newPos)
    {
        this->position = newPos;
        modelMatx = CalculateModelMatx();
        normalMatx = CalculateNormalMatx(modelMatx);
    }

    void setScale(glm::vec3 newScale)
    {
        this->scale = newScale;
        modelMatx = CalculateModelMatx();
        normalMatx = CalculateNormalMatx(modelMatx);
    }

    glm::vec3 getPosition() { return position; }
    glm::vec3 getScale() { return scale; }

private:
    glm::vec3 position;
    glm::vec3 scale;
    // glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);

    glm::mat4 CalculateModelMatx()
    {
        glm::mat4 transformMat = glm::mat4(1.0f);

        // // ! lmao this is so scuffed
        // float theta = glm::length(rotation);
        // glm::vec3 axis = glm::normalize(rotation);

        transformMat = glm::translate(transformMat, position);
        transformMat = glm::scale(transformMat, scale);
        // transformMat = glm::rotate(transformMat, theta, axis);

        return transformMat;
    }

    glm::mat3 CalculateNormalMatx(glm::mat4 transformMat)
    {
        glm::mat3 normalMat = glm::mat3(1.0);
        normalMat = glm::mat3(glm::transpose(glm::inverse(transformMat)));
        return normalMat;
    }
};

#endif