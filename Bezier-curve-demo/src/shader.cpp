#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "shader.h"

using namespace std;

Shader::Shader(const GLchar* vertexShaderFilePath, const GLchar* fragmentShaderFilePath) {
    ifstream vertexShaderFile, fragmentShaderFile;
    string vertexShaderCodeString, fragmentShaderCodeString;
    
    vertexShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    fragmentShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    
    try {
        vertexShaderFile.open(vertexShaderFilePath);
        fragmentShaderFile.open(fragmentShaderFilePath);
        
        // Read file content to string stream
        stringstream vertexShaderStream, fragmentShaderStream;
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();
        
        // From string stream to string
        vertexShaderCodeString = vertexShaderStream.str();
        fragmentShaderCodeString = fragmentShaderStream.str();
        
    } catch (ifstream::failure e) {
        cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
    }
    
    // From String to C string
    const char* vertexShaderCodeCString = vertexShaderCodeString.c_str();
    const char* fragmentShaderCodeCString = fragmentShaderCodeString.c_str();
    
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCodeCString, NULL);
    glCompileShader(vertexShader);
    this->checkComplieErrors(vertexShader, "VERTEX");
    
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCodeCString, NULL);
    glCompileShader(fragmentShader);
    this->checkComplieErrors(fragmentShader, "FRAGMENT");
    
    this->shaderProgram = glCreateProgram();
    glAttachShader(this->shaderProgram, vertexShader);
    glAttachShader(this->shaderProgram, fragmentShader);
    glLinkProgram(this->shaderProgram);
    this->checkComplieErrors(shaderProgram, "PROGRAM");
    
    // Delete the linked shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLuint Shader::getShaderProgram() {
    return this->shaderProgram;
}

void Shader::setUniform1i(const std::string& name, const int value) const {
    glUniform1i(glGetUniformLocation(this->shaderProgram, name.c_str()), value);
}

void Shader::setUniform1f(const std::string& name, const float value) const {
    glUniform1f(glGetUniformLocation(this->shaderProgram, name.c_str()), value);
}

void Shader::setUniform2fv(const std::string& name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, &value[0]);
}

void Shader::setUniform2f(const std::string& name, const float x, const float y) const {
    glUniform2f(glGetUniformLocation(this->shaderProgram, name.c_str()), x, y);
}

void Shader::setUniform3fv(const std::string& name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, &value[0]);
}

void Shader::setUniform3f(const std::string& name, const float x, const float y, const float z) const {
    glUniform3f(glGetUniformLocation(this->shaderProgram, name.c_str()), x, y, z);
}

void Shader::setUniform4fv(const std::string& name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, &value[0]);
}

void Shader::setUniformMatrix2fv(const std::string& name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniformMatrix3fv(const std::string& name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniformMatrix4fv(const std::string& name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(this->shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkComplieErrors(GLuint shader, std::string shaderType) {
    int success;
    char infoLog[512];
    if (shaderType != "PROGRAM") {
        // Check shader
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            cout << "ERROR::SHADER_COMPLICATION_ERROR of type " << shaderType << "\n" << infoLog << endl;
        }
    } else {
        // Check program
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            cout << "ERROR::PROGRAM_LINKING_ERROR of type " << shaderType << "\n" << infoLog << endl;
        }
    }
}
