#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding= 0) uniform sampler3D volumeDataTexture;
layout(binding= 1) uniform sampler3D octListTexture;

//layout(binding= 1) uniform sampler3D volumeColorTexture;
//layout(binding= 2) uniform sampler2D currentTextureColor;
     
layout(binding = 0, r32f) uniform image3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively
layout(binding = 1, rgba32f) uniform image2D outVertex;
layout(binding = 2, rgba32f) uniform image2D outNormal;
    //layout(binding = 1, r32f) uniform image2D volumeSlice;
//layout(binding = 3, rgba32f) uniform image2D volumeSliceNorm;
//layout(binding = 2, rgba32f) uniform image3D volumeColor;



//layout(std430, binding= 2) buffer posTsdf3D // ouput
//{
//    vec4 PositionTSDF [];
//};

//layout(std430, binding= 3) buffer norm3D // ouput
//{
//    vec4 NormalTSDF [];
//};

uniform int level = 8; //set me as dependent on texture size or minimum cutoff level

uniform mat4 view; // == raycast pose * invK
uniform float nearPlane;
uniform float farPlane;
uniform float step;
uniform float largeStep;
uniform vec3 volDim;
uniform vec3 volSize;

uniform vec4 boxMaxs = vec4(512.0, 512.0, 512.0, 0.0f);
uniform uint maxSteps = 512;
uniform float tStep = 2.0f;
uniform float zNear = 1.0f;
uniform float zFar = 2000.0f;

vec3 getVolumePosition(uvec3 p)
{
    return vec3((p.x + 0.5f) * volDim.x / volSize.x, (p.y + 0.5f) * volDim.y / volSize.y, (p.z + 0.5f) * volDim.z / volSize.z);
}

vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
}

vec3 opMul(mat4 M, vec3 v)
{
    return vec3(
        dot(vec3(M[0][0], M[1][0], M[2][0]), v) + M[3][0],
        dot(vec3(M[0][1], M[1][1], M[2][1]), v) + M[3][1],
        dot(vec3(M[0][2], M[1][2], M[2][2]), v) + M[3][2]);
}

float vs(uvec3 pos)
{  
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    //return data.x; // convert short to float

    vec4 data = imageLoad(volumeData, ivec3(pos));
    return float(data.x); // convert short to float

}

float interpVol(vec3 pos)
{
    vec3 scaled_pos = vec3((pos.x * volSize.x / volDim.x) - 0.5f, (pos.y * volSize.y / volDim.y) - 0.5f, (pos.z * volSize.z / volDim.z) - 0.5f);
    ivec3 base = ivec3(floor(scaled_pos));
    vec3 factor = fract(scaled_pos);
    ivec3 lower = max(base, ivec3(0));
    ivec3 upper = min(base + ivec3(1), ivec3(volSize) - ivec3(1));
    return (
          ((vs(uvec3(lower.x, lower.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, lower.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, lower.z)) * factor.x) * factor.y) * (1 - factor.z)
        + ((vs(uvec3(lower.x, lower.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, upper.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, upper.z)) * factor.x) * factor.y) * factor.z
        );
}



// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
bool intersectBox(vec4 r_o, vec4 r_d, vec4 boxmax, inout float tnear, inout float tfar)
{
    // compute intersection of ray with all six bbox planes
    vec4 invR = vec4(1.0f) / r_d;
    vec4 tbot = invR * -r_o;
    vec4 ttop = invR * (boxmax - r_o);

    // re-order intersections to find smallest and largest on each axis
    vec4 tmin = min(ttop, tbot);
    vec4 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

    tnear = largest_tmin;
    tfar = smallest_tmax;

    return smallest_tmax > largest_tmin;
}

float sampleOctList(vec3 pos, int lod)
{
    return samp = texelFetch(volumeDataTexture, ivec3(pos), lod).x;
}

