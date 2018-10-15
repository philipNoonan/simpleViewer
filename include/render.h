#ifndef RENDER_H
#define RENDER_H


#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


#include "glutils.h"
#include "glslprogram.h"
#include "glhelper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>

#include "camera.hpp"

//
//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"

#define _USE_MATH_DEFINES
#include <math.h>




class Render
{
public:
	Render() {};
	~Render() {};

	void SetCallbackFunctions();
	void setCamera(Camera *cam)
	{
		m_camera = cam;
	}

	void cleanup()
	{
		glDeleteTextures(1, &m_textureVolume);
	}

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
	void allocateBuffersForOctree();
	void bindTexturesForRendering();
	void render();
	void setLevel(int L)
	{
		m_level = L;
	}
	void setRotation(glm::vec3 R)
	{
		m_model = glm::mat4(1.0f);

		//m_model = glm::rotate(m_model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//m_model = glm::rotate(m_model, glm::radians(180.0f + m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//m_model = glm::rotate(m_model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//m_rotation = R;
	}
	void setCameraPos(glm::vec3 P)
	{
		m_cameraPos = P;
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
	glm::mat4 getModelView()
	{
		return m_MV;
	}
	glm::mat4 inverseMat4(glm::mat4 M)
	{
		glm::mat4 tempInvMat = glm::transpose(M);
		tempInvMat[3] = M[3] * -1.0f;

		tempInvMat[0][3] = 0.0f;
		tempInvMat[1][3] = 0.0f;
		tempInvMat[2][3] = 0.0f;

		tempInvMat[3][3] = 1.0f;

		return tempInvMat;
	}
	glm::mat4 getView()
	{
		//m_view = glm::lookAt(
		//	glm::vec3(0, 0, m_zoom),           // Camera is here
		//	glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
		//	glm::vec3(0.0f, 1.0f, 0.0f)                  // Head is up (set to 0,-1,0 to look upside-down)
		//);

		return m_camera->matrices.view;
	}
	glm::mat4 getModel()
	{

		return m_model;
	}
	glm::mat4 getInverseView()
	{
		return inverseMat4(m_camera->matrices.view);
	}
	glm::mat4 getInverseModel()
	{
		return inverseMat4(m_model);
	}
	glm::mat4 getProjection()
	{
		//int w, h;
		//glfwGetFramebufferSize(m_window, &w, &h);
		//m_projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 1000.0f); // scaling the texture to the current window size seems to work
		//glViewport(0, 0, w, h);

		return m_camera->matrices.perspective;
	}
	glm::mat4 getInverseProjection()
	{
		// taken from http://bookofhook.com/mousepick.pdf
		glm::mat4 tempInvMat(0.0f);
		tempInvMat[0][0] = 1.0f / m_camera->matrices.perspective[0][0];
		tempInvMat[1][1] = 1.0f / m_camera->matrices.perspective[1][1];
		tempInvMat[2][3] = 1.0f / m_camera->matrices.perspective[3][2];
		tempInvMat[3][2] = 1.0f / m_camera->matrices.perspective[2][3];
		tempInvMat[3][3] = -m_camera->matrices.perspective[2][2] / (m_camera->matrices.perspective[3][2] * m_camera->matrices.perspective[2][3]);

		//glm::mat4 temp = glm::inverse(m_projection);

		return tempInvMat;
	}
	void setRenderOthroFlag(bool flag)
	{
		m_renderOrtho = flag;
	}
	void setRenderMarchingCubesFlag(bool flg)
	{
		m_renderMarchingCubes = flg;
	}
	void setRenderRaytraceFlag(bool flg)
	{
		m_renderRaytrace = flg;
	}
	void setRenderOctlistFlag(bool flag)
	{
		m_renderOctree = flag;
	}
	void setOctlistBuffer(GLuint buffer)
	{
		m_bufferOctlist = buffer;
	}
	void setOctlistCount(int cnt)
	{
		m_octlistCount = cnt;
	}

private:

	struct {
		bool left = false;
		bool right = false;
		bool middle = false;
	} mouseButtons;

	// this static wrapped clas was taken from BIC comment on https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
	void MousePositionCallback(GLFWwindow* window, double positionX, double positionY);
	void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void WindowSizeCallback(GLFWwindow* window, int width, int height);

	class GLFWCallbackWrapper
	{
	public:
		GLFWCallbackWrapper() = delete;
		GLFWCallbackWrapper(const GLFWCallbackWrapper&) = delete;
		GLFWCallbackWrapper(GLFWCallbackWrapper&&) = delete;
		~GLFWCallbackWrapper() = delete;

		static void MousePositionCallback(GLFWwindow* window, double positionX, double positionY);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void WindowSizeCallback(GLFWwindow* window, int width, int height);

		static void SetApplication(Render *application);
	private:
		static Render* s_application;
	};

	Camera *m_camera;

	float rotationSpeed = 0.1f;
	float zoomSpeed = 1.0f;
	glm::vec3 m_rotation = glm::vec3();
	glm::vec3 m_cameraPos = glm::vec3();
	glm::vec2 m_mousePos;

	GLSLProgram renderProg;
	GLuint query[2];

	GLFWwindow * m_window;

	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_VBO_3D;
	GLuint m_VBO_MC;
	GLuint m_VBO_Oct;
	GLuint m_EBO;

	//GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);

	GLuint m_textureVolume;
	GLuint m_textureRaycast;

	GLuint m_bufferPos;
	GLuint m_bufferOctlist;

	std::vector<float> m_vertices;
	std::vector<float> m_vertices3D;
	std::vector<uint32_t> m_indices;
	std::vector<uint32_t> m_indicesOrtho;
	std::vector<float> m_cubePoints;

	GLuint m_ViewID;
	GLuint m_ProjectionID;
	GLuint m_ModelID;
	GLuint m_MvpID;
	GLuint m_imSizeID;
	GLuint m_sliceID;
	GLuint m_sliceValsID;
	GLuint m_levelID;

	GLuint m_positionSelectionRoutineID;
	GLuint m_standardTextureID;
	GLuint m_standardTexture3DID;
	GLuint m_standardTextureMCID;
	GLuint m_octlistID;

	GLuint m_colorSelectionRoutineID;
	GLuint m_fromVolumeID;
	GLuint m_fromVertexArrayID;
	GLuint m_fromTexture2DID;


	//glm::vec3 m_rotation = glm::vec3();

	//glm::vec3 m_camerPos = glm::vec3();
	float m_zoom = 5;
	glm::mat4 m_MV;

	glm::mat4 m_cameraMatrix = glm::mat4(1.0f);
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 3000.0f); // some default matrix
	glm::mat4 m_model_color = glm::mat4(1.0);
	glm::mat4 m_model = glm::mat4(1.0f);

	float m_slice = 0.0f;
	int m_level = 0;
	int m_numTrianglesMC = 0;
	int m_octlistCount = 0;

	bool m_renderOrtho = false;
	bool m_renderMarchingCubes = false;
	bool m_renderRaytrace = false; bool m_renderFastRaytrace = true;
	bool m_renderOctree = false;

};


#endif