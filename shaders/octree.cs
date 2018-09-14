#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; // 

// images
layout(binding = 0, r32f) uniform image3D hpVolumeOutput;
// textures
layout(binding = 0) uniform sampler3D hpVolumeTexture; 
layout(binding = 1) uniform sampler3D originalDataVolumeTexture; 

//uniform int baseLevel;
uniform int hpLevel;
uniform float isoLevel;
uniform uint cutoff;

uniform uvec4 cubeOffsets[8] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 1, 0},    
 };
   
subroutine bool launchSubroutine();
subroutine uniform launchSubroutine hpOctreeSubroutine;

subroutine(launchSubroutine)
bool hpDiscriminator()
{
    ivec3 readPix = ivec3(gl_GlobalInvocationID.xyz);
    //vec3 texSize = vec3(textureSize(originalDataVolumeTexture, 2));

    float inputValue = texelFetch(originalDataVolumeTexture, readPix, 0).x;

   // float inputValue = textureLod(originalDataVolumeTexture, readPix / texSize, 2).x;

    float writeValue;

    if (inputValue < isoLevel)
    {
        writeValue = -1.0f;
    }
    else
    {
        writeValue = 0.0f;
    }

    imageStore(hpVolumeOutput, readPix, vec4(writeValue));

    return true;

}

subroutine(launchSubroutine)
bool hpBuilder()
{
    vec3 writePos = vec3(gl_GlobalInvocationID.xyz);
    vec3 readPix = writePos * 2;

    vec3 texSize = vec3(textureSize(hpVolumeTexture, hpLevel));


    // we can use the linear interp trick here, if we have a pre-mipmapped image volume, read one value from the middle of all 4, if its -1, then all 4 are minus 1, if its all 0, then all are zero
    // if its -0.5 then the ouput is 2
    float inputValue;

    vec3 readPosTexture = writePos.xyz * 2.0f + 1.0f;
    readPosTexture /= texSize;
    inputValue = textureLod(hpVolumeTexture, readPosTexture, hpLevel).x;
    float writeValue;

    if (hpLevel != 0 && hpLevel == cutoff)
    {
        if (inputValue == -1.0) // all neighbours are -1
        {
            writeValue = -1.0f;
        }
        else
        {
            writeValue = 0.0f;
        }
    }
    else
    {
        if (inputValue == -1.0) // all neighbours are -1
        {
            writeValue = -1.0f;
        }
        else // take the abs of the neighbours and sum them
        {
            vec4 oneVec = vec4(1.0f);
            vec4 inputVecLo, inputVecHi;
            inputVecLo.x = texelFetch(hpVolumeTexture, ivec3(readPix), hpLevel).x;
            inputVecLo.y = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[1].xyz), hpLevel).x;
            inputVecLo.z = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[2].xyz), hpLevel).x;
            inputVecLo.w = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[3].xyz), hpLevel).x;

            inputVecHi.x = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[4].xyz), hpLevel).x;
            inputVecHi.y = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[5].xyz), hpLevel).x;
            inputVecHi.z = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[6].xyz), hpLevel).x;
            inputVecHi.w = texelFetch(hpVolumeTexture, ivec3(readPix) + ivec3(cubeOffsets[7].xyz), hpLevel).x;

            writeValue = dot(oneVec, abs(inputVecLo)) + dot(oneVec, abs(inputVecHi)); // faster way to sum? just takes one cycle on gpu, pub abs here or on each line above?
        }
    }
    



    imageStore(hpVolumeOutput, ivec3(writePos.xyz), vec4(writeValue));

    return true;
}

void main()
{
    bool done = hpOctreeSubroutine();


}