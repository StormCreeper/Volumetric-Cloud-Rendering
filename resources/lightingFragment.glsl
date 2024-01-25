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

#define MAX_LIGHTS 10

struct Light {
	int type; // 0 = ambiant, 1 = directional, 2 = point
	vec3 position;
	vec3 color;
	float intensity;
};

uniform Light u_lights[MAX_LIGHTS];
uniform int u_numLights;

uniform vec3 u_domainOrigin;
uniform vec3 u_domainSize;

void swap(inout float a, inout float b) {
	float tmp = a;
	a = b;
	b = tmp;
}


bool projectToDomain(vec3 ro, vec3 rd, out float tmin, out float tmax) {

	float dminx = u_domainOrigin.x;
	float dmaxx = u_domainOrigin.x + u_domainSize.x;
	float dminy = u_domainOrigin.y;
	float dmaxy = u_domainOrigin.y + u_domainSize.y;
	float dminz = u_domainOrigin.z;
	float dmaxz = u_domainOrigin.z + u_domainSize.z;
	
	tmin = (dminx - ro.x) / rd.x;
	tmax = (dmaxx - ro.x) / rd.x;
	if (tmin > tmax) swap(tmin, tmax);

	float tymin = (dminy - ro.y) / rd.y;
	float tymax = (dmaxy - ro.y) / rd.y;
	if (tymin > tymax) swap(tymin, tymax);
	if ((tmin > tymax) || (tymin > tmax)) 
        return false; 
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (dminz - ro.z) / rd.z;
	float tzmax = (dmaxz - ro.z) / rd.z;
	if (tzmin > tzmax) swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax)) 
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	
	tmin = max(tmin, 0.0f);
	tmax = max(tmax, 0.0f);
	if(tmin >= tmax) return false;

	return true;
}

void main() {
	vec3 albedo = texture(u_Albedo, TexCoords).rgb;
	vec3 normal = texture(u_Normal, TexCoords).rgb;
	vec3 position = texture(u_Position, TexCoords).rgb;

	// Raymarching
	vec2 uv = TexCoords * 2.0 - 1.0;

	vec4 clip = vec4(uv, -1.0, 1.0);
	vec4 eye = vec4(vec2(u_invProjMat * clip), -1.0, 0.0);
	vec3 rayDir = vec3(u_invViewMat * eye);
	vec3 rayOrigin = u_invViewMat[3].xyz;

	float tmin, tmax;
	if(!projectToDomain(rayOrigin, rayDir, tmin, tmax)) {
		FragColor = vec4(0, 0, 0, 1);
		return;
	}

	if(position == vec3(0)) {
		FragColor = vec4(rayDir, 1);
		return;
	}

	// Solid objects color

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

	


	vec3 finalObjectColor = ambient + diffuse;



    FragColor = vec4(finalObjectColor, 1.0);
}