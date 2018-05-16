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
	void setInverseCameraMatrix(glm::mat4 invCamMat)
	{
		m_invK = glm::inverse(invCamMat);
	}
	void setPose(glm::mat4 pose)
	{
		m_pose = pose;
	}
	void setScreenWidth(int w)
	{
		m_screenWidth = w;
	}	
	void setScreenHeight(int h)
	{
		m_screenHeight = h;
	}

private:

	GLSLProgram raycastProg;

	void compileAndLinkShader();
	void setLocations();
	void allocateTextures();
	void allocateBuffers();


	// locations
	GLuint m_viewID_r;
	GLuint m_nearPlaneID;
	GLuint m_farPlaneID;
	GLuint m_stepID;
	GLuint m_largeStepID;
	GLuint m_volDimID_r;
	GLuint m_volSizeID_r;
	GLuint m_helpersSubroutineID;
	// textures
	GLuint m_textureVolume;
	GLuint m_textureVertices;
	GLuint m_textureNormals;

	glm::mat4 m_invK = glm::mat4(1.0f);
	glm::mat4 m_pose = glm::mat4(1.0f);

	int m_screenWidth;
	int m_screenHeight;


};

#endif