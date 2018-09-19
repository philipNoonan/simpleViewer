#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout (location = 2) in vec3 position3D;
layout (location = 3) in vec3 texCoord3D;

layout (location = 4) in uint positionMC;
layout (location = 5) in vec4 normalMC;

layout (location = 6) in vec3 cubePoints;

layout (location = 7) in uint octlist;

uniform mat4 MVP;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 TexCoord3D;

out vec3 Norm;

subroutine vec4 getPosition();
subroutine uniform getPosition getPositionSelection;

subroutine(getPosition)
vec4 fromStandardTexture()
{
	TexCoord = vec2(texCoord.x, texCoord.y);
	return vec4(position.x, position.y, 0.0f, 1.0f);
	//return vec4(1);
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

	TexCoord3D = vertPoint / 512.0f;
	return vec4(MVP * vec4((vertPoint / 256.0f) - 1.0f, 1.0f));

	//TexCoord3D = vec3(positionMC.x / 512.0f, positionMC.y / 512.0f, positionMC.z / 512.0f);
	//return vec4(MVP * vec4((positionMC.x / 256.0f)- 1.0, (positionMC.y / 256.0f) - 1.0, (positionMC.z / 256.0f) - 1.0, 1.0f));
}

subroutine(getPosition)
vec4 fromOctlist()
{
	//TexCoord3D = vec3(texCoord3D.x, texCoord3D.y, texCoord3D.z);

	uint xPos = (octlist & 4286578688) >> 23;
	uint yPos = (octlist & 8372224) >> 14;
	uint zPos = (octlist & 16352) >> 5;
	uint lod = (octlist & 31);


	float octSideLength = float(pow(2, lod));
	vec3 origin = vec3(xPos, yPos, zPos) * octSideLength;

	mat4 transMat = mat4(1.0f);

	transMat[3] = vec4(origin.xyz / 256.0f - 1.0f, 1.0f);

	//transMat[3][3] = 1.0f;

	// shift cube to centre
	// rotate
	// shift cube back


	mat4 modelLocal = model * transMat;


	//modelLocal[3][0] += origin.x / 256.0f - 1.0f;
	//modelLocal[3][1] += origin.y / 256.0f - 1.0f;
	//modelLocal[3][2] += origin.z / 256.0f - 1.0f;

		TexCoord3D = vec3(-1);

	return vec4(projection * view * modelLocal * vec4(cubePoints, 1.0f / (octSideLength / 256.0f)));
	//  return vec4(projection * view * vec4(origin.xyz, 1.0f)); // can this be reduced to remove the * model, if we just multiply origin by lod
	//	return vec4(projection * view * model * vec4(origin.xyz, 1.0f)); // can this be reduced to remove the * model, if we just multiply origin by lod

}

void main()
{
	gl_Position = getPositionSelection();
}