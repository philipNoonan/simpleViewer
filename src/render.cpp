#include "render.h"

void Render::GLFWCallbackWrapper::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->MousePositionCallback(window, positionX, positionY);
}

void Render::GLFWCallbackWrapper::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	s_application->MouseButtonCallback(window, button, action, mods);
}

void Render::GLFWCallbackWrapper::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	s_application->ScrollCallback(window, xoffset, yoffset);
}

void Render::GLFWCallbackWrapper::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_application->KeyboardCallback(window, key, scancode, action, mods);
}

void Render::GLFWCallbackWrapper::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	s_application->WindowSizeCallback(window, width, height);
}

void Render::GLFWCallbackWrapper::SetApplication(Render* application)
{
	GLFWCallbackWrapper::s_application = application;
}

Render* Render::GLFWCallbackWrapper::s_application = nullptr;

void Render::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{

	int32_t dx = (int32_t)m_mousePos.x - positionX;
	int32_t dy = (int32_t)m_mousePos.y - positionY;

	if (mouseButtons.left)
	{
		m_rotation.x += dy * 1.25f * m_camera->rotationSpeed;
		m_rotation.y -= dx * 1.25f * m_camera->rotationSpeed;
		m_camera->rotate(glm::vec3(dy * m_camera->rotationSpeed, -dx * m_camera->rotationSpeed, 0.0f));
		//viewUpdated = true;
	}
	if (mouseButtons.right) {
		m_zoom += dy * .005f * zoomSpeed;
		m_camera->translate(glm::vec3(-0.0f, 0.0f, dy * .005f * zoomSpeed));
		//viewUpdated = true;
	}
	if (mouseButtons.middle) {
		m_cameraPos.x -= dx * 0.01f;
		m_cameraPos.y -= dy * 0.01f;
		m_camera->translate(glm::vec3(-dx * 0.01f, dy * 0.01f, 0.0f));
		//viewUpdated = true;
	}

	m_mousePos.x = positionX;
	m_mousePos.y = positionY;

}
void Render::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouseButtons.left = true;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		mouseButtons.right = true;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
	{
		mouseButtons.middle = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mouseButtons.left = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		mouseButtons.right = false;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
	{
		mouseButtons.middle = false;
	}

}

void Render::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//m_zoom += yoffset * zoomSpeed;
	m_camera->translate(glm::vec3(0.0f, 0.0f, yoffset * zoomSpeed));
}

void Render::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:

		if (m_camera->firstperson)
		{
			switch (key)
			{
			case GLFW_KEY_W:
				m_camera->keys.up = true;
				break;
			case GLFW_KEY_S:
				m_camera->keys.down = true;
				break;
			case GLFW_KEY_A:
				m_camera->keys.left = true;
				break;
			case GLFW_KEY_D:
				m_camera->keys.right = true;
				break;
			}
		}
		break;

	case GLFW_RELEASE:
		if (m_camera->firstperson)
		{
			switch (key)
			{
			case GLFW_KEY_W:
				m_camera->keys.up = false;
				break;
			case GLFW_KEY_S:
				m_camera->keys.down = false;
				break;
			case GLFW_KEY_A:
				m_camera->keys.left = false;
				break;
			case GLFW_KEY_D:
				m_camera->keys.right = false;
				break;
			}
		}
		break;
	}
	
	//std::cout << m_camera->keys.up << std::endl;

}

void Render::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	m_camera->setPerspective(45.0f, float(width) / float(height), 0.1, 1000.0f);
}

void Render::SetCallbackFunctions()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MousePositionCallback);
	glfwSetKeyCallback(m_window, GLFWCallbackWrapper::KeyboardCallback);
	glfwSetMouseButtonCallback(m_window, GLFWCallbackWrapper::MouseButtonCallback);
	glfwSetScrollCallback(m_window, GLFWCallbackWrapper::ScrollCallback);
	glfwSetWindowSizeCallback(m_window, GLFWCallbackWrapper::WindowSizeCallback);
}

