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

    PointLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightPos, int index)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightPos = _lightPos;

        posChar = "pointLights[" + std::to_string(index) + "].lightPos";
        colorChar = "pointLights[" + std::to_string(index) + "].lightColor";
        strengthChar = "pointLights[" + std::to_string(index) + "].lightStrength";

        shaderProg->use();

        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
        shaderProg->setFloat(strengthChar, lightStrength);
        shaderProg->setVec3(posChar, glm::value_ptr(lightPos));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat(strengthChar, lightStrength);
    }

    void SetLightPos(glm::vec3 _lightPos)
    {
        lightPos = _lightPos;
        shaderProg->setVec3(posChar, glm::value_ptr(lightPos));
    }

private:
    std::string posChar;
    std::string colorChar;
    std::string strengthChar;
};

class DirectionalLight
{
public:
    glm::vec3 lightColor;
    float lightStrength;
    glm::vec3 lightDir;
    Shader *shaderProg;

    DirectionalLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightDir, int index)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightDir = _lightDir;

        shaderProg->use();

        dirChar = "directionalLights[" + std::to_string(index) + "].lightDir";
        colorChar = "directionalLights[" + std::to_string(index) + "].lightColor";
        strengthChar = "directionalLights[" + std::to_string(index) + "].lightStrength";

        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
        shaderProg->setFloat(strengthChar, lightStrength);
        shaderProg->setVec3(dirChar, glm::value_ptr(lightDir));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat(strengthChar, lightStrength);
    }

    void SetLightDir(glm::vec3 _lightDir)
    {
        lightDir = _lightDir;
        shaderProg->setVec3(dirChar, glm::value_ptr(lightDir));
    }

private:
    std::string dirChar;
    std::string colorChar;
    std::string strengthChar;
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

    SpotLight(Shader *_shaderProg, glm::vec3 _lightColor, float _lightStrength, glm::vec3 _lightPos, glm::vec3 _lightDir, float _innerCutoff, float _outerCutoff, int index)
    {
        shaderProg = _shaderProg;
        lightColor = _lightColor;
        lightStrength = _lightStrength;
        lightPos = _lightPos;
        lightDir = _lightDir;
        innerCutoff = _innerCutoff;
        outerCutoff = _outerCutoff;

        dirChar = "spotLights[" + std::to_string(index) + "].lightDir";
        colorChar = "spotLights[" + std::to_string(index) + "].lightColor";
        posChar = "spotLights[" + std::to_string(index) + "].lightPos";
        strengthChar = "spotLights[" + std::to_string(index) + "].lightStrength";
        innerCutoffChar = "spotLights[" + std::to_string(index) + "].innerCutoff";
        outerCutoffChar = "spotLights[" + std::to_string(index) + "].outerCutoff";

        shaderProg->use();

        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
        shaderProg->setFloat(strengthChar, lightStrength);
        shaderProg->setVec3(posChar, glm::value_ptr(lightPos));
        shaderProg->setFloat(innerCutoffChar, glm::cos(glm::radians(innerCutoff)));
        shaderProg->setFloat(outerCutoffChar, glm::cos(glm::radians(outerCutoff)));
    }

    void SetLightColor(glm::vec3 _lightColor)
    {
        lightColor = _lightColor;
        shaderProg->setVec3(colorChar, glm::value_ptr(lightColor));
    }

    void SetLightStrength(float _lightStrength)
    {
        lightStrength = _lightStrength;
        shaderProg->setFloat(strengthChar, lightStrength);
    }

    void SetLightPos(glm::vec3 _lightPos)
    {
        lightPos = _lightPos;
        shaderProg->setVec3(posChar, glm::value_ptr(lightPos));
    }

    void SetLightDir(glm::vec3 _lightDir)
    {
        lightDir = _lightDir;
        shaderProg->setVec3(dirChar, glm::value_ptr(lightDir));
    }

    void SetInnerCutoff(float _innerCutoff)
    {
        innerCutoff = _innerCutoff;
        shaderProg->setFloat(innerCutoffChar, innerCutoff);
    }

    void SetOuterCutoff(float _outerCutoff)
    {
        outerCutoff = _outerCutoff;
        shaderProg->setFloat(outerCutoffChar, outerCutoff);
    }

private:
    std::string posChar;
    std::string dirChar;
    std::string colorChar;
    std::string strengthChar;
    std::string innerCutoffChar;
    std::string outerCutoffChar;
};