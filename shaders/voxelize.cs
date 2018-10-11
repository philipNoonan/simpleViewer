
//--------------------------------------------------------------------------------
// NVIDIA(R) GVDB VOXELS
// Copyright 2017, NVIDIA Corporation. 
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
//    in the documentation and/or  other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
//    from this software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Version 1.0: Rama Hoetzlein, 5/1/2017
//----------------------------------------------------------------------------------
// GVDB Points
// - ClearNodeCounts	- clear brick particle counts
// - InsertPoints		- insert points into bricks
// - SplatPoints		- splat points into bricks


// taken from https://github.com/NVIDIA/gvdb-voxels/blob/master/source/gvdb_library/kernels/cuda_gvdb_particles.cuh


#version 430
layout(local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0, std430) buffer infoBlock
{
    float unit;
    uint n_triangles;
    float bbox_min;
    uint gridsize;

} info;

layout(binding = 0, r32f) uniform image3D volumeData; 

layout(binding = 1, std430) buffer triangleBuffer
{
    float triangle_data[];
};

//subroutine void launchSubroutine();
//subroutine uniform launchSubroutine voxelizerSubroutine;

// Main triangle voxelization method
//subroutine(launchSubroutine)
void voxelize_triangle()
{
    uint thread_id = gl_GlobalInvocationID.x;

   // uint stride = gl_WorkgroupSize.x;

    // Common variables
    vec3 delta_p = vec3(info.unit, info.unit, info.unit);
    vec3 c = vec3(0.0f, 0.0f, 0.0f); // critical point

    if (thread_id < info.n_triangles) // just use global invocations rather than keep on looping?
    { // every thread works on specific triangles in its stride
        uint t = thread_id * 9; // triangle contains 9 vertices

        // COMPUTE COMMON TRIANGLE PROPERTIES
        vec3 v0 = vec3(triangle_data[t], triangle_data[t + 1], triangle_data[t + 2]) + vec3(61, 110, 37); // get v0 and move to origin
        vec3 v1 = vec3(triangle_data[t + 3], triangle_data[t + 4], triangle_data[t + 5]) + vec3(61, 110, 37); // get v1 and move to origin
        vec3 v2 = vec3(triangle_data[t + 6], triangle_data[t + 7], triangle_data[t + 8]) + vec3(61, 110, 37); // get v2 and move to origin
        vec3 e0 = v1 - v0;
        vec3 e1 = v2 - v1;
        vec3 e2 = v0 - v2;
        vec3 n = normalize(cross(e0, e1));

        //COMPUTE TRIANGLE BBOX IN GRID
        vec3 t_bbox_world_min = min(v0, min(v1, v2));
        vec3 t_bbox_world_max = max(v0, max(v1, v2));

        ivec3 t_bbox_grid_min;
        ivec3 t_bbox_grid_max;

        t_bbox_grid_min = ivec3(clamp(t_bbox_world_min / info.unit, vec3(0.0f, 0.0f, 0.0f), vec3(info.gridsize - 1, info.gridsize - 1, info.gridsize - 1)));
        t_bbox_grid_max = ivec3(clamp(t_bbox_world_max / info.unit, vec3(0.0f, 0.0f, 0.0f), vec3(info.gridsize - 1, info.gridsize - 1, info.gridsize - 1)));

        // PREPARE PLANE TEST PROPERTIES
        if (n.x > 0.0f) { c.x = info.unit; }
        if (n.y > 0.0f) { c.y = info.unit; }
        if (n.z > 0.0f) { c.z = info.unit; }
        float d1 = dot(n, (c - v0));
        float d2 = dot(n, ((delta_p - c) - v0));

        // PREPARE PROJECTION TEST PROPERTIES
        // XY plane
        vec2 n_xy_e0 = vec2(-1.0f * e0.y, e0.x);
        vec2 n_xy_e1 = vec2(-1.0f * e1.y, e1.x);
        vec2 n_xy_e2 = vec2(-1.0f * e2.y, e2.x);
        if (n.z < 0.0f)
        {
            n_xy_e0 = -n_xy_e0;
            n_xy_e1 = -n_xy_e1;
            n_xy_e2 = -n_xy_e2;
        }
        float d_xy_e0 = (-1.0f * dot(n_xy_e0, vec2(v0.x, v0.y))) + max(0.0f, info.unit * n_xy_e0[0]) + max(0.0f, info.unit * n_xy_e0[1]);
        float d_xy_e1 = (-1.0f * dot(n_xy_e1, vec2(v1.x, v1.y))) + max(0.0f, info.unit * n_xy_e1[0]) + max(0.0f, info.unit * n_xy_e1[1]);
        float d_xy_e2 = (-1.0f * dot(n_xy_e2, vec2(v2.x, v2.y))) + max(0.0f, info.unit * n_xy_e2[0]) + max(0.0f, info.unit * n_xy_e2[1]);
        // YZ plane
        vec2 n_yz_e0 = vec2(-1.0f * e0.z, e0.y);
        vec2 n_yz_e1 = vec2(-1.0f * e1.z, e1.y);
        vec2 n_yz_e2 = vec2(-1.0f * e2.z, e2.y);
        if (n.x < 0.0f)
        {
            n_yz_e0 = -n_yz_e0;
            n_yz_e1 = -n_yz_e1;
            n_yz_e2 = -n_yz_e2;
        }
        float d_yz_e0 = (-1.0f * dot(n_yz_e0, vec2(v0.y, v0.z))) + max(0.0f, info.unit * n_yz_e0[0]) + max(0.0f, info.unit * n_yz_e0[1]);
        float d_yz_e1 = (-1.0f * dot(n_yz_e1, vec2(v1.y, v1.z))) + max(0.0f, info.unit * n_yz_e1[0]) + max(0.0f, info.unit * n_yz_e1[1]);
        float d_yz_e2 = (-1.0f * dot(n_yz_e2, vec2(v2.y, v2.z))) + max(0.0f, info.unit * n_yz_e2[0]) + max(0.0f, info.unit * n_yz_e2[1]);
        // ZX plane
        vec2 n_zx_e0 = vec2(-1.0f * e0.x, e0.z);
        vec2 n_zx_e1 = vec2(-1.0f * e1.x, e1.z);
        vec2 n_zx_e2 = vec2(-1.0f * e2.x, e2.z);
        if (n.y < 0.0f)
        {
            n_zx_e0 = -n_zx_e0;
            n_zx_e1 = -n_zx_e1;
            n_zx_e2 = -n_zx_e2;
        }
        float d_xz_e0 = (-1.0f * dot(n_zx_e0, vec2(v0.z, v0.x))) + max(0.0f, info.unit * n_zx_e0[0]) + max(0.0f, info.unit * n_zx_e0[1]);
        float d_xz_e1 = (-1.0f * dot(n_zx_e1, vec2(v1.z, v1.x))) + max(0.0f, info.unit * n_zx_e1[0]) + max(0.0f, info.unit * n_zx_e1[1]);
        float d_xz_e2 = (-1.0f * dot(n_zx_e2, vec2(v2.z, v2.x))) + max(0.0f, info.unit * n_zx_e2[0]) + max(0.0f, info.unit * n_zx_e2[1]);

        // test possible grid boxes for overlap
        for (int z = t_bbox_grid_min.z; z <= t_bbox_grid_max.z; z++)
        {
            for (int y = t_bbox_grid_min.y; y <= t_bbox_grid_max.y; y++)
            {
                for (int x = t_bbox_grid_min.x; x <= t_bbox_grid_max.x; x++)
                {
                    // size_t location = x + (y*info.gridsize) + (z*info.gridsize*info.gridsize);
                    // if (checkBit(voxel_table, location)){ continue; }
                    // TRIANGLE PLANE THROUGH BOX TEST
                    vec3 p = vec3(x* info.unit, y* info.unit, z* info.unit);
                    float nDOTp = dot(n, p);
                    if ((nDOTp + d1) * (nDOTp + d2) > 0.0f) { continue; }

                    // PROJECTION TESTS
                    // XY
                    vec2 p_xy = vec2(p.x, p.y);
                    if ((dot(n_xy_e0, p_xy) + d_xy_e0) < 0.0f) { continue; }
                    if ((dot(n_xy_e1, p_xy) + d_xy_e1) < 0.0f) { continue; }
                    if ((dot(n_xy_e2, p_xy) + d_xy_e2) < 0.0f) { continue; }

                    // YZ
                    vec2 p_yz = vec2(p.y, p.z);
                    if ((dot(n_yz_e0, p_yz) + d_yz_e0) < 0.0f) { continue; }
                    if ((dot(n_yz_e1, p_yz) + d_yz_e1) < 0.0f) { continue; }
                    if ((dot(n_yz_e2, p_yz) + d_yz_e2) < 0.0f) { continue; }

                    // XZ	
                    vec2 p_zx = vec2(p.z, p.x);
                    if ((dot(n_zx_e0, p_zx) + d_xz_e0) < 0.0f) { continue; }
                    if ((dot(n_zx_e1, p_zx) + d_xz_e1) < 0.0f) { continue; }
                    if ((dot(n_zx_e2, p_zx) + d_xz_e2) < 0.0f) { continue; }

                    imageStore(volumeData, ivec3(x,y,z), vec4(1.0));


                    //atomicAdd(&voxel_count, 1);
                    //if (morton_order)
                    //{
                    //    //size_t location = mortonEncode_LUT(x, y, z);
                    //    //setBit(voxel_table, location);
                    //}
                    //else
                    //{
                    //    //size_t location = x + (y * info.gridsize) + (z * info.gridsize * info.gridsize);
                    //    //setBit(voxel_table, location);
                    //}
                    continue;
                }
            }
        }
        // sanity check: atomically count triangles
        //atomicAdd(&triangles_seen_count, 1);
        //thread_id += stride;
    }
}