GLFWwindow * Render::loadGLFWWindow()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glAlphaFunc(GL_GREATER, 0.1);
	//glEnable(GL_ALPHA_TEST);

	m_window = glfwCreateWindow(1024, 1024, "simpleViewer CTA Demo", nullptr, nullptr);

	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(m_window);
	//glfwSwapInterval(1); // Enable vsync
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		//return -1;
	}


	return m_window;
}


void Render::compileAndLinkShader()
{
	try {
		renderProg.compileShader("shaders/vertShader.vs");
		renderProg.compileShader("shaders/fragShader.fs");
		renderProg.link();


	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenQueries(2, query);

}

void Render::setLocations()
{
	m_ViewID = glGetUniformLocation(renderProg.getHandle(), "view");
	m_ProjectionID = glGetUniformLocation(renderProg.getHandle(), "projection");
	m_ModelID = glGetUniformLocation(renderProg.getHandle(), "model");

	m_invViewID = glGetUniformLocation(renderProg.getHandle(), "invView");
	m_invProjectionID = glGetUniformLocation(renderProg.getHandle(), "invProj");
	m_invModelID = glGetUniformLocation(renderProg.getHandle(), "invModel");

	m_MvpID = glGetUniformLocation(renderProg.getHandle(), "MVP");
	m_RotMatID = glGetUniformLocation(renderProg.getHandle(), "rotMat");

	m_imSizeID = glGetUniformLocation(renderProg.getHandle(), "imSize");
	m_sliceID = glGetUniformLocation(renderProg.getHandle(), "slice");
	m_sliceValsID = glGetUniformLocation(renderProg.getHandle(), "sliceVals");
	m_levelID = glGetUniformLocation(renderProg.getHandle(), "level");
	m_lightPosID = glGetUniformLocation(renderProg.getHandle(), "lightPos");



	m_positionSelectionRoutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_VERTEX_SHADER, "getPositionSelection");
	m_standardTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTexture");
	m_standardTexture3DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTexture3D");
	m_standardTextureMCID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTextureMC");
	m_octlistID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromOctlist");



	m_colorSelectionRoutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_FRAGMENT_SHADER, "getColorSelection");
	m_fromVolumeID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVolume");
	m_fromTexture2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromTexture2D");

	m_fromVertexArrayID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVertexArray");
}


void Render::setVertPositions()
{
	std::vector<float> vertices = {
		// Positions				// TEXTURES COORDS NOT USED HERE, going to pass into shader using uniforms
		1.0f,	1.0f,	0.0f,		1.0f, 1.0f, // top right
		1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, // bottom right
		-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, // bottom left
		-1.0f,	1.0f,	0.0f,		0.0f, 1.0f  // Top left
	};

	m_vertices = vertices;

	//std::vector<float> vertices3D = {
	//	// Positions				// 3D Texture coords
	//	1.0f,	1.0f,	0.0f,		1.0f, 1.0f, 0.0f,// top right
	//	1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, 0.0f, // bottom right
	//	-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, 0.0f, // bottom left
	//	-1.0f,	1.0f,	0.0f,		0.0f, 1.0f, 0.0f // Top left
	//};

	//m_vertices3D = vertices3D;

	std::vector<uint32_t>  indices = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};

	m_indices = indices;


	// set ortho plane positions
	std::vector<float> orthoVerts = {
		// pos						// texures
		1.0f,	1.0f,	0.0f,		1.0f, 1.0f, 0.0f, // top right
		1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, 0.0f, // bottom right
		-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, 0.0f, // bottom left
		-1.0f,	1.0f,	0.0f,		0.0f, 1.0f, 0.0f,  // Top left

		0.0f, 1.0f,	  1.0f,		    0.0f, 1.0f, 1.0f, // top right
		0.0f, 1.0f,	 -1.0f,		    0.0f, 1.0f, 0.0f, // bottom right
		0.0f, -1.0f, -1.0f,		    0.0f, 0.0f, 0.0f, // bottom left
		0.0f, -1.0f,  1.0f,		    0.0f, 0.0f, 1.0f,  // Top left

		1.0f,	0.0f, 1.0f,			1.0f, 0.0f, 1.0f, // top right
		1.0f,	0.0f, -1.0f,		1.0f, 0.0f, 0.0f, // bottom right
		-1.0f,	0.0f, -1.0f,		0.0f, 0.0f, 0.0f, // bottom left
		-1.0f,	0.0f, 1.0f,			0.0f, 0.0f, 1.0f, // Top left

	};

	std::vector<uint32_t>  indicesOrtho = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3, // Second Triangle

		4, 5, 7,
		5, 6, 7,

		8, 9, 11,
		9, 10, 11,
	};

	m_indicesOrtho = indicesOrtho;


	m_vertices3D = orthoVerts;

	std::vector<float> cubeOffsets = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 0.0f, 0.0f,-1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,-1.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,-1.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,-1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,-1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,-1.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	};

	m_cubePoints = cubeOffsets ;

}



