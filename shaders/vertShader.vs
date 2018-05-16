#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout (location = 2) in vec3 position3D;
layout (location = 3) in vec3 texCoord3D;

layout (location = 4) in vec4 positionMC;
layout (location = 5) in vec4 normalMC;

uniform mat4 MVP;

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
	TexCoord3D = vec3(positionMC.x / 512.0f, positionMC.y / 512.0f, positionMC.z / 512.0f);
	return vec4(MVP * vec4((positionMC.x / 256.0f)- 1.0, (positionMC.y / 256.0f) - 1.0, (positionMC.z / 256.0f) - 1.0, 1.0f));
}

void main()
{
	gl_Position = getPositionSelection();
}