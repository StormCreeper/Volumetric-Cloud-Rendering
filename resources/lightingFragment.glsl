#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_Albedo;

uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform mat4 u_invViewMat;
uniform mat4 u_invProjMat;

uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;

#define MAX_LIGHTS 10

struct Light {
	int type; // 0 = ambiant, 1 = directional, 2 = point
	vec3 position;
	vec3 color;
	float intensity;
};

uniform Light u_lights[MAX_LIGHTS];
uniform int u_numLights;

vec3 lightDir = normalize(u_lightPosition);

void main() {
	vec3 albedo = texture(u_Albedo, TexCoords).rgb;
	vec3 normal = texture(u_Normal, TexCoords).rgb;

	vec3 diffuse = vec3(0);
	vec3 ambient = vec3(0);

	for (int i = 0; i < u_numLights; i++) {
		if(u_lights[i].type == 0) {
			ambient += albedo * u_lights[i].color * u_lights[i].intensity;
			continue;
		}
		vec3 lightDir;
		if(u_lights[i].type == 1) {
			lightDir= normalize(u_lights[i].position - texture(u_Position, TexCoords).xyz);
		} else {
			lightDir= normalize(u_lights[i].position);
		}
		float diff = max(dot(normal, lightDir), 0.0);
		diffuse += albedo * diff * u_lights[i].color * u_lights[i].intensity;
	}

	vec3 finalColor = ambient + diffuse;


    FragColor = vec4(finalColor, 1.0);
}