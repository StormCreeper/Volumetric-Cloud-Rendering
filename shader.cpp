/*
    shader.cpp
    author: Telo PHILIPPE

    Some useful shader functions.
*/

#include "shader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
    GLuint shader = glCreateShader(type);                                     // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
    std::string shaderSourceString = file2String(shaderFilename);             // Loads the shader source from a file to a C++ string
    const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str();  // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL);                           // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
    std::ifstream t(filename.c_str());
    // check if file exists
    if (!t.good()) {
        std::cerr << "ERROR: Cannot open file '" << filename << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

void setUniform(GLuint program, const std::string &name, float x) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniform1f(loc, x);
}
void setUniform(GLuint program, const std::string &name, int x) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniform1i(loc, x);
}
void setUniform(GLuint program, const std::string &name, bool x) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniform1i(loc, x);
}
void setUniform(GLuint program, const std::string &name, const glm::vec3 &v) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(v));
}
void setUniform(GLuint program, const std::string &name, const glm::vec4 &v) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniform4fv(loc, 1, glm::value_ptr(v));
}
void setUniform(GLuint program, const std::string &name, const glm::mat3 &m) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}
void setUniform(GLuint program, const std::string &name, const glm::mat4 &m) {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}