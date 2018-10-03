#ifndef OCTREE_H
#define OCTREE_H

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

class Octree
{
public:
	Octree() {};
	~Octree() {};

	void init();

	void setInputFloatVolume(GLuint volumeData)
	{
		m_texture_hpOriginalData = volumeData;
	}
	GLuint getOctlistBuffer()
	{
		return m_bufferPosEncode;
	}
	GLuint getOctreeTexture()
	{
		return m_texture_hpOctree;
	}
	int getLength()
	{
		return m_totalSum;
	}
	void setIsoLevel(float level)
	{
		m_isoLevel = level;
	}
	void setCutoff(int cOff)
	{
		m_cutoff = cOff;
	}

	void buildTree();
	void createList();

	void cleanup()
	{
		glDeleteTextures(1, &m_texture_hpOctree);
		glDeleteBuffers(1, &m_bufferPos);
	}
private:

	void compileAndLinkShader();
	void setLocations();
	void allocateTextures();
	void allocateBuffers();

	GLSLProgram octreeProg;
	GLSLProgram octlistProg;

	GLuint m_subroutine_hpOctreeID;
	GLuint m_hpDiscriminatorID;
	GLuint m_hpBuilderID;

	GLuint m_hpLevelID;

	GLuint m_subroutine_hpOctlistID;
	GLuint m_traverseHPLevelID;
	GLuint m_totalSumID;
	GLuint m_isoLevelID;
	GLuint m_cutoffID;
	GLuint m_cutoffTreeID;

	// textures
	GLuint m_texture_hpOctree;
	GLuint m_texture_hpOriginalData;

	// buffers
	GLuint m_bufferPos;
	GLuint m_bufferPosEncode;

	int m_totalSum;
	float m_isoLevel = 2000.0f;
	uint32_t m_cutoff;

	GLuint query[2];


};

#endif