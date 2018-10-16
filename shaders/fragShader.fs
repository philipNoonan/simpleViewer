#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTexture2D; 
layout (binding=1) uniform sampler3D currentTexture3D; 


in vec2 TexCoord;
in vec3 TexCoord3D;

in vec3 Normal;
in vec3 FragPos;

uniform mat4 view;



layout(location = 0) out vec4 color;

uniform int level = 0;
uniform float slice;
uniform vec3 sliceVals;

uniform vec3 lightPos = vec3(10.0f, 0.0f, 100.0f);
uniform vec3 lightColor = vec3(1.0f);


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
	//float alpha = outfloat > 0.1 ? 1 : 0;
	return vec4(outfloat.xxx, 1.0);

	//return vec4(1.0, 0.0, 0.0, 1.0);
}

subroutine(getColor)
vec4 fromVertexArray()
{
// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * vec3(1.0f);
// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
//specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(view[3].xyz - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 res = vec3(1.0f);

	//vec4 tData = 0.0001f * textureLod(currentTexture3D, vec3(TexCoord3D.x, TexCoord3D.y, TexCoord3D.z), float(level) );
    if (TexCoord3D.x < 0)
    {
		res = (ambient + diffuse + specular) * vec3(0.12f, 0.92f, 0.05f);
    }
    else
    {
		res = norm;//(ambient + diffuse + specular) * vec3(0.95f, 0.12f, 0.05f);
    }
	
	return vec4(res, 1.0f); 

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