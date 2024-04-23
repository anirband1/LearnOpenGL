#ifndef EFFECTS_H
#define EFFECTS_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/constants.h>
#include <lib/shader_s.h>
#include <lib/transform.h>

#include <iostream>

void outlineAndDraw(Shader *shader, Shader *outlineShader, unsigned int VAO, int numDrawnVertices, Transform transform, glm::vec3 color, float thickness)
{
    // * inits
    // * draw (normal shader)
    // * disable
    // * draw (outline shdaer)
    // * defaults

    // ----------------------------------------------------------------------------------------------------
    // inits
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    shader->use();

    glm::mat4 modelMat = transform.GetModelMat();
    glm::mat3 normalMat = transform.GetNormalMat();
    shader->setMat4("model", glm::value_ptr(modelMat));      // + whaaaaaa?
    shader->setMat3("normalMat", glm::value_ptr(normalMat)); // + whaaaaaa?

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

    // + ---------------------------------------------------------------------------------
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    // doing *= scale makes outline relative to object (cuz it takes obj scale into acc)
    // doing scale += thickness makes it absolute thickness // im doing this

    outlineShader->use();
    outlineShader->setVec3("outlineColor", glm::value_ptr(color));

    // + ---------------------------------------------------------------------------------
    Transform scaledTransform = transform;
    scaledTransform.scale += thickness;

    modelMat = scaledTransform.GetModelMat();
    normalMat = scaledTransform.GetNormalMat();
    outlineShader->setMat4("model", glm::value_ptr(modelMat));
    outlineShader->setMat3("normalMat", glm::value_ptr(normalMat));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, numDrawnVertices);

    // defaults
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);

    glBindVertexArray(0);

    shader->use();
}

#endif