void Render::setOrthoVerts(float x, float y, float z)
{
	// set ortho plane positions
	std::vector<float> orthoVerts = {
		// pos						// texures
		1.0f,	1.0f,	z * 2.0f - 1.0f,		1.0f, 1.0f, z, // top right
		1.0f,	-1.0f,	z * 2.0f - 1.0f,		1.0f, 0.0f, z, // bottom right
		-1.0f,	-1.0f,	z * 2.0f - 1.0f,		0.0f, 0.0f, z, // bottom left
		-1.0f,	1.0f,	z * 2.0f - 1.0f,		0.0f, 1.0f, z,  // Top left

		x * 2.0f - 1.0f, 1.0f,	  1.0f,		    x, 1.0f, 1.0f, // top right
		x * 2.0f - 1.0f, 1.0f,	 -1.0f,		    x, 1.0f, 0.0f, // bottom right
		x * 2.0f - 1.0f, -1.0f, -1.0f,		    x, 0.0f, 0.0f, // bottom left
		x * 2.0f - 1.0f, -1.0f,  1.0f,		    x, 0.0f, 1.0f,  // Top left

		1.0f,	y * 2.0f - 1.0f, 1.0f,			1.0f, y, 1.0f, // top right
		1.0f,	y * 2.0f - 1.0f, -1.0f,		    1.0f, y, 0.0f, // bottom right
		-1.0f,	y * 2.0f - 1.0f, -1.0f,		    0.0f, y, 0.0f, // bottom left
		-1.0f,	y * 2.0f - 1.0f, 1.0f,			0.0f, y, 1.0f, // Top left

	};


	m_vertices3D = orthoVerts;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_3D);
	glBufferData(GL_ARRAY_BUFFER, m_vertices3D.size() * sizeof(float), &m_vertices3D[0], GL_DYNAMIC_DRAW);

}

void Render::allocateBuffers()
{
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_VBO_3D);
	glGenBuffers(1, &m_VBO_Oct);

	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), &m_vertices[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), &m_indices[0], GL_DYNAMIC_DRAW);
	// Screen Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_3D);
	glBufferData(GL_ARRAY_BUFFER, m_vertices3D.size() * sizeof(float), &m_vertices3D[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indicesOrtho.size() * sizeof(uint32_t), &m_indicesOrtho[0], GL_DYNAMIC_DRAW);

	// Screen Position attribute 3d 
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	//// TexCoord attribute 3d
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(3);



	glBindVertexArray(0);




}


void Render::allocateBuffersFromMarchingCubes()
{
	glBindVertexArray(m_VAO);

	// standard verts for marching cubes
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferPos);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, (GLvoid*)0);
	glEnableVertexAttribArray(4);


	glBindBuffer(GL_ARRAY_BUFFER, m_bufferNorm);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(5);



	glBindVertexArray(0);

}

void Render::allocateBuffersForOctree()
{
	glBindVertexArray(m_VAO);


	// OCTRREE
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Oct);
	glBufferData(GL_ARRAY_BUFFER, m_cubePoints.size() * sizeof(float), &m_cubePoints[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(3 * sizeof(float)));




	glEnableVertexAttribArray(8);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);
	glVertexAttribIPointer(8, 1, GL_UNSIGNED_INT, 1 * sizeof(uint32_t), (GLvoid*)0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(8, 1); // IMPORTANT https://learnopengl.com/Advanced-OpenGL/Instancing




	glBindVertexArray(0);

}


