#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

// Input from the vertex shaders
in vec3 vertexNormal;
in vec3 worldPos;
in vec2 textureUV;

uniform vec3 u_cameraPosition;

void main() {
	gPosition = worldPos;
	gNormal = vertexNormal;
	gAlbedo = vec3(1);
}