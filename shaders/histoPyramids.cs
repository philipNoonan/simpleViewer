#version 430
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// This shader is called twice. Firstly we classify each voxel using classify cubes. 
// This takes the value at each corner of the voxel, and compares it to the threshold.
// The ouput of this stage is two numbers per voxel, whether the voxel is active,
// and the 0-255 cube index specifying the configuration of these vertices inside the active voxel
//
// The second call of this shader is to create the histopyramid by summing up the number of verts as outputted in the first call
// On the base level the texture is read as a two channel 16 bit float volume, but on higher levels
// we consider them as single channel 32 bit floats. This allows us to count more values than available in 16 bits

// images
layout(binding = 0, r32f) uniform image3D volumeData;
layout(binding = 1, r32f) uniform image3D volumeDataOutput;
layout(binding = 2, rg16f) uniform image3D histoPyramidBaseLevel;
layout(binding = 3, r32f) uniform image3D histoPyramidBaseLevel32f;

// textures
layout(binding = 0) uniform sampler3D histoPyramidTexture; 
layout(binding = 1) uniform sampler3D volumeFloatTexture; 

layout(binding = 3) uniform usampler1D edgeTable;
layout(binding = 4) uniform usampler1D triTable;
layout(binding = 5) uniform usampler1D nrOfTrianglesTable;
layout(binding = 6) uniform usampler1D offsets3;

// uniforms
uniform int hpLevel;
uniform int bottomLevel;

uniform float isoLevel = 1000.0f;
uniform uint totalSum;

uniform uint nrOfTriangles[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0,
};

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
subroutine uniform launchSubroutine histoPyramidsSubroutine;


subroutine(launchSubroutine)
bool constructHPLevel()
{
    int useInterpTextureSamplers = 1;
    
    vec3 writePos = vec3(gl_GlobalInvocationID.xyz);

    ivec3 imSize = ivec3(0);

    if (hpLevel == 0)
    {
        imSize = imageSize(histoPyramidBaseLevel);
    }
    else if (hpLevel > 0)
    {
        imSize = imageSize(volumeData);
    }

    vec3 texSize = vec3(textureSize(histoPyramidTexture, hpLevel));

    if (writePos.x > imSize.x || writePos.y > imSize.y || writePos.z > imSize.z)
    {
        return false;
    }

    float writeValue = 0.0f;

    // on level = zero, then we read from the texture view rg16f texture, other levels we read from the r32f texture. this is controled in c++ code
    
    if (useInterpTextureSamplers == 1)
    {
        vec3 readPos = writePos * 2 + 1.0f;
        readPos /= texSize;

        writeValue = textureLod(histoPyramidTexture, readPos, hpLevel).x;

        imageStore(volumeDataOutput, ivec3(writePos), vec4(writeValue * 8.0f));

    }
    else
    {
        ivec3 readPos = ivec3(writePos * 2);

        if (hpLevel == 0)
        {
            writeValue = imageLoad(histoPyramidBaseLevel, readPos).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[1].xyz)).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[2].xyz)).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[3].xyz)).x +

                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[4].xyz)).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[5].xyz)).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[6].xyz)).x +
                             imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[7].xyz)).x;
        }
        else if (hpLevel > 0)
        {

            writeValue = imageLoad(volumeData, readPos).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[1].xyz)).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[2].xyz)).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[3].xyz)).x +

                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[4].xyz)).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[5].xyz)).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[6].xyz)).x +
                             imageLoad(volumeData, readPos + ivec3(cubeOffsets[7].xyz)).x;
        }
               
        imageStore(volumeDataOutput, ivec3(writePos), vec4(writeValue));

    }

    return true;
}


// SHARED MEMORY ISNT FASTER< ACTUALLY SLOWER :( THE MAIN SLOW HERE IS THE IMAGE STORE TO THE LARGE HP VOLUMNE
subroutine(launchSubroutine)
bool classifyCubes()
{
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    uvec3 localPos = gl_LocalInvocationID.xyz;

    uint cubeIndex;
        
    float field[8];
    field[0] = texelFetch(volumeFloatTexture, pos, 0).x;
    field[1] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[1].xyz), 0).x;
    field[2] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[2].xyz), 0).x;
    field[3] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[3].xyz), 0).x;
    field[4] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[4].xyz), 0).x;
    field[5] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[5].xyz), 0).x;
    field[6] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[6].xyz), 0).x;
    field[7] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[7].xyz), 0).x;

    // https://stackoverflow.com/questions/43769622/bit-manipulation-to-store-multiple-values-in-one-int-c
    cubeIndex = uint(field[0] < isoLevel);
    cubeIndex += uint(field[1] < isoLevel) * 2;
    cubeIndex += uint(field[3] < isoLevel) * 4;
    cubeIndex += uint(field[2] < isoLevel) * 8;
    cubeIndex += uint(field[4] < isoLevel) * 16;
    cubeIndex += uint(field[5] < isoLevel) * 32;
    cubeIndex += uint(field[7] < isoLevel) * 64;
    cubeIndex += uint(field[6] < isoLevel) * 128;
    
    // uint numTri = texelFetch(nrOfTrianglesTable, int(cubeIndex), 0).x;
    uint numTri = nrOfTriangles[cubeIndex];
   
    imageStore(histoPyramidBaseLevel, pos, vec4(numTri, cubeIndex, 0, 0)); // this takes 6 ms, nearly 1/3 of whole running cost
    //imageStore(testVolume, pos, vec4(numTri));

    return true;

}




void main()
{
    bool done = histoPyramidsSubroutine();
}