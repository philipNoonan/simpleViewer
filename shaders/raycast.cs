#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0) uniform sampler3D volumeDataTexture;
layout(binding = 1) uniform sampler3D octTreeTexture;

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

uniform mat4 invView; // == raycast pose * invK
uniform mat4 invProj;
uniform mat4 invModel;

uniform float nearPlane;
uniform float farPlane;
uniform float step;
uniform float largeStep;
uniform vec3 volDim;
uniform vec3 volSize;
uniform uvec2 screenSize;

uniform int useOctree;
uniform float thresh;

uniform vec4 boxMaxs = vec4(1, 1, 1, 0.0f);
uniform vec4 boxMins = vec4(-1, -1, -1, 0.0f);

uint maxSteps = 1024;
float tStep = 1.0f / 512.0f;
float zNear = -1.0f;
float zFar = 1.0f;

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
bool intersectBox(vec4 r_o, vec4 r_d, vec4 boxmin, vec4 boxmax, inout float tnear, inout float tfar)
{
    // compute intersection of ray with all six bbox planes
    vec3 invR = vec3(1.0f) / r_d.xyz;
    vec3 tbot = invR * (boxmin.xyz - r_o.xyz);
    vec3 ttop = invR * (boxmax.xyz - r_o.xyz);

    // re-order intersections to find smallest and largest on each axis
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

    tnear = largest_tmin;
    tfar = smallest_tmax;

    return smallest_tmax > largest_tmin;
}

float sampleOctList(vec3 pos, int lod)
{
    return texelFetch(volumeDataTexture, ivec3(pos), lod).x;
}

void raytraceOctTree()
{
    int lod = 9;
    uvec2 pix = gl_GlobalInvocationID.xy;
    uvec2 imSize = imageSize(outVertex).xy;

    if ((pix.x >= imSize.x) || (pix.y >= imSize.y)) return;

    // NDS coords
    float u = (2.0 * float(pix.x)) / imSize.x - 1.0f;
    float v = (2.0 * float(pix.y)) / imSize.y - 1.0f;

    ivec4 boxmax0;
    ivec4 boxmin0;


    vec4 origin;
    vec4 direction;

    origin = (invModel * invView)[3];

    vec4 ray_eye = invProj * vec4(u, v, -1.0, 1.0f);

    ray_eye = vec4(ray_eye.xy, -1.0f, 0.0f);

    direction = normalize(invModel * invView * ray_eye);


    float tnear, tfar;
    vec4 volumecolor = vec4(0.0f);

    bool back = false;


    boxmax0 = ivec4(int(boxMaxs[0]), int(boxMaxs[1]), int(boxMaxs[2]), 0.0f);
    boxmin0 = ivec4(int(boxMins[0]), int(boxMins[1]), int(boxMins[2]), 0.0f);




    bool hit = intersectBox(origin, direction, boxmin0, boxmax0, tnear, tfar);

    float rayLength = distance(tfar, tnear);
    //float midpoint = rayLength / 2.0f;

    if (!hit)
    {
        imageStore(outVertex, ivec2(pix), volumecolor);
        return;
    }
    //else
    //{
    //    imageStore(outVertex, ivec2(pix), vec4(1.0f, 1.0, 1.0, midpoint));
    //    return;
    //}

    //float rayLength = distance(tfar, tnear);
    //float midpoint = rayLength / 2.0f;

    //  float t = tnear + midpoint;
    float t = tnear;// +midpoint;

    for (uint i = 0; i < maxSteps; i++)
    {
        t = tnear + rayLength / 2.0f;


        vec4 pos = origin + direction * t;

        vec4 texPos = (pos + 1.0) / 2.0f;

        if (texPos.x <= 0 || texPos.y <= 0 || texPos.z <= 0 || texPos.x >= 1 || texPos.y >= 1 || texPos.z >= 1)
        {
            //t += tStep;
            continue;
        }

        // change this into nearest neighhtbour or into texelfecth
        //float samp = textureLod(octTreeTexture, vec3(texPos.xyz), lod).x;
        float samp = texelFetch(octTreeTexture, ivec3(texPos.xyz * (512 >> lod)), lod).x; // i think this is fine, since we want to go to the corner of the voxel, i.e. the clipping from vec to ivec should do this for us


        if (samp == 0.0f)
        {
            float sampTex = texture(volumeDataTexture, vec3(texPos.xyz)).x; // i think this is fine, since we want to go to the corner of the voxel, i.e. the clipping from vec to ivec should do this for us

            // this assumes local homogeneity
            volumecolor += vec4(texPos.z, texPos.z, texPos.z, 1.0);
            //volumecolor += vec4(0.010 * float(lod));

            //tnear = tfar; // move to back face of voxel, and therefore front face of next voxel
            if (back)
            {
                lod++;
                back = true;
                tnear = tnear + rayLength;
                rayLength *= 2.0f;

            }
            else
            {

                tnear = tnear + rayLength;
                back = true;

            }

            //imageStore(outVertex, ivec2(pix), volumecolor);
            //return;

        }
        else if (samp == -1.0f)
        {
            if (back)
            {
                lod++;
                //back = true;
                tnear = tnear + rayLength;
                rayLength *= 2.0f;
            }
            else
            {
                tnear = tnear + rayLength;

                back = true;

            }
        }
        else // we are positive (or nan)
        {
            lod--;
            rayLength /= 2.0f;
            back = false;
        }

        // need to change the box max values too, change the name as well so it makes sense

        if (volumecolor.w >= 1.0f)
        {

            break;
        }
    }

    imageStore(outVertex, ivec2(pix), volumecolor);



}

