#ifndef RAYCATSTER_H
#define RAYCATSTER_H

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


#include "glutils.h"
#include "glslprogram.h"
#include "glhelper.h"

#include <vector>
#include <iostream>
#include <fstream>


//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"


class RCaster
{
public:
	RCaster() {};
	~RCaster() {};

	void init();

	void raycast();

	void setVolumeTexture(GLuint V) 
	{
		m_textureVolume = V;
	}
	void setOctreeTexture(GLuint oct)
	{
		m_textureOctree = oct;
	}
	void setVertexTexture(GLuint vB)
	{
		m_textureVertices = vB;
	}
	GLuint getVertexTexture()
	{
		return m_textureVertices;
	}
	void setNormalTexture(GLuint nB)
	{
		m_textureNormals = nB;
	}
	void setInverseProjection(glm::mat4 invProj)
	{
		m_invProj = invProj;
	}
	void setView(glm::mat4 view)
	{
		m_view = view;
	}
	void setInverseView(glm::mat4 invView)
	{
		m_invView = invView;
	}
	void setInverseModel(glm::mat4 invModel)
	{
		m_invModel = invModel;
	}
	void setScreenWidth(int w)
	{
		m_screenWidth = w;
	}	
	void setScreenHeight(int h)
	{
		m_screenHeight = h;
	}
	void setFastRaytraceFlag(bool flg)
	{
		m_useOctree = flg;
	}
	void setThresh(float thr)
	{
		m_thresh = thr;
	}
private:

	GLSLProgram raycastProg;

	void compileAndLinkShader();
	void setLocations();
	void allocateTextures();
	void allocateBuffers();


	// locations
	GLuint m_invModelID_r;
	GLuint m_invViewID_r;
	GLuint m_invProjID_r;

	GLuint m_nearPlaneID;
	GLuint m_farPlaneID;
	GLuint m_stepID;
	GLuint m_largeStepID;
	GLuint m_volDimID_r;
	GLuint m_volSizeID_r;
	GLuint m_screenSizeID;
	GLuint m_helpersSubroutineID;

	GLuint m_threshID;

	GLuint m_useOctreeID;

	// textures
	GLuint m_textureVolume;
	GLuint m_textureVertices;
	GLuint m_textureNormals;
	GLuint m_textureOctree;

	glm::mat4 m_invProj = glm::mat4(1.0f);
	glm::mat4 m_model = glm::mat4(1.0f);
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_invView = glm::mat4(1.0f);

	glm::mat4 m_invModel = glm::mat4(1.0f);

	bool m_useOctree;

	float m_thresh;

	int m_screenWidth;
	int m_screenHeight;
	GLuint query[2];


};

#endif