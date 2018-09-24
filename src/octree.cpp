#include "octree.h"

void Octree::init()
{
	compileAndLinkShader();
	setLocations();
	allocateTextures();
	allocateBuffers();
}

void Octree::compileAndLinkShader()
{
	try {
		octreeProg.compileShader("shaders/octree.cs");
		octreeProg.link();

		octlistProg.compileShader("shaders/octlist.cs");
		octlistProg.link();
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenQueries(2, query);



}
void Octree::setLocations()
{
	// quadtrees
	m_subroutine_hpOctreeID = glGetSubroutineUniformLocation(octreeProg.getHandle(), GL_COMPUTE_SHADER, "hpOctreeSubroutine");
	m_hpDiscriminatorID = glGetSubroutineIndex(octreeProg.getHandle(), GL_COMPUTE_SHADER, "hpDiscriminator");
	m_hpBuilderID = glGetSubroutineIndex(octreeProg.getHandle(), GL_COMPUTE_SHADER, "hpBuilder");

	m_hpLevelID = glGetUniformLocation(octreeProg.getHandle(), "hpLevel");
	m_isoLevelID = glGetUniformLocation(octreeProg.getHandle(), "isoLevel");
	m_cutoffTreeID = glGetUniformLocation(octreeProg.getHandle(), "cutoff");

	m_subroutine_hpOctlistID = glGetSubroutineUniformLocation(octlistProg.getHandle(), GL_COMPUTE_SHADER, "octlistSubroutine");
	m_traverseHPLevelID = glGetSubroutineIndex(octlistProg.getHandle(), GL_COMPUTE_SHADER, "traverseHPLevel");
	m_totalSumID = glGetUniformLocation(octlistProg.getHandle(), "totalSum");
	m_cutoffID = glGetUniformLocation(octlistProg.getHandle(), "cutoff");

	

}
void Octree::allocateTextures()
{
	// OCTREEE
	//m_texture_hpOriginalData = GLHelper::createTexture(m_texture_hpOriginalData, GL_TEXTURE_2D, 1, 512, 512, 0, GL_R32F);
	m_texture_hpOctree = GLHelper::createTexture(m_texture_hpOctree, GL_TEXTURE_3D, 10, 512, 512, 512, GL_R32F);
}
void Octree::allocateBuffers()
{
	// quadtrees
	//glGenBuffers(1, &m_bufferPos);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 10e7, NULL, GL_STREAM_COPY); // some max size

	

	glGenBuffers(1, &m_bufferPosEncode);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferPosEncode);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 512*512*512 * sizeof(uint32_t), NULL, GL_STREAM_COPY); // some max size, look into this
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, 512 * 512 * 512 * sizeof(uint32_t), NULL, 0);


}

void Octree::buildTree()
{
	glBeginQuery(GL_TIME_ELAPSED, query[0]);

	octreeProg.use();

	glBindImageTexture(0, m_texture_hpOctree, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texture_hpOctree);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_texture_hpOriginalData);

	glm::uvec3 nthreads = GLHelper::divup(glm::uvec3(512, 512, 512), glm::uvec3(8, 8, 8));
	glUniform1f(m_isoLevelID, m_isoLevel);
	glUniform1ui(m_cutoffTreeID, m_cutoff);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_hpDiscriminatorID);
	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	


	glActiveTexture(GL_TEXTURE0);

	for (int i = 0; i < 9; i++)
	{
		glm::uvec3 nthreads = GLHelper::divup(glm::uvec3((512 >> i) / 2, (512 >> i) / 2, (512 >> i) / 2), glm::uvec3(8, 8, 8));

		glUniform1i(m_hpLevelID, i);
		glUniform1ui(m_cutoffTreeID, m_cutoff);

		glBindImageTexture(0, m_texture_hpOctree, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_hpBuilderID);
		glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
	auto hpTime = elapsed / 1000000.0;

	std::cout << "octree build time : " << hpTime << std::endl;

	std::vector<float> sumData(1, 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texture_hpOctree);
	glGetTexImage(GL_TEXTURE_3D, 9, GL_RED, GL_FLOAT, sumData.data());
	glBindTexture(GL_TEXTURE_3D, 0);

	std::cout << "sum " << sumData[0] << std::endl;

	m_totalSum = sumData[0];


}

void Octree::createList()
{
	/*int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group size x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);*/

	glBeginQuery(GL_TIME_ELAPSED, query[1]);

	octlistProg.use();
	glm::uvec3 nthreads = GLHelper::divup(glm::uvec3(m_totalSum, 1, 1), glm::uvec3(32, 1, 1));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texture_hpOctree);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferPosEncode);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_traverseHPLevelID);
	glUniform1ui(m_totalSumID, m_totalSum);
	glUniform1ui(m_cutoffID, m_cutoff);

	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[1], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[1], GL_QUERY_RESULT, &elapsed);
	auto hpTime = elapsed / 1000000.0;

	std::cout << "octree list time : " << hpTime << std::endl;

	//uint32_t xPos = 1023 << 20;
	//uint32_t yPos = 1023 << 10;
	//uint32_t zPos = 1023 << 5;

	//std::vector<float> posData(m_totalSum * 4);
	//std::vector<uint32_t> posDataOri(m_totalSum );
	//std::vector<uint32_t> posDataOri2(m_totalSum * 4);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPos);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(posData.data(), posData.size() * sizeof(float), ptr, posData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


	//int j = 0;
	//for (int i = 0; i < posDataOri.size(); i++, j += 4)
	//{
	//	posDataOri[i] = (uint32_t)posData[j] << 23 | (uint32_t)posData[j + 1] << 14 | (uint32_t)posData[j + 2] << 5 | (uint32_t)posData[j + 3];
	//	posDataOri2[j] = (posDataOri[i] & (511 << 23)) >> 23;
	//	posDataOri2[j+1] = (posDataOri[i] & (511 << 14)) >> 14;
	//	posDataOri2[j+2] = (posDataOri[i] & (511 << 5)) >> 5;
	//	posDataOri2[j+3] = posDataOri[i] & (31);

	//}

	//for (int k = 0; k < posDataOri.size(); k++)
	//{

	//}

	//std::vector<uint32_t> posDataEncode(m_totalSum);
	//std::vector<uint32_t> posDataOut(m_totalSum * 4);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPosEncode);
	//void *ptrEnc = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(posDataEncode.data(), posDataEncode.size() * sizeof(uint32_t), ptrEnc, posDataEncode.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);






	//j = 0;
	//for (int i = 0; i < posDataEncode.size(); i++, j += 4)
	//{
	//	  

	//	posDataOut[j] = posDataEncode[i] & 4286578688;
	//	posDataOut[j + 1] = posDataEncode[i] & 8372224;
	//	posDataOut[j + 2] = posDataEncode[i] & 16352;
	//	posDataOut[j + 3] = posDataEncode[i] & 31;

	//}





}