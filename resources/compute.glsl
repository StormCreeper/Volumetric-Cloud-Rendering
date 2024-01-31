#version 430

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (r32f, binding = 0) uniform image3D img_output;

uniform vec3 u_resolution;
uniform vec3 u_targetSize;
uniform vec3 u_targetOffset;
uniform float u_time;

float tseed = 0;
uint rngState;

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
} 
float RandomFloat01(inout uint state) {
    return float(wang_hash(state)) / 4294967296.0;
}

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

float fbm(vec3 pos, int octaves)  {
    float noiseSum = 0.0, frequency = 1.0, amplitude = 1.0;
    float ampSum = 0.0;
    
    for(int i = 0; i < octaves; ++i) {
        noiseSum += snoise(pos * frequency + vec3(i * 100.02341, 121 + i * 200.0354310, 121 + i * 150.02451)) * amplitude;
        ampSum += amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }

    return noiseSum;
}

#define NUM_SPHERES 50

void main() {
	ivec3 coords = ivec3(gl_GlobalInvocationID);
	vec3 nPos = (vec3(coords) / vec3(u_resolution) * 2.0 - 1.0) * u_targetSize + u_targetOffset;
	
	uint rngState = uint(uint(coords.x) * uint(1973) + uint(coords.y) * uint(12573) + uint(coords.z) * uint(9277) + uint(tseed * 100) * uint(26699)) | uint(1);
	uint rngState2 = 0;

	float minDist = 1000000.0;

	for (int i = 0; i < NUM_SPHERES; i++) {
		vec3 center = (vec3(RandomFloat01(rngState2), RandomFloat01(rngState2), RandomFloat01(rngState2)) * 2.0 - 1.0) * 25.;
		float radius = RandomFloat01(rngState2) * 4.0 + 0.5;

		float dist = length(nPos - center) - radius;

		if (dist < minDist) {
			minDist = dist;
		}
	}	

    float p = fbm(vec3(nPos) * 1.f, 4) * 0.5 + 0.5;
	p *= minDist < 0.0 ? 1.0f : exp(-minDist * 3.f);

	//p *= 1.0f - length(nPos);
	
	imageStore(img_output, coords, vec4(p));
}