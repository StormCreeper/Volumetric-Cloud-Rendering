#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <iostream>

#include "gl_includes.hpp"
#include "mesh.hpp"

class FrameBuffer {
public:
    GLuint m_Buffer {};
    GLuint m_position {};
    GLuint m_normal {};
    GLuint m_albedo {};
    GLuint m_depth {};

    int m_Width {};
    int m_Height {};

    std::shared_ptr<Mesh> m_quad {};
public:
    FrameBuffer(int width, int height) {
        m_Width = width;
        m_Height = height;
        initFramebuffer();
        initMesh();
    }

    void initFramebuffer() {
        glGenFramebuffers(1, &m_Buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Buffer);

        // - Position color buffer
        glGenTextures(1, &m_position);
        glBindTexture(GL_TEXTURE_2D, m_position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_position, 0);

        // - Normal color buffer
        glGenTextures(1, &m_normal);
        glBindTexture(GL_TEXTURE_2D, m_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normal, 0);

        // - Color + Specular color buffer
        glGenTextures(1, &m_albedo);
        glBindTexture(GL_TEXTURE_2D, m_albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_albedo, 0);

        // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);

        // - Create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &m_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

        // - Finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    void initMesh() {
        m_quad = Mesh::genPlane();
    }
};



#endif // FRAMEBUFFER_HPP