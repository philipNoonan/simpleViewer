#ifndef MCUBES_H
#define MCUBES_H

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

#include <limits>


struct mCubeConfig
{
	glm::uvec3 gridSize;
	glm::uvec3 gridSizeMask;
	glm::uvec3 gridSizeShift;
	GLuint numVoxels;
	glm::vec3 voxelSize;
	float isoValue;
	GLuint maxVerts;
	float activeVoxels;

	mCubeConfig()
	{
		gridSize = glm::uvec3(128, 128, 128);
		gridSizeMask = glm::uvec3(gridSize.x - 1, gridSize.y - 1, gridSize.z - 1);
		gridSizeShift = glm::uvec3(0, log2(gridSize.x), log2(gridSize.y) + log2(gridSize.z));
		numVoxels = gridSize.x * gridSize.y * gridSize.z;
		voxelSize = glm::vec3(1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f);
		isoValue = 1650.0f;
		maxVerts = 128 * 128 * 128;

	}

};


class MCubes
{
public:
	MCubes() {};
	~MCubes() {};

	void init();

	void cleanup()
	{
		glDeleteTextures(1, &m_textureHistoPyramid);
		glDeleteTextures(1, &m_textureviewHistoPyramid);

		glDeleteBuffers(1, &m_bufferPos);


	}

	void setVolumeTexture(GLuint V)
	{
		m_textureVolume = V;
	}
	void setConfig(mCubeConfig MCC)
	{
		m_mcubeConfiguration = MCC;
	}
	void setIsolevel(float iso)
	{
		m_isoLevel = iso;
	}
	void generateMarchingCubes();
	GLuint getPosBuffer()
	{
		return m_bufferPosEncode;
	}
	int getNumberTriangles()
	{
		return m_totalSum;
	}
	void exportMesh();


private:
	GLSLProgram marchingCubesProg;
	GLSLProgram prefixSumProg;
	GLSLProgram histoPyramidsProg;
	GLSLProgram traverseHistoPyramidsProg;

	mCubeConfig m_mcubeConfiguration;

	// histopyramids
	GLuint m_histoPyramidsSubroutineID;
	GLuint m_traverseHistoPyramidsSubroutineID;
	GLuint m_classifyCubesID;
	GLuint m_constructHPLevelID;
	GLuint m_traverseHPLevelID;

	GLuint m_totalSumID;
	GLuint m_baseLevelID;
	GLuint m_isoLevel_TravID;

	// prefix sum
	GLuint m_prefixSumSubroutineID;
	GLuint m_resetSumsArrayID;
	GLuint m_forEachGroupID;
	GLuint m_forEveryGroupID;
	GLuint m_forFinalIncrementalSumID;

	// marching cubes
	GLuint m_marchingCubesSubroutineID;
	GLuint m_classifyVoxelID;
	GLuint m_compactVoxelsID;
	GLuint m_generateTrianglesID;

	GLuint m_gridSizeID;
	GLuint m_gridSizeShiftID;
	GLuint m_gridSizeMaskID;
	GLuint m_isoLevelID;
	GLuint m_numVoxelsID;
	GLuint m_activeVoxelsID;
	GLuint m_maxVertsID;
	GLuint m_voxelSizeID;

	// textures
	GLuint m_textureVolume;
	GLuint m_textureHistoPyramid;
	GLuint m_textureviewHistoPyramid;
	GLuint m_textureEdgeTable;
	GLuint m_textureTriTable;
	GLuint m_textureNumVertsTable;
	GLuint m_textureNrOfTriangles;
	GLuint m_textureOffsets3;

	GLuint m_testVolumeTexture;

	// buffers
	GLuint m_bufferVoxelVerts;
	GLuint m_bufferVoxelVertsScan;
	GLuint m_bufferVoxelOccupied;
	GLuint m_bufferVoxelOccupiedScan;
	GLuint m_bufferCompactedVoxelArray;
	GLuint m_bufferPos;
	GLuint m_bufferNorm;
	GLuint m_bufferPosEncode;
	GLuint m_bufferPrefixSumByGroup;


	// methods
	void compileAndLinkShader();
	void setLocations();
	//GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);
	void allocateTextures();
	void allocateBuffers();
	GLuint prefixSum(GLuint inputBuffer, GLuint outputBuffer);
	void loadTables();

	void histoPyramids();


	//inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

	GLuint query[2];


	uint32_t m_totalVerts;
	int m_totalSum = 0;
	float m_isoLevel = 1500.0f;






};

#endif