void Render::allocateTextures()
{
	m_textureVolume = GLHelper::createTexture(m_textureVolume, GL_TEXTURE_3D, 1, 512, 512, 512, GL_R32F);
}


void Render::uploadImageData(vtkSmartPointer<vtkImageData> imData)
{
	int dims[3];
	imData->GetDimensions(dims);

	//m_inputImageVolume.resize(dims[0] * dims[1] * dims[2]);
	//memcpy_s(m_inputImageVolume.data(), dims[0] * dims[1] * dims[2] * sizeof(float), imageData->GetScalarPointer(), dims[0] * dims[1] * dims[2] * sizeof(float));


     GLenum theErr;

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	 glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	 if (imData->GetScalarType() == VTK_UNSIGNED_SHORT )
	 {
		 // IF THE IMAGE DATA IS IN SHORT< THEN WE CONVERT ON THE CPU BEFORE TRANSMITTING TO THE GPU
		 std::vector<uint16_t> tempShortData(dims[0] * dims[1] * dims[2], 0);
		 memcpy_s(tempShortData.data(), dims[0] * dims[1] * dims[2] * sizeof(uint16_t), imData->GetScalarPointer(), dims[0] * dims[1] * dims[2] * sizeof(uint16_t));
		 std::vector<float> tempFloatData(tempShortData.begin(), tempShortData.end());
		 glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, tempFloatData.data());

	 }
	 else if (imData->GetScalarType() == VTK_SHORT)
	 {
		 // IF THE IMAGE DATA IS IN SHORT< THEN WE CONVERT ON THE CPU BEFORE TRANSMITTING TO THE GPU
		 std::vector<int16_t> tempShortData(dims[0] * dims[1] * dims[2], 0);
		 memcpy_s(tempShortData.data(), dims[0] * dims[1] * dims[2] * sizeof(int16_t), imData->GetScalarPointer(), dims[0] * dims[1] * dims[2] * sizeof(uint16_t));
		 std::vector<float> tempFloatData(tempShortData.begin(), tempShortData.end());
		 glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, tempFloatData.data());

	 }
	 else if (imData->GetScalarType() == VTK_FLOAT)
	 {
		 glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, imData->GetScalarPointer());
	 }

	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);

	theErr = glGetError();
}


