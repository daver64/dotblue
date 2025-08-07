#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <utility>
#include <string>
#include <iostream>
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

namespace DotBlue {

GLShader::GLShader() : programID(0) {}

GLShader::~GLShader() {
    deleteProgram();
}

void GLShader::deleteProgram() {
    if (programID) {
        glDeleteProgram(programID);
        programID = 0;
    }
}

unsigned int GLShader::compileShader(unsigned int type, const std::string& src) {
    unsigned int shader = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool GLShader::load(const std::string& vertexSrc, const std::string& fragmentSrc) {
    deleteProgram();

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!vs || !fs) return false;

    programID = glCreateProgram();
    glAttachShader(programID, vs);
    glAttachShader(programID, fs);
    glLinkProgram(programID);

    int status = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &status);
    if (!status) {
        char log[512];
        glGetProgramInfoLog(programID, 512, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteProgram(programID);
        programID = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

bool GLShader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);
    if (!vFile.is_open() || !fFile.is_open()) {
        std::cerr << "Failed to open shader files:\n" << vertexPath << "\n" << fragmentPath << std::endl;
        return false;
    }
    else {
        std::cout << "Shader files opened successfully:\n" << vertexPath << "\n" << fragmentPath << std::endl;
    }
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    return load(vStream.str(), fStream.str());
}

void GLShader::bind() const {
    glUseProgram(programID);
}

void GLShader::unbind() const {
    glUseProgram(0);
}

void GLShader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void GLShader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}

void GLShader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}

void GLShader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

// GLM-friendly uniform setters
void GLShader::setVec2(const std::string &name, const DotBlue::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void GLShader::setVec3(const std::string &name, const DotBlue::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void GLShader::setVec4(const std::string &name, const DotBlue::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void GLShader::setMat3(const std::string &name, const DotBlue::mat3 &matrix) const {
    glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void GLShader::setMat4(const std::string &name, const DotBlue::mat4 &matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

}