//uniform uvec3 res;
//uniform vec3 vminExt;
//uniform vec3 vmaxExt;
//uniform float bdiv;
//uniform float bmax;

//uniform uint ecnt;
//uniform mat4 cxform;

//layout(std430, binding = 0) buffer ebufBuf
//{
//    uint ebuf [];
//};
//layout(std430, binding = 1) buffer vbufBuf
//{
//    vec4 vbuf [];
//};
//layout(std430, binding = 2) buffer tbufBuf
//{
//    vec4 tbuf [];
//};

//layout(std430, binding = 3) buffer bcntBuf
//{
//    uint bcnt [];
//};
//layout(std430, binding = 4) buffer boffBuf
//{
//    uint boff [];
//};


//subroutine void launchSubroutine();
//subroutine uniform launchSubroutine voxelizerSubroutine;




//void fminmax3 (float a, float b, float c, inout float mn, inout float mx)
//{
//    mn = mx = a;
//    if ( b < mn ) mn = b;
//    if ( b > mx ) mx = b;
//    if ( c < mn ) mn = c;
//    if ( c > mx ) mx = c;

//}

//void insertTriangles()
//{
//    uint n = gl_GlobalInvocationID.x;
//    if (n >= ecnt) return;

//    // get transformed triangle
//    vec4 v0, v1, v2;
//    ivec3 f = ivec3(ebuf[n * 3], ebuf[n * 3 + 1], ebuf[n * 3 + 2]);
//    v0 = vbuf[(f.x << 1)]; v0 = cxform * v0;
//    v1 = vbuf[(f.y << 1)]; v1 = cxform * v1;
//    v2 = vbuf[(f.z << 1)]; v2 = cxform * v2;

