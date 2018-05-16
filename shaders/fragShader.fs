#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTexture2D; 
layout (binding=1) uniform sampler3D currentTexture3D; 


in vec2 TexCoord;
in vec3 TexCoord3D;
in vec3 Norm;

layout(location = 0) out vec4 color;

uniform int level = 0;
uniform float slice;
uniform vec3 sliceVals;

uniform struct Light {
	vec3 position;
	vec3 intensities;
	float attenuation;
	float ambientCoefficient;
} light;




subroutine vec4 getColor();
subroutine uniform getColor getColorSelection;

subroutine(getColor)
vec4 fromVolume()
{
	vec4 tData = textureLod(currentTexture3D, vec3(TexCoord3D.x, TexCoord3D.y, TexCoord3D.z), float(level) );
	//vec4 tData = imageLoad(volumeData, vec3(TexCoord.x * 512.0f, TexCoord.y * 512.0f, slice));
	float outfloat = tData.x > 100 ? tData.x * 0.0005f : 0;
	return vec4(outfloat.xxx, 1.0f);

	//return vec4(1.0, 0.0, 0.0, 1.0);
}

subroutine(getColor)
vec4 fromVertexArray()
{
	//vec4 tData = 0.0001f * textureLod(currentTexture3D, vec3(TexCoord3D.x, TexCoord3D.y, TexCoord3D.z), float(level) );
	return vec4(0.95f, 0.12f, 0.05f, 1.0f); 

}

subroutine(getColor)
vec4 fromTexture2D()
{
	vec4 tData = texture(currentTexture2D, vec2(TexCoord.x, TexCoord.y));
	return vec4(tData.xyzw); 

}


void main()
{
	//vec3 normals = normalize(cross(dFdx(vert4D.xyz), dFdy(vert4D.xyz)));



	color = getColorSelection();

}