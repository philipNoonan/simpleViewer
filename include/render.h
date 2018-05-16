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

#include <iostream>
#include <fstream>
#include <vector>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>

#define _USE_MATH_DEFINES
#include <math.h>

class Render
{
public:
	Render() {};
	~Render() {};

	GLFWwindow * window()
	{
		return m_window;
	}


	GLFWwindow * loadGLFWWindow();

	void compileAndLinkShader();

	void uploadImageData(vtkSmartPointer<vtkImageData> imagePtr);

	void setLocations();
	void allocateTextures();
	void setVertPositions();
	void allocateBuffers();
	void allocateBuffersFromMarchingCubes();
	void bindTexturesForRendering();
	void render();
	void setLevel(int L)
	{
		m_level = L;
	}
	void setRotation(glm::vec3 R)
	{
		m_rotation = R;
	}
	void setCameraPos(glm::vec3 P)
	{
		m_camerPos = P;
	}
	void setZoom(float Z)
	{
		m_zoom += Z;
	}
	void setOrthoVerts(float x, float y, float z);
	GLuint getVolumeTexture()
	{
		return m_textureVolume;
	}
	void setPosBuffer(GLuint PB)
	{
		m_bufferPos = PB;
	}
	void setNumTrianglesMC(int numTri)
	{
		m_numTrianglesMC = numTri;
	}
	void setRaycastTexture(GLuint rcT)
	{
		m_textureRaycast = rcT;
	}
	//void setTextures();
	glm::mat4 getMV()
	{
		return m_MV;
	}
	void setRenderMarchingCubesFlag(bool flg)
	{
		m_renderMarchingCubes = flg;
	}
	void setRenderRaytraceFlag(bool flg)
	{
		m_renderRaytrace = flg;
	}

private:
	GLSLProgram renderProg;

	GLFWwindow * m_window;

	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_VBO_3D;
	GLuint m_VBO_MC;
	GLuint m_EBO;

	//GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);

	GLuint m_textureVolume;
	GLuint m_textureRaycast;

	GLuint m_bufferPos;

	std::vector<float> m_vertices;
	std::vector<float> m_vertices3D;
	std::vector<uint32_t> m_indices;
	std::vector<uint32_t> m_indicesOrtho;

	GLuint m_ProjectionID;
	GLuint m_MvpID;
	GLuint m_imSizeID;
	GLuint m_sliceID;
	GLuint m_sliceValsID;
	GLuint m_levelID;

	GLuint m_positionSelectionRoutineID;
	GLuint m_standardTextureID;
	GLuint m_standardTexture3DID;
	GLuint m_standardTextureMCID;

	GLuint m_colorSelectionRoutineID;
	GLuint m_fromVolumeID;
	GLuint m_fromVertexArrayID;
	GLuint m_fromTexture2DID;


	glm::vec3 m_rotation = glm::vec3();
	glm::vec3 m_camerPos = glm::vec3();
	float m_zoom = 0;
	glm::mat4 m_MV;

	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 3000.0f); // some default matrix
	glm::mat4 m_model_color = glm::mat4(1.0);
	float m_slice = 0.0f;
	int m_level = 0;
	int m_numTrianglesMC = 0;

	bool m_renderMarchingCubes = false;
	bool m_renderRaytrace = false;

};