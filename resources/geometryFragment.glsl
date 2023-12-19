/*
	fragmentShader.glsl
	author: Telo PHILIPPE

	Use shell texturing to create a grass effect
*/

#version 330 core

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//  x0 = x0 - 0. + 0.0 * C 
	vec3 x1 = x0 - i1 + 1.0 * C.xxx;
	vec3 x2 = x0 - i2 + 2.0 * C.xxx;
	vec3 x3 = x0 - 1. + 3.0 * C.xxx;

	// Permutations
	i = mod(i, 289.0 ); 
	vec4 p = permute( permute( permute( 
	         i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
	       + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
	       + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients
	// ( N*N points uniformly over a square, mapped onto an octahedron.)
	float n_ = 1.0/7.0; // N=7
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                            dot(p2,x2), dot(p3,x3) ) );
}

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec3 vertexNormal;  // Input from the vertex shader
in vec3 worldPos;
in vec2 textureUV;

in float worldHeight;

uniform vec3 u_cameraPosition;

uniform float u_height;

uniform float u_time;

vec3 lightDir = normalize(vec3(1.0f, 1.0f, 1.0f));

float rand2D(in vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float waterHeightFunction(vec2 pos) {
	float height=0;
	float freq = 15.0f;
	float amp = 1.0f;
	int n = 3;
	for(int i = 0; i < n; i++) {
		height += snoise(vec3(pos, u_time*0.02f) * freq) * amp;
		freq *= 2.0f;
		amp *= 0.5f;
	}
	return height;
}

vec3 waterNormal(vec2 pos) {
	float height11 = waterHeightFunction(pos);
	float height12 = waterHeightFunction(pos + vec2(0.01f, 0.0f));
	float height21 = waterHeightFunction(pos + vec2(0.0f, 0.01f));
	vec3 bumpNormal = normalize(cross(vec3(1, height12 - height11, 0),vec3(0, height21-height11, 1)));
	return bumpNormal;
}

vec3 getBumpMappedNormal() {
	// Bump mapping using triplanar mapping

	// transform bumpNormal to world space
	
	// Triplanar uvs
	vec2 uvX = worldPos.zy; // x facing plane
	vec2 uvY = worldPos.xz; // y facing plane
	vec2 uvZ = worldPos.xy; // z facing plane

	// Tangent space normal maps
	vec3 tnormalX = waterNormal(uvX);
	vec3 tnormalY = waterNormal(uvY);
	vec3 tnormalZ = waterNormal(uvZ);

	// Get the sign of the normal
	vec3 axisSign = sign(vertexNormal);

	// Flip tangent normal z to account for surface normal facing
	tnormalX.z *= axisSign.x;
	tnormalY.z *= axisSign.y;
	tnormalZ.z *= axisSign.z;

	// Swizzle the tangent space normals to match world orientation
	vec3 blend = abs(vertexNormal.xyz);
	blend /= blend.x + blend.y + blend.z;

	vec3 tnormal = normalize(
		blend.x * tnormalX +
		blend.y * tnormalY +
		blend.z * tnormalZ +
		vertexNormal
	);

	return tnormal;
}

struct texturingParams {
    vec3 groundColor;
    vec3 sandColor;
    vec3 waterColor;

	vec3 treeColor;
	vec3 trunkColor;
	vec3 grassColor;

    int treeDensity;
    int grassDensity;
};

uniform texturingParams u_texturingParams;

float treeDensity = u_texturingParams.treeDensity;
float grassDensity = u_texturingParams.grassDensity;

vec3 groundColor = u_texturingParams.groundColor;
vec3 sandColor = u_texturingParams.sandColor;
vec3 waterColor = u_texturingParams.waterColor;

vec3 treeColor = u_texturingParams.treeColor;
vec3 trunkColor = u_texturingParams.trunkColor;
vec3 grassColor = u_texturingParams.grassColor;

struct Material {
	bool filled;
	vec3 color;
};

void getTreeMaterial(inout Material material) {
	ivec2 seed = ivec2(textureUV * treeDensity);
	float val = rand2D(seed);
	float forest = snoise(vec3(seed / treeDensity, 0.0f) * 5.0f) ;
	if(val > forest) {
		return;
	}
	vec2 localPos = fract(textureUV * treeDensity) * 2.0f - 1.0f;
	float dist = length(localPos);

	float treeHeight = rand2D(seed + ivec2(1, 1)) * 0.7f + 0.3f;
	
	float radius = 0.2f;
	float truncHeight = 0.7f * treeHeight;
	if(u_height > truncHeight) {
		radius = sin((u_height - truncHeight) / (treeHeight - truncHeight)  * 3.1415f);
	}
	
	material.filled = material.filled || ( dist < radius && u_height < treeHeight);
	material.color = (u_height > truncHeight ? treeColor : trunkColor) * u_height;
}

float grassHeightFactor = 1.0f / 5.0f;

void getGrassMaterial(inout Material material) {
	ivec2 seed = ivec2(textureUV * grassDensity);
	float val = rand2D(seed) * grassHeightFactor * min(abs(worldHeight - 0.15f) / 0.15f, 1.0f);
	float grassVal = rand2D(seed + ivec2(1, 1));

	float forest = pow(snoise(vec3(ivec2(textureUV * treeDensity) / treeDensity, 0.0f) * 5.0f), 0.5f);
	//forest = max(forest, 0.01f);

	float localGrassDensity =  snoise(vec3(seed / grassDensity, 0.0f) * 10.0f) * 0.5f + 0.5f;
	float totalGrassDensity = localGrassDensity;// * (1.0f - forest);

	if(grassVal > totalGrassDensity) {
		return;
	}

	// Random vector for the offset
	vec2 offset = (vec2(rand2D(seed + ivec2(1, 0)), rand2D(seed + ivec2(0, 1))) * 2.0f - 1.0f) * 0.5f;
	// Random vector for the dropping direction
	vec3 droppingDirection = normalize(vec3(rand2D(seed + ivec2(1, 1)), rand2D(seed + ivec2(0, 2)), rand2D(seed + ivec2(1, 2))) * 2.0f - 1.0f);

	offset += droppingDirection.xy * u_height;

	vec2 localPos = fract(textureUV * grassDensity) * 2.0f - 1.0f;
	float dist = length(localPos - offset);

	float radius = (val - u_height) / val / grassHeightFactor;

	if(u_height < val && dist < radius) {
		material.filled = true;
		material.color = grassColor * u_height / grassHeightFactor;
	}
}

void getSurfaceMaterial(inout Material material) {
	if(worldHeight <= 0.01f) { // Is water
		if(u_height == 0.0f) { // First layer
			material.filled = true;
			material.color = waterColor;

			vec3 viewDir = normalize(u_cameraPosition - worldPos);
			
			vec3 tnormal = getBumpMappedNormal();

			vec3 reflectDir = reflect(lightDir, tnormal);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
			material.color += vec3(0.5f, 0.5f, 0.5f) * spec * 3.0f;

		} else {
			material.filled = false;
		}

	} else if (worldHeight <= 0.15f) { // Sand
		if(u_height == 0.0f) { // First layer
			material.filled = true;
			material.color = mix(groundColor, sandColor, min(abs(worldHeight - 0.15f) / 0.15f, 1.0f));

		} else {
			material.filled = false;
		}
	} else {
		if(u_height == 0.0f) { // Ground
			material.filled = true;
			material.color = groundColor;
		} else {
			getGrassMaterial(material);
			getTreeMaterial(material);
		}
	}
}

void main() {
	Material material;
	material.filled = false;

	getSurfaceMaterial(material);

	if(!material.filled) {
		discard;
	}
	
	gPosition = worldPos;
	gNormal = vertexNormal;
	gAlbedo = material.color;
}