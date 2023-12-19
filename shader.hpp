/*
    shader.hpp
    author: Telo PHILIPPE

    Some useful functions to load shaders and set uniforms.
*/

#ifndef SHADER_HPP
#define SHADER_HPP

#include "gl_includes.hpp"
#include <string>

std::string file2String(const std::string &filename);
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename);

void setUniform(GLuint program, const std::string &name, float x);
void setUniform(GLuint program, const std::string &name, int x);
void setUniform(GLuint program, const std::string &name, bool x);
void setUniform(GLuint program, const std::string &name, const glm::vec3 &v);
void setUniform(GLuint program, const std::string &name, const glm::vec4 &v);
void setUniform(GLuint program, const std::string &name, const glm::mat3 &m);
void setUniform(GLuint program, const std::string &name, const glm::mat4 &m);

#endif  // SHADER_HPP