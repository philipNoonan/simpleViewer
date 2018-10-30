#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout (location = 2) in vec3 position3D;
layout (location = 3) in vec3 texCoord3D;

layout (location = 4) in uint positionMC;
layout (location = 5) in vec4 normalMC;

layout (location = 6) in vec3 cubePoints;
layout (location = 7) in vec3 cubeNormals;

layout (location = 8) in uint octlist;

uniform mat4 MVP;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 TexCoord3D;

out vec3 Normal;
out vec3 FragPos;

out vec3 boxCenter;
out vec3 boxRadius;


subroutine vec4 getPosition();
subroutine uniform getPosition getPositionSelection;

subroutine(getPosition)
vec4 fromStandardTexture()
{
	TexCoord = vec2(texCoord.x, texCoord.y);
	return vec4(position.x, position.y, 0.0f, 1.0f);
}

subroutine(getPosition)
vec4 fromStandardTexture3D()
{
	TexCoord3D = vec3(texCoord3D.x, texCoord3D.y, texCoord3D.z);
	return vec4(MVP * vec4(position3D.x, position3D.y, position3D.z, 1.0f));
}

subroutine(getPosition)
vec4 fromStandardTextureMC()
{
	vec3 vertPoint = vec3((positionMC & 1072693248) >> 20, (positionMC & 1047552) >> 10, positionMC & 1023);
	Normal = normalMC.xyz;
	TexCoord3D = vertPoint / 512.0f;
	return vec4(MVP * vec4((vertPoint / 256.0f) - 1.0f, 1.0f));
}

// objectToScreen = projection * objectToWorld
// objectSpace Position = osPosition
//Fast Quadric Proj: "GPU-Based Ray-Casting of Quadratic Surfaces" http://dl.acm.org/citation.cfm?id=2386396
void quadricProj(in vec3 osPosition, in float voxelSize, in mat4 objectToScreenMatrix, in vec2 halfScreenSize, inout vec4 position, inout float pointSize) 
{
	const vec4 quadricMat = vec4(1.0, 1.0, 1.0, -1.0); // the diagonal elements of the quadric matrix of the unit sphere?
	float sphereRadius = voxelSize * 1.732051;
	vec4 sphereCenter = vec4(osPosition.xyz, 1.0);
	mat4 modelViewProj = transpose(objectToScreenMatrix);
		// matT is not quite the T matrix from the OG paper on quadric projection. It looks to be amalgamated into the P . M . T matricies. It is the variance matrix, it expresses the basis of the parameter space in object coordinates

	mat3x4 matT = mat3x4( mat3(modelViewProj[0].xyz, modelViewProj[1].xyz, modelViewProj[3].xyz) * sphereRadius);
	matT[0].w = dot(sphereCenter, modelViewProj[0]);
	matT[1].w = dot(sphereCenter, modelViewProj[1]);
	matT[2].w = dot(sphereCenter, modelViewProj[3]);
		// matD is the D matrix from the OG paper on quadric projection. Defines the quadric

	mat3x4 matD = mat3x4(matT[0] * quadricMat, matT[1] * quadricMat, matT[2] * quadricMat);

	// solving quadratic equation
	vec4 eqCoefs =
		vec4(dot(matD[0], matT[2]), dot(matD[1], matT[2]), dot(matD[0], matT[0]), dot(matD[1], matT[1]))
		/ dot(matD[2], matT[2]);

	vec4 outPosition = vec4(eqCoefs.x, eqCoefs.y, 0.0, 1.0);
	vec2 AABB = sqrt(eqCoefs.xy*eqCoefs.xy - eqCoefs.zw);
	
	AABB *= halfScreenSize * 2.0f;
	
	position.xy = outPosition.xy * position.w; // only the screen coords matter on the homogenous coord
	pointSize = max(AABB.x, AABB.y);
}

subroutine(getPosition)
vec4 fromOctlist()
{

	uint xPos = (octlist & 4286578688) >> 23;
	uint yPos = (octlist & 8372224) >> 14;
	uint zPos = (octlist & 16352) >> 5;
	uint lod = (octlist & 31);

	float octSideLength = float(pow(2, lod));
	vec3 origin = vec3(xPos, yPos, zPos) * octSideLength;

	mat4 transMat = mat4(1.0f);

	//shift into NDS
	transMat[3] = vec4(origin.xyz / 256.0f - 1.0f, 1.0f);
	TexCoord3D = vec3(origin.z, 0, -1);
	Normal = cubeNormals;
	FragPos = origin;//vec3(model * transMat * vec4(cubePoints, 1.0f / (octSideLength / 256.0f)));

	//gl_PointSize = octSideLength *2;
	//return vec4(MVP * transMat * vec4(cubePoints, 1.0f / (octSideLength / 256.0f)));

	float pointy;
	vec4 posy = vec4(0,0,0,1);

	// THIS BIT SHOULD BE + HALF A OCTLENGTH< MAYBE
	quadricProj(transMat[3].xyz, (octSideLength / 256.0f), MVP, vec2(512.0f), posy, pointy);

	 // Square area
	float stochasticCoverage = pointy * pointy;
	if ((stochasticCoverage < 0.8) &&
		((gl_VertexID & 0xffff) > stochasticCoverage * (0xffff / 0.8))) {
		// "Cull" small voxels in a stable, stochastic way by moving past the z = 0 plane.
		// Assumes voxels are in randomized order.
		posy = vec4(-1, -1, -1, -1);
	}

	gl_PointSize = pointy;

	boxRadius = vec3(pointy);
	boxCenter = vec3(origin);
	return vec4(posy);




	//  return vec4(projection * view * vec4(origin.xyz, 1.0f)); // can this be reduced to remove the * model, if we just multiply origin by lod
	//	return vec4(projection * view * model * vec4(origin.xyz, 1.0f)); // can this be reduced to remove the * model, if we just multiply origin by lod

}

void main()
{
	gl_Position = getPositionSelection();
}