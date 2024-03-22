#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lib/shader_s.h>

class PointLight
{
public:
    glm::vec3 lightColor;
    float lightStrength;
    glm::vec3 lightPos;
    Shader *shaderProg;

    PointLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightPos)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightPos = _lightPos;

        shaderProg->use();

        shaderProg->setVec3("pointLight.lightColor", glm::value_ptr(lightColor));
        shaderProg->setFloat("pointLight.lightStrength", lightStrength);
        shaderProg->setVec3("pointLight.lightPos", glm::value_ptr(lightPos));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3("pointLight.lightColor", glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat("pointLight.lightStrength", lightStrength);
    }

    void SetLightPos(glm::vec3 _lightPos)
    {
        lightPos = _lightPos;
        shaderProg->setVec3("pointLight.lightPos", glm::value_ptr(lightPos));
    }
};

class DirectionalLight
{
public:
    glm::vec3 lightColor;
    float lightStrength;
    glm::vec3 lightDir;
    Shader *shaderProg;

    DirectionalLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightDir)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightDir = _lightDir;

        shaderProg->use();

        shaderProg->setVec3("directionalLight.lightColor", glm::value_ptr(lightColor));
        shaderProg->setFloat("directionalLight.lightStrength", lightStrength);
        shaderProg->setVec3("directionalLight.lightDir", glm::value_ptr(lightDir));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3("directionalLight.lightColor", glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat("directionalLight.lightStrength", lightStrength);
    }

    void SetLightDir(glm::vec3 _lightDir)
    {
        lightDir = _lightDir;
        shaderProg->setVec3("directionalLight.lightDir", glm::value_ptr(lightDir));
    }
};

class SpotLight
{
public:
    glm::vec3 lightColor;
    float lightStrength;
    glm::vec3 lightPos;
    glm::vec3 lightDir;
    float innerCutoff;
    float outerCutoff;
    Shader *shaderProg;

    SpotLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightPos, glm::vec3 _lightDir, float _innerCutoff, float _outerCutoff)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightPos = _lightPos;
        lightDir = _lightDir;
        innerCutoff = _innerCutoff;
        outerCutoff = _outerCutoff;

        shaderProg->use();

        shaderProg->setVec3("spotLight.lightColor", glm::value_ptr(lightColor));
        shaderProg->setFloat("spotLight.lightStrength", lightStrength);
        shaderProg->setVec3("spotLight.lightPos", glm::value_ptr(lightPos));
        shaderProg->setFloat("spotLight.innerCutoff", glm::cos(glm::radians(innerCutoff)));
        shaderProg->setFloat("spotLight.outerCutoff", glm::cos(glm::radians(outerCutoff)));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3("spotLight.lightColor", glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat("spotLight.lightStrength", lightStrength);
    }

    void SetLightPos(glm::vec3 _lightPos)
    {
        lightPos = _lightPos;
        shaderProg->setVec3("spotLight.lightPos", glm::value_ptr(lightPos));
    }

    void SetLightDir(glm::vec3 _lightDir)
    {
        lightDir = _lightDir;
        shaderProg->setVec3("spotLight.lightDir", glm::value_ptr(lightDir));
    }

    void SetInnerCutoff(float _innerCutoff)
    {
        innerCutoff = _innerCutoff;
        shaderProg->setFloat("spotLight.innerCutoff", innerCutoff);
    }

    void SetOuterCutoff(float _outerCutoff)
    {
        outerCutoff = _outerCutoff;
        shaderProg->setFloat("spotLight.outerCutoff", outerCutoff);
    }
};