void raytraceOctList()
{
    // using texelfetches sample the voxel at entry point + half length of voxel at current LOD

    // two cases to detect

    // if voxel is -1 then we have hit a leaf node, do functionality here, such as update color integral 

    // if voxel is > 0 then go down a level and repeat texelfetch

    // calculate exit point of voxel

    // early termination of ray tracing if certain threshold met. thresh tbd.

    // input to this shader is x,y pixel space coords, one thread per pixel

    int lod = level;
    vec2 pix = vec2(gl_GlobalInvocationID.xy);
    vec3 texSize = vec3(textureSize(volumeDataTexture, lod).xyz);

    if (pix.x >= texSize.x || pix.y >= texSize.y)
    {
        return;
    }

    float u = (((pix.x / float(texSize.x)) * 2.0f) - 1.0f) * 1.0f; // right // PROBLEM??
    float v = (((pix.y / float(texSize.y)) * 2.0f) - 1.0f) * 1.0f; // top

    ivec4 boxMax0;


    vec4 eyeRay_o;
    vec4 eyeRay_d;

    vec4 tempEyeRay_d = normalize(vec4(u, v, -zNear, 0.0f)); //-zNear???

    eyeRay_o = vec4(view[3][0], view[3][1], view[3][2], 0.0f); // PROBLEM??
    eyeRay_d.x = dot(tempEyeRay_d, vec4(view[0][0], view[1][0], view[2][0], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.y = dot(tempEyeRay_d, vec4(view[0][1], view[1][1], view[2][1], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.z = dot(tempEyeRay_d, vec4(view[0][2], view[1][2], view[2][2], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.w = 1.0f;

    float tNear, tFar;
    vec4 volumeColor = vec4(0.0f);


    while (tFar < boxMax0[2]) // fixme
    {
        boxMax0 = vec4(int(boxMaxs[0]) >> lod, int(boxMaxs[1]) >> lod, int(boxMaxs[2]) >> lod, 0.0f);
        
        bool hit = intersectBox(eyeRay_o, eyeRay_d, boxMax0, tNear, tFar);

        if (!hit)
        {
            imageStore(outVertex, ivec2(pix), volumeColor);
            return;
        }

        float t = tNear + float(int(boxMaz0[0]) >> lod) / 2.0f; //back to front?

        vec4 pos = eyeRay_o + eyeRay_d * t;

        float samp = sampleOctList(pos, lod);

        if (samp == -1.0f)
        {
            volumeColor.xyz += vec3(0.1);
        }
        else
        {
            lod--;
        }

        tNear = tFar; // move to back face of voxel, and therefore front face of next voxel

        // need to change the box max values too, change the name as well so it makes sense

    if (volumeColor.w >= 1.0f)
        {
            imageStore(outVertex, ivec2(pix), volumeColor);
            return;
        }
    }

   


}

void raycast()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    uvec2 imSize = imageSize(outVertex).xy;

    if ((pix.x >= imSize.x) || (pix.y >= imSize.y)) return;
    //uint outputIndex = (pix.y * imSize.x) + pix.x;

    float u = (((pix.x / float(imSize.x)) * 2.0f) - 1.0f) * 1.0f; // right // PROBLEM??
    float v = (((pix.y / float(imSize.y)) * 2.0f) - 1.0f) * 1.0f; // top

    vec4 boxMax0 = vec4(boxMaxs[0], boxMaxs[1], boxMaxs[2], 0.0f);

    vec4 eyeRay_o;
    vec4 eyeRay_d;

    vec4 tempEyeRay_d = normalize(vec4(u, v, -zNear, 0.0f)); //-zNear???

    eyeRay_o = vec4(view[3][0], view[3][1], view[3][2], 0.0f); // PROBLEM??
    eyeRay_d.x = dot(tempEyeRay_d, vec4(view[0][0], view[1][0], view[2][0], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.y = dot(tempEyeRay_d, vec4(view[0][1], view[1][1], view[2][1], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.z = dot(tempEyeRay_d, vec4(view[0][2], view[1][2], view[2][2], 0.0f)); // PROBLEM?? // PROBLEM??
    eyeRay_d.w = 1.0f;

    //eyeRay_o = vec4(view[0][3], view[1][3], view[2][3], 0.0f); // PROBLEM??
    //eyeRay_d.x = dot(tempEyeRay_d, vec4(view[0][0], view[0][1], view[0][2], 0.0f)); // PROBLEM?? // PROBLEM??
    //eyeRay_d.y = dot(tempEyeRay_d, vec4(view[1][0], view[1][1], view[1][2], 0.0f)); // PROBLEM?? // PROBLEM??
    //eyeRay_d.z = dot(tempEyeRay_d, vec4(view[2][0], view[2][1], view[2][2], 0.0f)); // PROBLEM?? // PROBLEM??
    //eyeRay_d.w = 1.0f;


    // vec4 eyeRay_o = vec4(view[3][0], view[3][1], view[3][2], 0.0f);
    // vec4 eyeRay_d = vec4(rotate(view, vec3(pix.x, pix.y, 1.0f)), 1.0f);

    float tNear, tFar;

    bool hit = intersectBox(eyeRay_o, eyeRay_d, boxMax0, tNear, tFar);

    if (!hit)
    {
        imageStore(outVertex, ivec2(pix), vec4(0.0f));
        return;
    }

    if (tNear < zFar) tNear = zNear; // clamp to near plane
    if (tFar > zFar) tFar = zFar; // clamp to far plane

    vec4 volumeColor = vec4(0.0f);

    float t = tNear; //back to front?

    for (uint i = 0; i < maxSteps; i++)
    {
        vec4 pos = eyeRay_o + eyeRay_d * t;

        float samp = texture(volumeDataTexture, vec3(pos.x / 512.0f, pos.y / 512.0f, pos.z / 512.0f)).x;

        //volumeColor = mix(volumeColor, vec4(samp), 0.5f);
        if (samp > 1300.0f)
        {
            volumeColor += vec4(samp * 0.0005f); // PROBLEM??
        }
        if (volumeColor.x > 100.0f) break; // PROBLEM??

        t += tStep;

        if (t < tNear) break;


    }

    imageStore(outVertex, ivec2(pix), volumeColor / 100.0f);


}



vec3 getGradient(vec4 hit)
{
    vec3 scaled_pos = vec3((hit.x * volSize.x / volDim.x) - 0.5f, (hit.y * volSize.y / volDim.y) - 0.5f, (hit.z * volSize.z / volDim.z) - 0.5f);
    ivec3 baseVal = ivec3(floor(scaled_pos));
    vec3 factor = fract(scaled_pos);
    ivec3 lower_lower = max(baseVal - ivec3(1), ivec3(0));
    ivec3 lower_upper = max(baseVal, ivec3(0));
    ivec3 upper_lower = min(baseVal + ivec3(1), ivec3(volSize) - ivec3(1));
    ivec3 upper_upper = min(baseVal + ivec3(2), ivec3(volSize) - ivec3(1));
    ivec3 lower = lower_upper;
    ivec3 upper = upper_lower;

    vec3 gradient;

    gradient.x =
              (((vs(uvec3(upper_lower.x, lower.y, lower.z)) - vs(uvec3(lower_lower.x, lower.y, lower.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, lower.z)) - vs(uvec3(lower_upper.x, lower.y, lower.z))) * factor.x) * (1 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, lower.z)) - vs(uvec3(lower_lower.x, upper.y, lower.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, lower.z)) - vs(uvec3(lower_upper.x, upper.y, lower.z))) * factor.x) * factor.y) * (1 - factor.z)
            + (((vs(uvec3(upper_lower.x, lower.y, upper.z)) - vs(uvec3(lower_lower.x, lower.y, upper.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, upper.z)) - vs(uvec3(lower_upper.x, lower.y, upper.z))) * factor.x) * (1 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, upper.z)) - vs(uvec3(lower_lower.x, upper.y, upper.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, upper.z)) - vs(uvec3(lower_upper.x, upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.y =
          (((vs(uvec3(lower.x, upper_lower.y, lower.z)) - vs(uvec3(lower.x, lower_lower.y, lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, lower.z)) - vs(uvec3(upper.x, lower_lower.y, lower.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, lower.z)) - vs(uvec3(lower.x, lower_upper.y, lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, lower.z)) - vs(uvec3(upper.x, lower_upper.y, lower.z))) * factor.x) * factor.y) * (1 - factor.z)
        + (((vs(uvec3(lower.x, upper_lower.y, upper.z)) - vs(uvec3(lower.x, lower_lower.y, upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, upper.z)) - vs(uvec3(upper.x, lower_lower.y, upper.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, upper.z)) - vs(uvec3(lower.x, lower_upper.y, upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, upper.z)) - vs(uvec3(upper.x, lower_upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.z =
          (((vs(uvec3(lower.x, lower.y, upper_lower.z)) - vs(uvec3(lower.x, lower.y, lower_lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_lower.z)) - vs(uvec3(upper.x, lower.y, lower_lower.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_lower.z)) - vs(uvec3(lower.x, upper.y, lower_lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_lower.z)) - vs(uvec3(upper.x, upper.y, lower_lower.z))) * factor.x) * factor.y) * (1 - factor.z)
        + (((vs(uvec3(lower.x, lower.y, upper_upper.z)) - vs(uvec3(lower.x, lower.y, lower_upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_upper.z)) - vs(uvec3(upper.x, lower.y, lower_upper.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_upper.z)) - vs(uvec3(lower.x, upper.y, lower_upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_upper.z)) - vs(uvec3(upper.x, upper.y, lower_upper.z))) * factor.x) * factor.y) * factor.z;

    return gradient * vec3(volDim.x / volSize.x, volDim.y / volSize.y, volDim.z / volSize.z) * (0.5f * 0.00003051944088f);


}

void main()
{
    raycast();
    //raytraceOctList();
}










// USEFUL SANITY CHECKING STUFF

// vec4 interpData = texture(volumeDataTexture, vec3(pix / 256.0f, 2)); // texture float reads are from 0 - 1
//vec4 interpData = texelFetch(volumeDataTexture, ivec3(pix, 0), 0);
// vec4 interpData = imageLoad(volumeData, ivec3(pix, 0));

//if (interpData.x > -5.0f && interpData.x < 0.0f)
//{
//    imageStore(volumeSlice, ivec2(pix), vec4(0.5f, 0, 0, 0));
//}

//if (interpData.x > 0.0f && interpData.x < 5.0f)
//{
//    imageStore(volumeSlice, ivec2(pix), vec4(0.25f, 0, 0, 0));
//}