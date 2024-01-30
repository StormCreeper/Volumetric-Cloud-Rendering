#ifndef VOXEL_TEXTURE_HPP
#define VOXEL_TEXTURE_HPP

#include "gl_includes.hpp"
#include "shader.hpp"

class VoxelTexture {
public:
    GLuint textureID {};
    GLuint shaderID {};
    
    GLuint dim {};

public:
    VoxelTexture() {
        dim = 64;
    }

    void generateTexture() {

        if (shaderID) glDeleteProgram(shaderID);

        shaderID = glCreateProgram();
        
        loadShader(shaderID, GL_COMPUTE_SHADER, "../resources/compute.glsl");

        glLinkProgram(shaderID);

        glUseProgram(shaderID);

        if (!textureID) {
            glGenTextures(1, &textureID);
        }

        glBindTexture(GL_TEXTURE_3D, textureID);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dim, dim, dim, 0, GL_RED, GL_FLOAT, nullptr);

        glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glDispatchCompute(dim/8, dim/8, dim/8);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glBindTexture(GL_TEXTURE_3D, 0);

        glUseProgram(0);
    }
};

#endif // voxeltexture.hpp