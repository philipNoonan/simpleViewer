#include "voxelizer.h"

void Voxelizer::init()
{
	compileAndLinkShader();
	setLocations();
	allocateTextures();
	allocateBuffers();
}

void Voxelizer::compileAndLinkShader() 
{ 
	try {
		voxelizerProg.compileShader("shaders/voxelize.cs");
		voxelizerProg.link();
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Voxelizer::setLocations() 
{

}

void Voxelizer::allocateTextures() 
{
	m_textureVolume = GLHelper::createTexture(m_textureVolume, GL_TEXTURE_3D, 1, 512, 512, 512, GL_R32F);

}

void Voxelizer::allocateBuffers() 
{
	uint32_t numVerts = 3;
	std::vector<float> triangle{ 30.0457993, 2.85114002, -35.9970016,
								 29.9820004, 2.80616999, -35.8725014,
								 29.9253006,	2.82903004, -35.9459991 };

	glGenBuffers(1, &m_infoSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_infoSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_infoSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VoxelizerInfo), NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &m_triangleData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_triangleData);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_triangleData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numVerts * sizeof(float) * 3, triangle.data(), GL_STATIC_DRAW);

}

void Voxelizer::voxelize(bool first)
{
	voxelizerProg.use();

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_infoSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VoxelizerInfo), &m_info, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_infoSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_triangleData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_triangleData);
	if (first)
	{
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_vertexData.size() * sizeof(float), m_vertexData.data(), GL_STATIC_DRAW);
	}



	int xWidth;
	int yWidth;


	xWidth = GLHelper::divup(m_vertexData.size(), 512);
	//yWidth = GLHelper::divup(m_vertexData.size() / 9, 512);


	glDispatchCompute(xWidth, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


}