#version 430
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

// In this 1D shader we traverse the histopyramid for each active voxel. 
// For each path from top to bottom of the pyramid, 8 neighbouring voxels are read in,
// to see if they are less than, equal to, or greater than the 1D index of the current thread's ID. 
// If the current voxel value is less than the target index, then the taregt voxel is further along than it. 
// If the current voxel is greater than the target index, then the target voxel in in a lower level of the pyramid.
// See the paper for a better explanation http://heim.ifi.uio.no/~erikd/pdf/hpmarcher_draft.pdf


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

layout(binding = 7) uniform sampler3D histoPyramidBaseLevelTexture;

uniform vec2 scaleVec;


// buffers 
//layout(std430, binding = 0) buffer posBuf
//{
//    // DONT USE VEC3 IN SSBO https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
//    vec4 pos [];
//};

layout(std430, binding = 1) buffer posBufEncode
{
    // DONT USE VEC3 IN SSBO https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
    uint posEncode [];
};

layout(std430, binding = 2) buffer normBuf
{
    vec4 norm [];
};

// uniforms
uniform int baseLevel;
uniform float isoLevel;
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
subroutine uniform launchSubroutine traverseHistoPyramidsSubroutine;

shared ivec3 currentShared = ivec3(0,0,0);
shared uint neighbors[8];

// current = ivec4 x y z sum
void scanHPLevel(uint target, int lod, inout uvec4 current)
{
    uint neighbors[8];

    if (lod == 0)
    {

        neighbors[0] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz)).x);
        neighbors[1] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[1].xyz)).x);
        neighbors[2] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[2].xyz)).x);
        neighbors[3] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[3].xyz)).x);

        neighbors[4] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[4].xyz)).x);
        neighbors[5] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[5].xyz)).x);
        neighbors[6] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[6].xyz)).x);
        neighbors[7] = uint(imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[7].xyz)).x);
    }
    else if (lod > 0 && lod <= 8 )
    {
        neighbors[0] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz), lod).x);
        neighbors[1] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[1].xyz), lod).x);
        neighbors[2] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[2].xyz), lod).x);
        neighbors[3] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[3].xyz), lod).x);

        neighbors[4] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[4].xyz), lod).x);
        neighbors[5] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[5].xyz), lod).x);
        neighbors[6] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[6].xyz), lod).x);
        neighbors[7] = uint(texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[7].xyz), lod).x);
    }
    

    uint acc = uint(current.w) + neighbors[0];

        uint cmp[8];

        cmp[0] = acc <= target ? 1 : 0;
        acc += neighbors[1];
        cmp[1] = acc <= target ? 1 : 0;
        acc += neighbors[2];
        cmp[2] = acc <= target ? 1 : 0;
        acc += neighbors[3];
        cmp[3] = acc <= target ? 1 : 0;
        acc += neighbors[4];
        cmp[4] = acc <= target ? 1 : 0;
        acc += neighbors[5];
        cmp[5] = acc <= target ? 1 : 0;
        acc += neighbors[6];
        cmp[6] = acc <= target ? 1 : 0;
        cmp[7] = 0;

        current += cubeOffsets[(cmp[0] + cmp[1] + cmp[2] + cmp[3] + cmp[4] + cmp[5] + cmp[6] + cmp[7])];
        current[0] = current[0] * 2;
        current[1] = current[1] * 2;
        current[2] = current[2] * 2;
        current[3] = current[3] +
            cmp[0] * neighbors[0] +
            cmp[1] * neighbors[1] +
            cmp[2] * neighbors[2] +
            cmp[3] * neighbors[3] +
            cmp[4] * neighbors[4] +
            cmp[5] * neighbors[5] +
            cmp[6] * neighbors[6] +
            cmp[7] * neighbors[7];
    


}

