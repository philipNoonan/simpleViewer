#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

// images
layout(binding = 0, r32f) uniform image3D volumeData;
layout(binding = 1, r32f) uniform image3D volumeDataOutput;
layout(binding = 2, rg16f) uniform image3D histoPyramidBaseLevel;

// textures
layout(binding = 0) uniform sampler3D histoPyramidTexture; 
layout(binding = 1) uniform sampler3D volumeFloatTexture; 

layout(binding = 3) uniform usampler1D edgeTable;
layout(binding = 4) uniform usampler1D triTable;
layout(binding = 5) uniform usampler1D nrOfTrianglesTable;
layout(binding = 6) uniform usampler1D offsets3;



// uniforms
uniform int hpLevel;
uniform float isoLevel = 1000.0f;
uniform uint totalSum;

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
    vec3 writePos = vec3(gl_GlobalInvocationID.xyz);
    vec3 readPos = writePos * 2 + 1.0f;
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
    readPos /= texSize;

    if (readPos.x > imSize.x || readPos.y > imSize.y || readPos.z > imSize.z)
    {
        return false;
    }

    float writeValue = 0.0f;

    // on level = zero, then we read from the texture view rg16f texture, other levels we read from the r32f texture. this is controled in c++ code
    writeValue = textureLod(histoPyramidTexture, readPos, hpLevel).x;

    imageStore(volumeDataOutput, ivec3(writePos), vec4(writeValue * 8.0f));

    //if (baseLevel == 0)
    //{
    //    writeValue = imageLoad(histoPyramidBaseLevel, readPos).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[1].xyz)).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[2].xyz)).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[3].xyz)).x +

    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[4].xyz)).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[5].xyz)).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[6].xyz)).x +
    //                     imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[7].xyz)).x;
    //}
    //else if (baseLevel > 0)
    //{
    //    if (baseLevel == 1)
    //    {
    //        writeValue = imageLoad(volumeData, readPos).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[1].xyz)).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[2].xyz)).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[3].xyz)).x +

    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[4].xyz)).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[5].xyz)).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[6].xyz)).x +
    //                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[7].xyz)).x;
    //    }
    //    else
    //    {
    //        float interpdval = float(texture(histoPyramidTexture, readPos, baseLevel - 1).x);
    //        writeValue = uint(interpdval.x * 8.0f);
    //    }


    //}

 

    //imageStore(volumeDataOutput, writePos, uvec4(writeValue));

    return true;
}



subroutine(launchSubroutine)
bool classifyCubes()
{
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 texSize = textureSize(volumeFloatTexture, 0);

    //if (pos.x < 0 || pos.x >= texSize.x - 10 || pos.y <= 10 || pos.y >= texSize.y -10 || pos.z <= 10 || pos.z >= texSize.z - 10)
    //{
    //    return false;
    //}

    float first = texelFetch(volumeFloatTexture, pos, 0).x;

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
    uint cubeIndex;
    cubeIndex = uint(field[0] < isoLevel);
    cubeIndex += uint(field[1] < isoLevel) * 2;
    cubeIndex += uint(field[3] < isoLevel) * 4;
    cubeIndex += uint(field[2] < isoLevel) * 8;
    cubeIndex += uint(field[4] < isoLevel) * 16;
    cubeIndex += uint(field[5] < isoLevel) * 32;
    cubeIndex += uint(field[7] < isoLevel) * 64;
    cubeIndex += uint(field[6] < isoLevel) * 128;

    uint numTri = texelFetch(nrOfTrianglesTable, int(cubeIndex), 0).x;

    imageStore(histoPyramidBaseLevel, pos, uvec4(numTri, cubeIndex, 0, 0));

    return true;

}




void main()
{
    bool done = histoPyramidsSubroutine();
}