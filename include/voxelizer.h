#ifndef VOXELIZER_H
#define VOXELIZER_H

#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

#include "glutils.h"
#include "glslprogram.h"
#include "glhelper.h"

#include <vector>
#include <iostream>
#include <fstream>


struct VoxelizerInfo {
	float unit;
	uint32_t n_triangles;
	float bbox_min;
	uint32_t gridsize;
};

class Voxelizer
{
public:
	Voxelizer() {};
	~Voxelizer() {};

	void configInfo(float un, uint32_t nTri, float bbmin, uint32_t grds)
	{
		m_info.unit = un;
		m_info.n_triangles = nTri;
		m_info.bbox_min = bbmin;
		m_info.gridsize = grds;
	}

	void init();
	void setVertexArray(std::vector<float>vData)
	{
		m_vertexData = vData;
	}
	void voxelize(bool first);

	GLuint getVolumeTexture()
	{
		return m_textureVolume;
	}



private:

	void compileAndLinkShader();
	void setLocations();
	void allocateTextures();
	void allocateBuffers();

	GLSLProgram voxelizerProg;
	//GLSLProgram insertTrianglesProg;

	std::vector<float> m_vertexData;

	GLuint m_textureVolume;
	GLuint m_infoSSBO;
	GLuint m_triangleData;

	VoxelizerInfo m_info;








};


























#endif