void raycast()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    uvec2 imSize = imageSize(outVertex).xy;

    if ((pix.x >= imSize.x) || (pix.y >= imSize.y)) return;

    // NDS coords
    float u = (2.0 * float(pix.x)) / imSize.x - 1.0f;
    float v = (2.0 * float(pix.y)) / imSize.y - 1.0f;


    //float u = float(pix.x);
    //float v = float(pix.y);


    vec4 boxMax0 = vec4(boxMaxs[0], boxMaxs[1], boxMaxs[2], 0.0f);
    vec4 boxMin0 = vec4(boxMins[0], boxMins[1], boxMins[2], 0.0f);

    vec4 origin;
    vec4 direction;

    origin = (invModel * invView)[3]; // make these un inverted iinverseres


    // vec4 eyeRay_o = vec4(view[3][0], view[3][1], view[3][2], 0.0f);
    // eyeRay_d = vec4(rotate(view, vec3(pix.x, pix.y, 1.0f)), 1.0f);
    vec4 ray_eye = invProj * vec4(u, v, -1.0, 1.0f);
    // vec4 ray_eye = vec4(u, v, -1.0, 1.0f);

    ray_eye = vec4(ray_eye.xy, -1.0f, 0.0f);

    direction = normalize(invModel * invView * ray_eye);
    //eyeRay_d.w = 1.0f;
    //eyeRay_d = (eyeRay_d);


    float tNear, tFar;

    bool hit = intersectBox(origin, direction, boxMin0, boxMax0, tNear, tFar);

    if (!hit)
    {
        imageStore(outVertex, ivec2(pix), vec4(0.0f));
        return;
    }
    //else
    //{
    //    imageStore(outVertex, ivec2(pix), vec4(1.0f, 1.0, 1.0, 0.2f));
    //    //return;
    //}


    if (tNear < zFar) tNear = zNear; // clamp to near plane
    if (tFar > zFar) tFar = zFar; // clamp to far plane

    vec4 volumeColor = vec4(0.0f);

    float t = tNear; //back to front?

    for (uint i = 0; i < maxSteps; i++)
    {
        vec4 pos = origin + direction * t;
        vec4 texPos = (pos + 1.0) / 2.0f;

        if (texPos.x < 0 || texPos.y < 0 || texPos.z < 0 || texPos.x > 1 || texPos.y > 1 || texPos.z > 1)
        {
            t += tStep;
            continue;
        }
        float samp = texture(volumeDataTexture, vec3(texPos.xyz)).x;

        //volumeColor = mix(volumeColor, vec4(samp), 0.5f);
        if (samp > thresh)
        {
            //volumeColor += vec4(samp * 0.001f); // PROBLEM??
            volumeColor = vec4(texPos.z, texPos.z, texPos.z, 1.0);
        }
        if (volumeColor.w >= 1.0f) break; // PROBLEM??

        t += tStep;

        //if (t > tFar) break;


    }

    imageStore(outVertex, ivec2(pix), volumeColor);


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
    if (useOctree == 1)
    {
        raytraceOctTree();
    }
    else
    {
        raycast();
    }
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