void Render::bindTexturesForRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureRaycast);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);


}
void Render::render()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDepthFunc(GL_LESS);

	glm::mat4 model = getModel();
	//glm::mat4 view = getView();
	//glm::mat4 projection = getProjection();

	glm::mat4 projection = m_camera->matrices.perspective;
	glm::mat4 view = m_camera->matrices.view;

	//glm::mat4 m_view512 = glm::lookAt(
	//	glm::vec3(0, 0, -m_zoom),           // Camera is here
	//	glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
	//	glm::vec3(0.0f, 1.0f, 0.0f)                  // Head is up (set to 0,-1,0 to look upside-down)
	//);

	//std::cout << m_camerPos.x << " " << m_camerPos.y << " " << m_camerPos.z << std::endl;
	
	//float zDist;
	//zDist = ((float)512 * 1) / tan(45.0f * M_PI / 180.0f);

	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	//	m_model = m_model_color;

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderProg.use();
	glm::mat4 MVP;

	glm::vec3 imageSize;

	imageSize = glm::vec3(512, 512, 304);
	glm::vec3 sVals(0, 0, m_slice);
	MVP = projection * view * model;

	//m_MV = m_view * m_model_color;
	
	//

	//glm::mat4 invMV = glm::inverse(m_MV);
	//glm::mat4 transMV = glm::transpose(m_MV);

	glBindVertexArray(m_VAO);

	camAngle += 0.5f;

	if (camAngle >= 360.0f)
	{
		camAngle = 0.0f;
	}

	
	m_lightPos.x = 10000.0f * cos(camAngle * 0.0174533f);
	m_lightPos.z = 10000.0f * sin(camAngle * 0.0174533f);

	glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(camAngle), glm::vec3(0, 0, 1));

	glUniform3fv(m_lightPosID, 1, glm::value_ptr(m_lightPos));

	if (m_renderOrtho)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTexture3DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(m_RotMatID, 1, GL_FALSE, glm::value_ptr(rotMat));

		glUniform1i(m_levelID, m_level);

		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	}
	


	if (m_renderMarchingCubes)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferPos);
		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferNorm);

		//glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTextureMCID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVertexArrayID);
		glDrawArrays(GL_TRIANGLES, 0, m_numTrianglesMC);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}



	if (m_renderOctree)
	{
		glBeginQuery(GL_TIME_ELAPSED, query[0]);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Oct);

		glEnableVertexAttribArray(7);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// set projectiopn and view mat
		glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(m_ViewID, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_ModelID, 1, GL_FALSE, glm::value_ptr(model));

		glUniformMatrix4fv(m_invProjectionID, 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
		glUniformMatrix4fv(m_invViewID, 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));
		glUniformMatrix4fv(m_invModelID, 1, GL_FALSE, glm::value_ptr(glm::inverse(model)));

		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(m_RotMatID, 1, GL_FALSE, glm::value_ptr(rotMat));


		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_octlistID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVertexArrayID);

		if (m_renderVoxels)
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_octlistCount);
		}
		else
		{
			glDrawArraysInstanced(GL_POINTS, 0, 1, m_octlistCount);
			//glDrawArrays(GL_POINTS, 0, 2);

		}
		

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

		glEndQuery(GL_TIME_ELAPSED);
		GLuint available = 0;
		while (!available) {
			glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
		}
		// elapsed time in nanoseconds
		GLuint64 elapsed;
		glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
		auto hpTime = elapsed / 1000000.0;

		std::cout << "render time : " << hpTime << std::endl;

	}
	
	if (m_renderRaytrace)
	{
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(m_ViewID, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m_ModelID, 1, GL_FALSE, glm::value_ptr(model));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromTexture2DID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//	//glm::vec4 origin = (m_view * m_model)[3];
	//	glm::vec3 boxmin = glm::vec3(-1.0, -1.0, -1.0);

	//	glm::vec3 boxmax = glm::vec3(1.0, 1.0, 1.0);

	//	/*	for (int i = 255; i < 256; i++)
	//		{
	//			for (int j = 255; j < 256; j++)
	//			{
	//				float u = (2.0 * float(i)) / 512.0 - 1.0f;
	//				float v = (2.0 * float(j)) / 512.0 - 1.0f;
	//*/
	//	float u = 0;
	//	float v = 0;

	//	glm::vec4 origin = (m_model * m_view)[3];

	//	std::cout << "origin : " << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;

	//	//glm::vec3 ray_nds = glm::vec3(u, v, 1.0f);

	//	glm::vec4 ray_clip = glm::vec4(u, v, -1.0, 1.0);
	//	glm::vec4 ray_eye = getInverseProjection() * ray_clip;

	//	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	//	glm::vec4 direction = glm::normalize(getInverseView() * getInverseModel() * ray_eye);
	//	std::cout << "direction : " << direction[0] << " " << direction[1] << " " << direction[2] << std::endl;






	//	glm::vec3 invR = glm::vec3(1.0f) / glm::vec3(direction);
	//	glm::vec3 tbot = invR * (boxmin - glm::vec3(origin));
	//	glm::vec3 ttop = invR * (boxmax - glm::vec3(origin));

	//	// re-order intersections to find smallest and largest on each axis
	//	glm::vec3 tmin = min(ttop, tbot);
	//	glm::vec3 tmax = max(ttop, tbot);

	//	// find the largest tmin and the smallest tmax
	//	float largest_tmin = std::max(std::max(tmin[0], tmin[1]), std::max(tmin[0], tmin[2]));
	//	float smallest_tmax = std::min(std::min(tmax[0], tmax[1]), std::min(tmax[0], tmax[2]));

	//	float tnear = largest_tmin;
	//	float tfar = smallest_tmax;

	//	if (smallest_tmax > largest_tmin)
	//	{
	//		std::cout << "hit " << std::endl;
	//	}

	//	/*		}
	//		}*/





	}



	glBindVertexArray(0);



}