//    // compute bounds on y-axis	
//    float p0, p1;
//    fminmax3(v0.y, v1.y, v2.y, p0, p1);
//    p0 = int(p0 / bdiv); p1 = int(p1 / bdiv);                           // y-min and y-max bins

//    // scan bins covered by triangle	
//    for (int y = int(p0); y <= int(p1); y++)
//    {
//        atomicAdd(bcnt[y], uint(1));                           // histogram bin counts
//    }
//}


//// Sort triangles
//// Give a list of bins and known offsets (prefixes), and a list of vertices and faces,
//// performs a deep copy of triangles into bins, where some may be duplicated.
//// This may be used generically by others kernel that need a bin-sorted mesh.
//// Input: 
////   bdiv, bmax - input: bins division and maximum number
////   bcnt       - input: number of triangles in each bin
////   boff       - input: starting offset of each bin in triangle buffer
////   vcnt, vbuf - input: vertex buffer (VBO) and number of verts
////   ecnt, ebuf - input: element buffer and number of faces
////   tricnt     - output: total number of triangles when sorted into bins
////   tbuf       - output: triangle buffer: list of bins and their triangles (can be more than vcnt due to overlaps)
//void sortTriangles()
//{
//    uint n = gl_GlobalInvocationID.x;

//    if (n >= ecnt) return;

