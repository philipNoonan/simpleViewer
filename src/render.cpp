#include "render.h"

GLFWwindow * Render::loadGLFWWindow()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

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
}

void Render::setLocations()
{
	m_ViewID = glGetUniformLocation(renderProg.getHandle(), "view");
	m_ProjectionID = glGetUniformLocation(renderProg.getHandle(), "projection");
	m_ModelID = glGetUniformLocation(renderProg.getHandle(), "model");

	m_MvpID = glGetUniformLocation(renderProg.getHandle(), "MVP");
	m_imSizeID = glGetUniformLocation(renderProg.getHandle(), "imSize");
	m_sliceID = glGetUniformLocation(renderProg.getHandle(), "slice");
	m_sliceValsID = glGetUniformLocation(renderProg.getHandle(), "sliceVals");
	m_levelID = glGetUniformLocation(renderProg.getHandle(), "level");
	
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
		0.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 

		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
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
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(4);

	// OCTRREE
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Oct);
	glBufferData(GL_ARRAY_BUFFER, m_cubePoints.size() * sizeof(float), &m_cubePoints[0], GL_STATIC_DRAW);

	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(6);

	glEnableVertexAttribArray(7);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(7, 1); // IMPORTANT https://learnopengl.com/Advanced-OpenGL/Instancing




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
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, imData->GetScalarPointer());
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


	m_view = glm::lookAt(
		glm::vec3(0, 0, m_zoom),           // Camera is here
		glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
		glm::vec3(0.0f, 1.0f, 0.0f)                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	//std::cout << m_camerPos.x << " " << m_camerPos.y << " " << m_camerPos.z << std::endl;
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	m_projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 1000.0f); // scaling the texture to the current window size seems to work

	float zDist;
	zDist = ((float)512 * 1) / tan(45.0f * M_PI / 180.0f);

	m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	m_model_color = glm::rotate(m_model_color, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model_color = glm::rotate(m_model_color, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_model_color = glm::rotate(m_model_color, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_model = m_model_color;
	glViewport(0, 0, w, h);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderProg.use();
	glm::mat4 MVP;

	glm::vec3 imageSize;

	imageSize = glm::vec3(512, 512, 304);
	glm::vec3 sVals(0, 0, m_slice);
	MVP = m_projection * m_view *m_model_color;
	m_MV = m_view * m_model_color;
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	glBindVertexArray(m_VAO);
	if (m_renderOrtho)
	{

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTexture3DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform1i(m_levelID, m_level);

		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	if (m_renderMarchingCubes)
	{
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferPos);

		glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTextureMCID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVertexArrayID);
		glDrawArrays(GL_TRIANGLES, 0, m_numTrianglesMC);
	}

	if (m_renderRaytrace)
	{
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_standardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromTexture2DID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	if (m_renderOctree)
	{

		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Oct);

		glEnableVertexAttribArray(7);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferOctlist);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// set projectiopn and view mat
		glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_ViewID, 1, GL_FALSE, glm::value_ptr(m_view));
		glUniformMatrix4fv(m_ModelID, 1, GL_FALSE, glm::value_ptr(m_model));


		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_octlistID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVertexArrayID);

		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, m_octlistCount);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}
	


	glBindVertexArray(0);



}