#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// images
layout(binding = 0, r32f) uniform image3D volumeData;
layout(binding = 1, r32f) uniform image3D volumeDataOutput;
layout(binding = 2, rg16f) uniform image3D histoPyramidBaseLevel;
layout(binding = 3, r32f) uniform image3D histoPyramidBaseLevel32f;

//layout(binding = 3, r16f) uniform image3D testVolume;

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

//shared float fieldMatrix[9][9][9];
//shared uvec3 cornerOrigin;

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

    //if (localPos == uvec3(0,0,0))
    //{
    //    cornerOrigin = gl_GlobalInvocationID.xyz;
    //    //for (int i = 0; i < 5; i++)
    //    //{
    //    //    for (int j = 0; j < 5; j++)
    //    //    {
    //    //        for (int k = 0; k < 5; k++)
    //    //        {
    //    //            fieldMatrix[i][j][k] = 100;
    //    //        }
    //    //    }
    //    //}
    //}
    //barrier();

    //vec3 texSize = textureSize(volumeFloatTexture, 0);

    //vec3 testTexSize = textureSize(volumeFloatTexture, 1);

    //// make this a uniform in to set roi
    ////if (pos.x < 0 || pos.x >= texSize.x - 10 || pos.y <= 10 || pos.y >= texSize.y -10 || pos.z <= 10 || pos.z >= 300)
    ////{
    ////    return false;
    ////}

    //vec3 testReadPos = vec3(pos) / 2.0f + 1.0f;
    //testReadPos /= testTexSize;

    //float testValue = textureLod(volumeFloatTexture, testReadPos, 1.0f).x;
    uint cubeIndex;

    //// this filter will probably not be efficient if the volume is being updated over time as the mipmap will be expensve to compute per frame
    //// a simple filter, if we dont think there are any isolevel crossing voxel in the mipmap layer below, then theres no need to read all the voxels
    //// how can we use shared memory to reduce the amount of redundant memeory reads...
    //// each local worker adds 
    ////if (testValue >= isoLevel / 8.0f)
    ////{
    //fieldMatrix[localPos.x][localPos.y][localPos.z] = texelFetch(volumeFloatTexture, pos, 0).x;

    ////if (localPos == uvec3(0, 0, 0))
    ////{
    ////    for (int i = 0; i < 5; i++)
    ////    {
    ////        for (int j = 0; j < 5; j++)
    ////        {

    ////            fieldMatrix[4][i][j] = texelFetch(volumeFloatTexture, ivec3(pos.x + 4, pos.y + i, pos.z + j), 0).x;

    ////        }
    ////    }

    ////}
    ////// now do edge cases
    //uint sharedOffset = 8;
    //if (localPos.x == 0)        // do y/z plane (0-7)
    //{
    //    fieldMatrix[sharedOffset][localPos.y][localPos.z] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + sharedOffset, cornerOrigin.y + localPos.y, cornerOrigin.z + localPos.z), 0).x;
    //}
    //else if (localPos.x == 1)        // do x/y plane (0-7)
    //{
    //    fieldMatrix[localPos.y][localPos.z][sharedOffset] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + localPos.y, cornerOrigin.y + localPos.z, cornerOrigin.z + sharedOffset), 0).x;
    //}
    //else if (localPos.x == 2)      // do x / z plane (0 - 7)
    //{
    //    fieldMatrix[localPos.y][sharedOffset][localPos.z] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + localPos.y, cornerOrigin.y + sharedOffset, cornerOrigin.z + localPos.z), 0).x;
    //}
    //else if (localPos.x == 3) // do the 2 connecting edge strips
    //{
    //    if (localPos.y == 0)
    //    {
    //        fieldMatrix[localPos.z][sharedOffset][sharedOffset] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + localPos.z, cornerOrigin.y + sharedOffset, cornerOrigin.z + sharedOffset), 0).x;
    //    }
    //    else if (localPos.y == 1)
    //    {
    //        fieldMatrix[sharedOffset][localPos.z][sharedOffset] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + sharedOffset, cornerOrigin.y + localPos.z, cornerOrigin.z + sharedOffset), 0).x;
    //    }
    //    else if (localPos.y == 2)
    //    {
    //        fieldMatrix[sharedOffset][sharedOffset][localPos.z] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + sharedOffset, cornerOrigin.y + sharedOffset, cornerOrigin.z + localPos.z), 0).x;
    //    }
    //    else if (localPos.y == 3)
    //    {
    //        if (localPos.z == 1)
    //        {
    //            fieldMatrix[sharedOffset][sharedOffset][sharedOffset] = texelFetch(volumeFloatTexture, ivec3(cornerOrigin.x + sharedOffset, cornerOrigin.y + sharedOffset, cornerOrigin.z + sharedOffset), 0).x;

    //        }
    //    }

    //}



    //barrier();
    //memoryBarrierShared();

    //cubeIndex = uint(fieldMatrix[localPos.x][localPos.y][localPos.z] < isoLevel);
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[1].x][localPos.y + cubeOffsets[1].y][localPos.z + cubeOffsets[1].z] < isoLevel) * 2;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[2].x][localPos.y + cubeOffsets[2].y][localPos.z + cubeOffsets[2].z] < isoLevel) * 8;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[3].x][localPos.y + cubeOffsets[3].y][localPos.z + cubeOffsets[3].z] < isoLevel) * 4;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[4].x][localPos.y + cubeOffsets[4].y][localPos.z + cubeOffsets[4].z] < isoLevel) * 16;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[5].x][localPos.y + cubeOffsets[5].y][localPos.z + cubeOffsets[5].z] < isoLevel) * 32;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[6].x][localPos.y + cubeOffsets[6].y][localPos.z + cubeOffsets[6].z] < isoLevel) * 128;
    //cubeIndex += uint(fieldMatrix[localPos.x + cubeOffsets[7].x][localPos.y + cubeOffsets[7].y][localPos.z + cubeOffsets[7].z] < isoLevel) * 64;

    float field[8];
    field[0] = texelFetch(volumeFloatTexture, pos, 0).x;
    field[1] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[1].xyz), 0).x;
    field[2] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[2].xyz), 0).x;
    field[3] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[3].xyz), 0).x;
    field[4] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[4].xyz), 0).x;
    field[5] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[5].xyz), 0).x;
    field[6] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[6].xyz), 0).x;
    field[7] = texelFetch(volumeFloatTexture, pos + ivec3(cubeOffsets[7].xyz), 0).x;

    //float field[8];
    //field[0] = fieldMatrix[localPos.x][localPos.y][localPos.z];
    //field[1] = fieldMatrix[localPos.x + cubeOffsets[1].x][localPos.y + cubeOffsets[1].y][localPos.z + cubeOffsets[1].z];
    //field[2] = fieldMatrix[localPos.x + cubeOffsets[2].x][localPos.y + cubeOffsets[2].y][localPos.z + cubeOffsets[2].z];
    //field[3] = fieldMatrix[localPos.x + cubeOffsets[3].x][localPos.y + cubeOffsets[3].y][localPos.z + cubeOffsets[3].z];
    //field[4] = fieldMatrix[localPos.x + cubeOffsets[4].x][localPos.y + cubeOffsets[4].y][localPos.z + cubeOffsets[4].z];
    //field[5] = fieldMatrix[localPos.x + cubeOffsets[5].x][localPos.y + cubeOffsets[5].y][localPos.z + cubeOffsets[5].z];
    //field[6] = fieldMatrix[localPos.x + cubeOffsets[6].x][localPos.y + cubeOffsets[6].y][localPos.z + cubeOffsets[6].z];
    //field[7] = fieldMatrix[localPos.x + cubeOffsets[7].x][localPos.y + cubeOffsets[7].y][localPos.z + cubeOffsets[7].z];

    // https://stackoverflow.com/questions/43769622/bit-manipulation-to-store-multiple-values-in-one-int-c
    cubeIndex = uint(field[0] < isoLevel);
    cubeIndex += uint(field[1] < isoLevel) * 2;
    cubeIndex += uint(field[3] < isoLevel) * 4;
    cubeIndex += uint(field[2] < isoLevel) * 8;
    cubeIndex += uint(field[4] < isoLevel) * 16;
    cubeIndex += uint(field[5] < isoLevel) * 32;
    cubeIndex += uint(field[7] < isoLevel) * 64;
    cubeIndex += uint(field[6] < isoLevel) * 128;

    // }
    //else
    // {
    //     cubeIndex = 0;
    //}


    // uint numTri = texelFetch(nrOfTrianglesTable, int(cubeIndex), 0).x;
    uint numTri = nrOfTriangles[cubeIndex];
    //  float outputValue0 = numTri;
    //  float outputValue1 = cubeIndex * 65025.0f;

    //uint outVal = numTri << 16 | cubeIndex;
    //vec4 outVal = vec4(numTri, cubeIndex, 0, 0);

    imageStore(histoPyramidBaseLevel, pos, vec4(numTri, cubeIndex, 0, 0)); // this takes 6 ms, nearly 1/3 of whole running cost
    //imageStore(testVolume, pos, vec4(numTri));

    return true;

}




void main()
{
    bool done = histoPyramidsSubroutine();
}