//    // get transformed triangle
//    vec4 v0, v1, v2;
//    ivec3 f = ivec3(ebuf[n * 3], ebuf[n * 3 + 1], ebuf[n * 3 + 2]);
//    v0 = vbuf[f.x << 1]; v0 = cxform * v0;
//    v1 = vbuf[f.y << 1]; v1 = cxform * v1;
//    v2 = vbuf[f.z << 1]; v2 = cxform * v2;

//    // compute bounds on y-axis	
//    float p0, p1;
//    fminmax3(v0.y, v1.y, v2.y, p0, p1);
//    p0 = int(p0 / bdiv); p1 = int(p1 / bdiv);                           // y-min and y-max bins
//    if (p0 >= bmax) p0 = bmax - 1;
//    if (p1 >= bmax) p1 = bmax - 1;

//    // scan bins covered by triangle	
//    uint bndx;
//    for (int y = int(p0); y <= int(p1); y++)
//    {
//        bndx = atomicAdd(bcnt[y], uint(1));        // get bin index (and histogram bin counts)
//        bndx += boff[y];                                // get offset into triangle buffer (tbuf)		
//        tbuf[bndx * 3] = v0;                            // deep copy transformed vertices of face
//        tbuf[bndx * 3 + 1] = v1;
//        tbuf[bndx * 3 + 2] = v2;
//    }
//}

//subroutine(launchSubroutine)
//void voxelize ()							
//{
//    uvec3 t = gl_GlobalInvocationID.xyz;
//    vec3 vmin = vminExt;
//    vec3 vmax = vmaxExt;

//	if ( t.x >= res.x || t.y >= res.y || t.z >= res.z ) return;

//	// solid voxelization
//	vec3 tdel = (vmax-vmin)/vec3(res);						// width of voxel
//	vmin += vec3(t.x+.5f, t.y+.5f, t.z+.5f)*tdel;		// center of voxel
//	vec3 v0, v1, v2;
//	vec3 e0, e1, e2;
//	vec3 norm, p;		
//	float rad;
//	uint n, cnt = 0;
//	int b = int(vmin.y / bdiv); // is this just a float to int floor? or should we + 0.5f it first?
//	if ( b >= bmax ) b = int(bmax-1); // same as above

//	for (n=boff[b]; n < boff[b]+bcnt[b]; n++ ) {

