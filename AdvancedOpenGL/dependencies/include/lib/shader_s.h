#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char *vertexPath, const char *fragmentPath);

    // insert directives
    // ------------------------------------------------------------------------

    /// @param whichShader 0 for vertex shader, 1 for fragment shader
    void insertDirective(int whichShader, std::string directive);

    // activate the shader
    // ------------------------------------------------------------------------
    void use() { glUseProgram(ID); }
    // ------------------------------------------------------------------------
    void del() { glDeleteProgram(ID); }

    // utility uniform functions
    // TODO make it so that when the below are called, it does shader.use()
    // TODO need ereference to current shader tho hmm
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, float *value) const;
    void setMat4(const std::string &name, float *value) const;
    void setMat3(const std::string &name, float *value) const;
    // void setMat4(const std::string &name, const glm::mat4 &mat) const
    // {
    //     glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    // }

private:
    char *vShaderCode;
    char *fShaderCode;

    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type);

    int compileAndLink(const char *vShaderCode, const char *fShaderCode);

    // ! really sus function to convert from const char* to char*
    char *unconstchar(const char *s);
};
#endif
