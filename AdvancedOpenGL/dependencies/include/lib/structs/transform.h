#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO can put Calculate funcs in one

struct Transform
{
public:
    glm::mat4 modelMatx;
    glm::mat3 normalMatx;

    Transform()
    {
        setPosition(glm::vec3(0.0, 0.0, 0.0));
        setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        setScale(glm::vec3(1.0, 1.0, 1.0));
    }

    Transform(glm::vec3 position, glm::vec3 scale)
    {
        setPosition(position);
        setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        setScale(scale);
    }

    Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
    {
        setPosition(position);
        setRotation(rotation);
        setScale(scale);
    }

    // -- getter-setter
    void setPosition(glm::vec3 newPos)
    {
        this->position = newPos;
        modelMatx = CalculateModelMatx();
        normalMatx = CalculateNormalMatx(modelMatx);
    }

    /// @brief applies deg z, deg x, deg y in that order
    /// @param newRot takes (deg x, deg y, deg z)
    void setRotation(glm::vec3 newRot)
    {
        this->rotation = newRot;
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
    glm::vec3 rotation;

    glm::mat4 CalculateModelMatx()
    {
        glm::mat4 transformMat = glm::mat4(1.0f);

        transformMat = glm::translate(transformMat, position);

        transformMat = glm::rotate(transformMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        transformMat = glm::rotate(transformMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transformMat = glm::rotate(transformMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

        transformMat = glm::scale(transformMat, scale);

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