//		v0 = tbuf[n*3].xyz;   v0 = (v0 - vmin)/tdel;
//		v1 = tbuf[n*3+1].xyz; v1 = (v1 - vmin)/tdel;
//		v2 = tbuf[n*3+2].xyz; v2 = (v2 - vmin)/tdel;
//		/*f = make_int3( ebuf[n*3], ebuf[n*3+1], ebuf[n*3+2] );
//		v0 = vbuf[f.x << 1];		v0 = mul4x ( v0, cxform );	v0 = (v0 - tcent)/tdel;
//		v1 = vbuf[f.y << 1];		v1 = mul4x ( v1, cxform );	v1 = (v1 - tcent)/tdel;
//		v2 = vbuf[f.z << 1];		v2 = mul4x ( v2, cxform );	v2 = (v2 - tcent)/tdel;*/
//		e0 = v1-v0;	e1 = v2-v0;	

//		//--- bounding box test
//		fminmax3( v0.y, v1.y, v2.y, p.x, p.y );	
//		if ( p.x > 0.5f || p.y < -0.5f ) continue; 
//		fminmax3( v0.z, v1.z, v2.z, p.x, p.y );	
//		if ( p.x > 0.5f || p.y < -0.5f ) continue;		
//		fminmax3( v0.x, v1.x, v2.x, p.x, p.y );		
//		if ( p.y < -0.5f ) continue;				// x- half space, keep x+ half space

//		//--- ray-triangle intersect
//		norm.x = 0;		
//		e2 = vec3(0, -e1.z, e1.y);			// P = CROSS(D, e1)		  e2 <=> P,  D={1,0,0}
//		p.z = dot ( e0, e2 );						// det = DOT(e0, P)		  p.z <=> det
//		if ( p.z > -0.001 && p.z < 0.001 ) norm.x=1;		
//		// T=-v0;									// T = SUB(O, v0)         -v0 <=> T  O={0,0,0}
//		p.y = dot ( -v0, e2 ) / p.z;				// u = DOT(T, P)*invdet   p.y <=> u
//		if ( p.y < 0.f || p.y > 1.f ) norm.x=1;
//		e2 = cross ( -v0, e0 );						// Q = CROSS(T, e0)		  e2 <=> Q
//		rad = e2.x / p.z;							// v = DOT(D, Q)*invdet   rad <=> v
//		if ( rad < 0.f || p.y+rad > 1.f ) norm.x=1;
//		rad = dot ( e1, e2 ) / p.z;					// t = DOT(e1, Q)*invdet  rad <=> t
//		if ( rad < 0.001f ) norm.x=1;
//		if ( norm.x==0 ) cnt++;						// count crossing for inside-outside test (solid voxelize)

//		if ( p.x > 0.5f ) continue;					// x+ half space

//		//--- fast box-plane test
//		e2 = -e1; e1 = v2-v1;					
//		norm = cross ( e0, e1 );
//		p.x = 0; p.y = 0;	
//		if ( norm.x > 0.0f ) { p.x += norm.x*(-0.5f - v0.x); p.y += norm.x*( 0.5f - v0.x); }
//		else				 { p.x += norm.x*( 0.5f - v0.x); p.y += norm.x*(-0.5f - v0.x); }
//		if ( norm.y > 0.0f ) { p.x += norm.y*(-0.5f - v0.y); p.y += norm.y*( 0.5f - v0.y); }
//		else				 { p.x += norm.y*( 0.5f - v0.y); p.y += norm.y*(-0.5f - v0.y); }
//		if ( norm.z > 0.0f ) { p.x += norm.z*(-0.5f - v0.z); p.y += norm.z*( 0.5f - v0.z); }
//		else				 { p.x += norm.z*( 0.5f - v0.z); p.y += norm.z*(-0.5f - v0.z); }
//		if( p.x > 0.0f )		continue;	// do not overlap
//		if( p.y < 0.0f )		continue;

