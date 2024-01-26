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

#define MAX_STEPS 20
#define MAX_LIGHT_STEPS 20

#define PI 3.1415926535897932384626433832795

struct Light {
	int type; // 0 = ambiant, 1 = point, 2 = directional
	vec3 position;
	vec3 color;
	float intensity;
};

uniform Light u_lights[MAX_LIGHTS];
uniform int u_numLights;

uniform vec3 u_domainCenter;
uniform vec3 u_domainSize;

uniform float u_cloudAbsorption;
uniform float u_lightAbsorption;

uniform float u_scatteringG;

void swap(inout float a, inout float b) {
	float tmp = a;
	a = b;
	b = tmp;
}


bool projectToDomain(vec3 ro, vec3 rd, out float tmin, out float tmax) {

	float dminx = u_domainCenter.x - u_domainSize.x;
	float dmaxx = u_domainCenter.x + u_domainSize.x;
	float dminy = u_domainCenter.y - u_domainSize.y;
	float dmaxy = u_domainCenter.y + u_domainSize.y;
	float dminz = u_domainCenter.z - u_domainSize.z;
	float dmaxz = u_domainCenter.z + u_domainSize.z;
	
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

float sampleDensity(vec3 p) {
	// Coordinates in domain space
	vec3 pDomain = (p - u_domainCenter) / u_domainSize * 0.5 + 0.5;

	if(pDomain.x < -0.01 || pDomain.x > 1.01 || pDomain.y < -0.01 || pDomain.y > 1.01 || pDomain.z < -0.01 || pDomain.z > 1.01)
		return 0.0;

	// Test density function

	float dx = min(abs(pDomain.x - 0.0), abs(pDomain.x - 1.0)) * 5.0;
	float dy = min(abs(pDomain.y - 0.0), abs(pDomain.y - 1.0)) * 5.0;
	float dz = min(abs(pDomain.z - 0.0), abs(pDomain.z - 1.0)) * 5.0;

	float d = min(min(dx, dy), dz) + sin(p.x * 4.0) * 0.1 + sin(p.y * 4.0) * 0.1 + sin(p.z * 4.0) * 0.1;
	d = clamp(d, 0.0, 1.0);
	
	return d;
}

float HenyeyGreenstein(float g, float cosTheta) {
	float g2 = g * g;
	return (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5) / (4.0 * PI);
}

float lightMarch(vec3 ro, Light light) {
	vec3 lightDir = normalize(light.position - ro);

	float tmin, tmax;
	if(!projectToDomain(ro, lightDir, tmin, tmax)) return 1.0;

	float t = tmin;

	if(light.type == 1) {
		// Point light
		float tmaxlight = length(light.position - ro);
		tmax = min(tmax, tmaxlight);

		if(tmin >= tmax) return 1.0;
	}
	float maxT = tmax - tmin + 0.01;

	float stepSize = maxT / MAX_LIGHT_STEPS;
	
	float totalDensity = 0.0;

	for(int i = 0; i < MAX_LIGHT_STEPS; i++) {
		vec3 p = ro + lightDir * t;
		float d = sampleDensity(p);
		totalDensity += d * stepSize;
		t += stepSize;
	}

	return exp(-totalDensity * u_lightAbsorption);
}

void main() {
	vec3 albedo = texture(u_Albedo, TexCoords).rgb;
	vec3 normal = texture(u_Normal, TexCoords).rgb;
	vec3 position = texture(u_Position, TexCoords).rgb;

	// Raymarching
	vec2 uv = TexCoords * 2.0 - 1.0;

	vec4 clip = vec4(uv, -1.0, 1.0);
	vec4 eye = vec4(vec2(u_invProjMat * clip), -1.0, 0.0);
	vec3 rayDir = normalize(vec3(u_invViewMat * eye));
	vec3 rayOrigin = u_invViewMat[3].xyz;
	
	float transmittance = 1.0;
	vec3 lightEnergy = vec3(0);

	float tmin, tmax;
	if(projectToDomain(rayOrigin, rayDir, tmin, tmax)) {
		float t = tmin;
		float stepSize = (tmax - tmin) / MAX_STEPS;
		for(int i = 0; i < MAX_STEPS; i++) {
			vec3 p = rayOrigin + rayDir * t;
			float density = sampleDensity(p);

			if(density > 0) {
				for(int j = 0; j < u_numLights; j++) {
					float lightTransmittance = lightMarch(p, u_lights[j]);
					float scattering = HenyeyGreenstein(u_scatteringG, dot(rayDir, rayDir));
					lightEnergy += density * stepSize * transmittance * lightTransmittance * scattering * u_lights[j].intensity * u_lights[j].color;
				}
				transmittance *= exp(-density * stepSize * u_cloudAbsorption);

				if(transmittance < 0.01) break;
			}
			t += stepSize;
		}
	}

	vec3 renderColor = vec3(0);

	if(position == vec3(0)) {
		// Sky color

		renderColor = rayDir;
	} else {

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

			float lightTransmittance = lightMarch(position, u_lights[i]);

			diffuse += albedo * diff * u_lights[i].color * u_lights[i].intensity * lightTransmittance;
		}

		renderColor = ambient + diffuse;

	}

	vec3 finalColor = renderColor * transmittance + lightEnergy;



    FragColor = vec4(finalColor, 1.0);
}