#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <lib/shader_s.h>

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    vShaderCode = unconstchar(vertexCode.c_str());
    fShaderCode = unconstchar(fragmentCode.c_str());
    // 2. compile shaders

    ID = compileAndLink(vShaderCode, fShaderCode);
}

void Shader::insertDirective(int whichShader, std::string directive)
{
    if (!whichShader) // vertex shader
    {
        std::string s = std::string(vShaderCode);

        std::string version = s.substr(0, s.find('\n')) + '\n';
        std::string rest = s.substr(s.find('\n'));
        std::string vertexCode = version + directive + '\n' + rest;
        vShaderCode = unconstchar(vertexCode.c_str());
    }
    else // fragment shader
    {
        std::string s = std::string(fShaderCode);

        std::string version = s.substr(0, s.find('\n')) + '\n';
        std::string rest = s.substr(s.find('\n'));
        std::string fragmentCode = version + directive + '\n' + rest;
        fShaderCode = unconstchar(fragmentCode.c_str());
    }

    ID = compileAndLink(vShaderCode, fShaderCode);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec3(const std::string &name, float *value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
}
void Shader::setMat4(const std::string &name, float *value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}
void Shader::setMat3(const std::string &name, float *value) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

int Shader::compileAndLink(const char *vShaderCode, const char *fShaderCode)
{
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return ID;
}

char *Shader::unconstchar(const char *s)
{
    if (!s)
        return NULL;
    int i;
    char *res = NULL;
    res = (char *)malloc(strlen(s) + 1);
    if (!res)
    {
        fprintf(stderr, "Memory Allocation Failed! Exiting...\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (i = 0; s[i] != '\0'; i++)
        {
            res[i] = s[i];
        }
        res[i] = '\0';
        return res;
    }
}