//		//--- schwarz-seidel tests
//		rad = (norm.z >= 0) ? 1 : -1;		
//		p = vec3( -e0.y*rad, e0.x*rad, 0 );	
//		if ( -(p.x+p.y)*0.5f - (p.x*v0.x + p.y*v0.y) + max(0, p.x) + max(0, p.y) < 0 ) continue; 	 // no overlap
//		p = vec3( -e1.y*rad, e1.x*rad, 0 ); 		
//		if ( -(p.x+p.y)*0.5f - (p.x*v1.x + p.y*v1.y) + max(0, p.x) + max(0, p.y) < 0 ) continue; 
//		p = vec3( -e2.y*rad, e2.x*rad, 0 );
//		if ( -(p.x+p.y)*0.5f - (p.x*v2.x + p.y*v2.y) + max(0, p.x) + max(0, p.y) < 0 ) continue; 

//		rad = (norm.y >= 0) ? -1 : 1;
//		p = vec3( -e0.z*rad, 0, e0.x*rad );	
//		if ( -(p.x+p.z)*0.5f - (p.x*v0.x + p.z*v0.z) + max(0, p.x) + max(0, p.z) < 0 ) continue; 	 // no overlap		
//		p = vec3( -e1.z*rad, 0, e1.x*rad ); 		
//		if ( -(p.x+p.z)*0.5f - (p.x*v1.x + p.z*v1.z) + max(0, p.x) + max(0, p.z) < 0 ) continue; 
//		p = vec3( -e2.z*rad, 0, e2.x*rad );
//		if ( -(p.x+p.z)*0.5f - (p.x*v2.x + p.z*v2.z) + max(0, p.x) + max(0, p.z) < 0 ) continue; 

//		rad = (norm.x >= 0) ? 1 : -1;		
//		p = vec3( 0, -e0.z*rad, e0.y*rad );	
//		if ( -(p.y+p.z)*0.5f - (p.y*v0.y + p.z*v0.z) + max(0, p.y) + max(0, p.z) < 0 ) continue; 	 // no overlap		
//		p = vec3( 0, -e1.z*rad, e1.y*rad ); 		
//		if ( -(p.y+p.z)*0.5f - (p.y*v1.y + p.z*v1.z) + max(0, p.y) + max(0, p.z) < 0 ) continue; 
//		p = vec3( 0, -e2.z*rad, e2.y*rad );
//		if ( -(p.y+p.z)*0.5f - (p.y*v2.y + p.z*v2.z) + max(0, p.y) + max(0, p.z) < 0 ) continue;

//		//--- akenine-moller tests
//		/*p.x = e0.z*v0.y - e0.y*v0.z;							// AXISTEST_X01(e0[Z], e0[Y], fez, fey);
//		p.z = e0.z*v2.y - e0.y*v2.z;
//		if (p.x<p.z) {min=p.x; max=p.z;} else {min=p.z; max=p.x;} 
//		rad = fabsf(e0.z) * 0.5f + fabsf(e0.y) * 0.5f;  
//		if (min>rad || max<-rad) continue;
//		p.x = -e0.z*v0.x + e0.x*v0.z;		      				// AXISTEST_Y02(e0.z, e0.x, fez, fex);
//		p.z = -e0.z*v2.x + e0.x*v2.z;
//		if (p.x<p.z) {min=p.x; max=p.z;} else {min=p.z; max=p.x;}
//		rad = fabsf(e0.z) * 0.5f + fabsf(e0.x) * 0.5f; 
//		if (min>rad || max<-rad) continue;
//		p.y = e0.y*v1.x - e0.x*v1.y;								// AXISTEST_Z12(e0.y, e0.x, fey, fex);
//		p.z = e0.y*v2.x - e0.x*v2.y;
//		if(p.z<p.y) {min=p.z; max=p.y;} else {min=p.y; max=p.z;}
//		rad = fabsf(e0.y) * 0.5f + fabsf(e0.x) * 0.5f;  
//		if(min>rad || max<-rad) continue;

