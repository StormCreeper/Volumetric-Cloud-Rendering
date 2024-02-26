#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>

#include "gl_includes.hpp"
//#include ""

class Renderer {
public:
    // GPU objects
    GLuint g_geometryShader {};
    GLuint g_lightingShader {};

    
    //std::shared_ptr<FrameBuffer> g_framebuffer {};


};




#endif // RENDERER_HPP