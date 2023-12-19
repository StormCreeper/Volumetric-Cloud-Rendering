#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vUV;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(vPosition, 1.0);
    TexCoords = vUV;
}