//		p.x = e1.z*v0.y - e1.y*v0.z;							// AXISTEST_X01(e1.z, e1.y, fez, fey);
//		p.z = e1.z*v2.y - e1.y*v2.z;
//		if(p.x<p.z) {min=p.x; max=p.z;} else {min=p.z; max=p.x;} 
//		rad = fabsf(e1.z) * 0.5f + fabsf(e1.y) * 0.5f;
//		if(min>rad || max<-rad) continue;
//		p.x = -e1.z*v0.x + e1.x*v0.z;							// AXISTEST_Y02(e1.z, e1.x, fez, fex);
//		p.z = -e1.z*v2.x + e1.x*v2.z;
//		if(p.x<p.z) {min=p.x; max=p.z;} else {min=p.z; max=p.x;}
//		rad = fabsf(e1.z) * 0.5f + fabsf(e1.x) * 0.5f;
//		if(min>rad || max<-rad) continue;
//		p.x = e1.y*v0.x - e1.x*v0.y;								// AXISTEST_Z0(e1.y, e1.x, fey, fex);
//		p.y = e1.y*v1.x - e1.x*v1.y;
//		if(p.x<p.y) {min=p.x; max=p.y;} else {min=p.y; max=p.x;} 
//		rad = fabsf(e1.y) * 0.5f + fabsf(e1.x) * 0.5f;
//		if(min>rad || max<-rad) continue;

//		p.x = e2.z*v0.y - e2.y*v0.z;								// AXISTEST_X2(e2.z, e2.y, fez, fey);
//		p.y = e2.z*v1.y - e2.y*v1.z;
//		if(p.x<p.y) {min=p.x; max=p.y;} else {min=p.y; max=p.x;} 
//		rad = fabsf(e2.z) * 0.5f + fabsf(e2.y) * 0.5f; 
//		if(min>rad || max<-rad) continue;

//		p.x = -e2.z*v0.x + e2.x*v0.z;		      				// AXISTEST_Y1(e2.z, e2.x, fez, fex);
//		p.y = -e2.z*v1.x + e2.x*v1.z;
//		if(p.x<p.y) {min=p.x; max=p.y;} else {min=p.y; max=p.x;} 
//		rad = fabsf(e2.z) * 0.5f + fabsf(e2.x) * 0.5f;
//		if(min>rad || max<-rad) continue;

//		p.y = e2.y*v1.x - e2.x*v1.y;								// AXISTEST_Z12(e2.y, e2.x, fey, fex); 
//		p.z = e2.y*v2.x - e2.x*v2.y;
//		if(p.z<p.y) {min=p.z; max=p.y;} else {min=p.y; max=p.z;} 
//		rad = fabsf(e2.y) * 0.5f + fabsf(e2.x) * 0.5f;
//		if(min>rad || max<-rad) continue; */

//		////////switch ( otype ) {
//		////////case T_UCHAR:	obuf [ (t.z*res.y + t.y)*res.x + t.x ] = (uchar) val_surf;			break;
//		////////case T_FLOAT:	((float*) obuf) [ (t.z*res.y + t.y)*res.x + t.x ] = val_surf;		break;
//		////////case T_INT:		((int*) obuf) [ (t.z*res.y + t.y)*res.x + t.x ] = (int) val_surf;	break;
//		////////};		
//        ///
//        // WRITE FLAG TO BUFFER THAT THIS IS SURFACE, OR WRITE TO OCTREE

//		break;
//	}

//	if ( n == boff[b]+bcnt[b] ) {
//		// solid voxelization		
//		if ( cnt % 2 == 1) {
//            ////////switch ( otype ) {
//            ////////case T_UCHAR:	obuf [ (t.z*res.y + t.y)*res.x + t.x ] = (uchar) val_inside;		break;
//            ////////case T_FLOAT:	((float*) obuf) [ (t.z*res.y + t.y)*res.x + t.x ] = val_inside;		break;
//            ////////case T_INT:		((int*) obuf) [ (t.z*res.y + t.y)*res.x + t.x ] = (int) val_inside;	break;
//            ///        // WRITE FLAG TO BUFFER THAT THIS IS INTERNAL, OR WRITE TO OCTREE

//        //};		
//		}
//	}
//}







void main()
{
    voxelize_triangle();
    //voxelizerSubroutine();
}