subroutine(launchSubroutine)
bool traverseHPLevel()
{
    ivec3 texSize = textureSize(histoPyramidTexture, 0);
    uint target = uint(gl_GlobalInvocationID.x);



    if (target >= totalSum)
    {
        target = 0;
    }

    uvec4 cubePosition = uvec4(0); // x y z sum

    if (texSize.x > 256)
    {
        scanHPLevel(target, 8, cubePosition);
    }
    if (texSize.x > 128)
    {
        scanHPLevel(target, 7, cubePosition);
    }
    if (texSize.x > 64)
    {
        scanHPLevel(target, 6, cubePosition);
    }

    scanHPLevel(target, 5, cubePosition);
    scanHPLevel(target, 4, cubePosition);
    scanHPLevel(target, 3, cubePosition);
    scanHPLevel(target, 2, cubePosition);
    scanHPLevel(target, 1, cubePosition);
    scanHPLevel(target, 0, cubePosition);

    cubePosition.x /= 2;
    cubePosition.y /= 2;
    cubePosition.z /= 2;

    int vertexNr = 0;

    //uvec4 cubeData = texelFetch(histoPyramidTexture, ivec3(cubePosition.xyz), 0);
    uint cubeIndex = uint(imageLoad(histoPyramidBaseLevel, ivec3(cubePosition.xyz)).y);

    // max 5 triangles 
    for (int i = int(target - cubePosition.w) * 3; i < int(target - cubePosition.w + 1) * 3; i++)
    { // for each vertex in triangle
        int edge = int(texelFetch(triTable, int(cubeIndex * 16 + i), 0).x);
        ivec3 point0 = ivec3(cubePosition.x + texelFetch(offsets3, edge * 6, 0).x, cubePosition.y + texelFetch(offsets3, edge * 6 + 1, 0).x, cubePosition.z + texelFetch(offsets3, edge * 6 + 2, 0).x);
        ivec3 point1 = ivec3(cubePosition.x + texelFetch(offsets3, edge * 6 + 3, 0).x, cubePosition.y + texelFetch(offsets3, edge * 6 + 4, 0).x, cubePosition.z + texelFetch(offsets3, edge * 6 + 5, 0).x);



        //// Store vertex in VBO

        vec3 forwardDifference0 = vec3(
                (-texelFetch(volumeFloatTexture, ivec3(point0.x + 1, point0.y, point0.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x - 1, point0.y, point0.z), 0).x),
                (-texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y + 1, point0.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y - 1, point0.z), 0).x),
                (-texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y, point0.z + 1), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y, point0.z - 1), 0).x)
            );

        vec3 forwardDifference1 = vec3(
                (-texelFetch(volumeFloatTexture, ivec3(point1.x + 1, point1.y, point1.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x - 1, point1.y, point1.z), 0).x),
                (-texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y + 1, point1.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y - 1, point1.z), 0).x),
                (-texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y, point1.z + 1), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y, point1.z - 1), 0).x)
            );


        float value0 = texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y, point0.z), 0).x;

        float testVal1 = texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y, point1.z), 0).x;

        
        float diff;


        diff = (isoLevel - value0) / (testVal1 - value0);
 
        vec3 vertex = mix(vec3(point0.x, point0.y, point0.z), vec3(point1.x, point1.y, point1.z), diff); // * scaing of voxels
        const vec3 normal = normalize(mix(forwardDifference0, forwardDifference1, diff));

        vertex = (vertex - scaleVec.x) / (scaleVec.y - scaleVec.x) * (1023); // 1023 since we are packing using 10 bits (1024 levels)



        // we would like to scale this so that you are not losing precision when we zoom in.
        // we need to cull verts that are outside of the viewing frustrum, therefore we need to find the viewing frustrum on the CPU then send its coords/plane eq as a uniform to the shader
        // currently we are always using a dynamic range of 0-1023 (actually its just uints 0 - 511)
        //posEncode[target * 3 + vertexNr] = uint(vertex.x) << 20 | uint(vertex.y) << 10 | uint(vertex.z);
        if (vertex.x < 1022 && vertex.y < 1022 && vertex.z < 1022 && vertex.x > 1 && vertex.y > 1 && vertex.z > 1)
        {
            posEncode[target * 3 + vertexNr] = uint(vertex.x) << 20 | uint(vertex.y) << 10 | uint(vertex.z);
            norm[target * 3 + vertexNr] = vec4(normal, 0.0f);

        }
        else
        {
            for (int vertN = 0; vertN < 3; vertN++)
            {
                posEncode[target * 3 + vertN] = 0;
                norm[target * 3 + vertN] = vec4(0.0f);
            }
            break;

        }


        // output normals here if we want to calc it here rather than in vertshader stage
        // norm[target * 3 + vertexNr * 3] = target;
        // norm[target * 3 + vertexNr * 3 + 1] = cubeIndex;
        // norm[target * 3 + vertexNr * 3 + 2] = vertexNr;

        ++vertexNr;
    }


    return true;
}

void main()
{
    bool done = traverseHistoPyramidsSubroutine();
}