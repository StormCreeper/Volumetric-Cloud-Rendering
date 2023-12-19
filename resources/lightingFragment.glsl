#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gDepth;

void main() {
    FragColor = vec4(texture(gAlbedo, TexCoords).rgb, 1.0);
}