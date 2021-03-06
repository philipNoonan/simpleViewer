#include "mcubes.h"
#include "tables.h"


void MCubes::init()
{
	compileAndLinkShader();
	setLocations();
	allocateTextures();
	allocateBuffers();
	loadTables();

}


void MCubes::compileAndLinkShader()
{
	try {
		//marchingCubesProg.compileShader("shaders/marchingCubes.cs");
		//marchingCubesProg.link();

		//prefixSumProg.compileShader("shaders/prefixSum.cs");
		//prefixSumProg.link();

		histoPyramidsProg.compileShader("shaders/histoPyramids.cs");
		histoPyramidsProg.link();

		traverseHistoPyramidsProg.compileShader("shaders/traverseHistoPyramids.cs");
		traverseHistoPyramidsProg.link();

	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	glGenQueries(2, query);

}
void MCubes::setLocations()
{

	// HISTO PYRA
	m_histoPyramidsSubroutineID = glGetSubroutineUniformLocation(histoPyramidsProg.getHandle(), GL_COMPUTE_SHADER, "histoPyramidsSubroutine");
	m_classifyCubesID = glGetSubroutineIndex(histoPyramidsProg.getHandle(), GL_COMPUTE_SHADER, "classifyCubes");
	m_constructHPLevelID = glGetSubroutineIndex(histoPyramidsProg.getHandle(), GL_COMPUTE_SHADER, "constructHPLevel");
	m_baseLevelID = glGetUniformLocation(histoPyramidsProg.getHandle(), "hpLevel");
	m_isoLevelID = glGetUniformLocation(histoPyramidsProg.getHandle(), "isoLevel");

	m_traverseHistoPyramidsSubroutineID = glGetSubroutineUniformLocation(traverseHistoPyramidsProg.getHandle(), GL_COMPUTE_SHADER, "traverseHistoPyramidsSubroutine");
	m_traverseHPLevelID = glGetSubroutineIndex(traverseHistoPyramidsProg.getHandle(), GL_COMPUTE_SHADER, "traverseHPLevel");
	m_totalSumID = glGetUniformLocation(traverseHistoPyramidsProg.getHandle(), "totalSum");
	m_isoLevel_TravID = glGetUniformLocation(traverseHistoPyramidsProg.getHandle(), "isoLevel");

	m_scaleVecID = glGetUniformLocation(traverseHistoPyramidsProg.getHandle(), "scaleVec");

	//// PREFIX SUMS
	//m_prefixSumSubroutineID = glGetSubroutineUniformLocation(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "getPrefixSum");
	//m_resetSumsArrayID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "resetSumsArray");

	//m_forEachGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEachGroup");
	//m_forEveryGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEveryGroup");
	//m_forFinalIncrementalSumID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forFinalIncrementalSum");

	//// MARCHING CUBES
	//m_marchingCubesSubroutineID = glGetSubroutineUniformLocation(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "doMarchingCubes");
	//m_classifyVoxelID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchClassifyVoxel");
	//m_compactVoxelsID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchCompactVoxels");
	//m_generateTrianglesID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchGenerateTriangles");

	//m_gridSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSize");
	//m_gridSizeShiftID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeShift");
	//m_gridSizeMaskID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeMask");
	//m_numVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "numVoxels");
	//m_activeVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "activeVoxels");
	//m_maxVertsID = glGetUniformLocation(marchingCubesProg.getHandle(), "maxVerts");
	//m_voxelSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "voxelSize");
}



void MCubes::allocateTextures()
{
	// MARCHING CUBES TEXTURES
	// SHOULD WE PAD TO POWERS OF TWO TEXTURE SIZES?
	m_textureHistoPyramid = GLHelper::createTexture(m_textureHistoPyramid, GL_TEXTURE_3D, 10, 512, 512, 512, GL_R32F); // texture is define as R32UI however level 0 will be read in shader as rg16ui https://www.khronos.org/opengl/wiki/Image_Load_Store#Format_conversion
	m_textureEdgeTable = GLHelper::createTexture(m_textureEdgeTable, GL_TEXTURE_1D, 1, 256, 1, 1, GL_R16UI);
	m_textureTriTable = GLHelper::createTexture(m_textureTriTable, GL_TEXTURE_1D, 1, 256 * 16, 1, 1, GL_R16UI);
	m_textureNumVertsTable = GLHelper::createTexture(m_textureNumVertsTable, GL_TEXTURE_1D, 1, 256, 1, 1, GL_R16UI);

	m_textureNrOfTriangles = GLHelper::createTexture(m_textureNrOfTriangles, GL_TEXTURE_1D, 1, 256, 1, 1, GL_R8UI);
	m_textureOffsets3 = GLHelper::createTexture(m_textureOffsets3, GL_TEXTURE_1D, 1, 72, 1, 1, GL_R8UI);

	//m_testVolumeTexture = GLHelper::createTexture(m_testVolumeTexture, GL_TEXTURE_3D, 10, 512, 512, 512, GL_R32F);

	//texture views, this creates a texture handle that is linked to the whole pyramid storage of the OG histopyramid, but thinks it is RG16F format
	// https://www.khronos.org/opengl/wiki/Texture_Storage#Texture_views
	glGenTextures(1, &m_textureviewHistoPyramid);
	glTextureView(m_textureviewHistoPyramid, GL_TEXTURE_3D, m_textureHistoPyramid, GL_RG16F, 0, 10, 0, 1);

}
void MCubes::allocateBuffers()
{
	// MARCHING CUBES BUFFERS
	size_t memSize = sizeof(GLuint) * m_mcubeConfiguration.numVoxels;
	size_t memSizeVec4 = 512 * 512 * 512 * sizeof(uint32_t); // SET THIS PROPERLY ONCE AND DONT CHNAGE IT

	/*glGenBuffers(1, &m_bufferVoxelVerts);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferVoxelOccupied);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferVoxelOccupiedScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferCompactedVoxelArray);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferVoxelVertsScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);*/

	//glGenBuffers(1, &m_bufferPos);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_STREAM_COPY);

	glGenBuffers(1, &m_bufferPosEncode);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferPosEncode);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_STREAM_COPY);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, 512 * 512 * 512 * sizeof(uint32_t), NULL, 0);

	glGenBuffers(1, &m_bufferNorm);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferNorm);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 512 * 512 * 512 * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	//glGenBuffers(1, &m_bufferPrefixSumByGroup);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bufferPrefixSumByGroup);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, memSize / 1024, NULL, GL_DYNAMIC_DRAW);
}



GLuint MCubes::prefixSum(GLuint inputBuffer, GLuint outputBuffer)
{
	std::vector<uint32_t> outData0, prefixsumgrp0, prefixsumgrp1;
	outData0.resize(m_mcubeConfiguration.numVoxels, 2);
	prefixsumgrp0.resize(m_mcubeConfiguration.numVoxels / 1024, 2);
	prefixsumgrp1.resize(m_mcubeConfiguration.numVoxels / 1024, 2);

	// reduction sum
	prefixSumProg.use();
	int xthreads = GLHelper::divup(m_mcubeConfiguration.numVoxels, 1024); // 1024 is the localworkgroupsize inside the shader

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, inputBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, outputBuffer);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetSumsArrayID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEachGroupID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPrefixSumByGroup);
	void *ptrp0 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(prefixsumgrp0.data(), prefixsumgrp0.size() * sizeof(uint32_t), ptrp0, prefixsumgrp0.size() * sizeof(uint32_t));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	int xthreads2 = GLHelper::divup(xthreads, 1024);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEveryGroupID);
	glDispatchCompute(xthreads2, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPrefixSumByGroup);
	//void *ptrp1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(prefixsumgrp1.data(), prefixsumgrp1.size() * sizeof(uint32_t), ptrp1, prefixsumgrp1.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);









	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forFinalIncrementalSumID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	uint32_t lastElement, lastScanElement;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (m_mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastScanElement);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (m_mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastElement);
	
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(outData0.data(), outData0.size() * sizeof(uint32_t), ptr, outData0.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//for (size_t i = 0; i < outData0.size(); i += 1000)
	//{
	//	std::cout << outData0[i] << " ";
	//}
	return lastElement + lastScanElement;

}

void MCubes::histoPyramids()
{
	glBeginQuery(GL_TIME_ELAPSED, query[0]);

	/*std::vector<float> wipeData(0, 4);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_RGBA32F, GL_RGBA, GL_FLOAT, wipeData.data());
*/

	histoPyramidsProg.use();
	glm::uvec3 lSize(8,8,8);
	/// Classify cubes



	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, m_textureEdgeTable);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D, m_textureTriTable);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, m_textureNrOfTriangles);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_1D, m_textureOffsets3);

	// https://stackoverflow.com/questions/30110521/format-conversions-in-opengl-images
	glBindImageTexture(2, m_textureHistoPyramid, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F); 
	//glBindImageTexture(3, m_testVolumeTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

	glm::uvec3 nthreads = GLHelper::divup(glm::uvec3(512, 512, 512), lSize);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_classifyCubesID);
	glUniform1f(m_isoLevelID, m_isoLevel);

	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);




	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, m_testVolumeTexture);
	//glGenerateMipmap(GL_TEXTURE_3D);

	//std::vector<float> sumVol0(1, 3);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, m_testVolumeTexture);
	//glGetTexImage(GL_TEXTURE_3D, 9, GL_RED, GL_FLOAT, sumVol0.data());
	//glBindTexture(GL_TEXTURE_3D, 0);
	//glActiveTexture(0);

	//std::cout << "mipmap end val : " << sumVol0[0] << std::endl;
	//std::cout << "mipmap sum : " << sumVol0[0] * 512.0f * 512.0f * 512.0f << std::endl;
	//std::cout.precision(std::numeric_limits<float>::max_digits10);
	//std::cout << "mipmap sum in float precision: " << sumVol0[0] * 512.0f * 512.0f * 512.0f << std::endl;

	/// Do first level of histoprys



	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureviewHistoPyramid); // texture view on first level

	glBindImageTexture(2, m_textureHistoPyramid, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16F);
	glBindImageTexture(1, m_textureHistoPyramid, 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	nthreads = GLHelper::divup(glm::uvec3(512 / 2, 512 / 2, 512 / 2), lSize);
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_constructHPLevelID);
	glUniform1i(m_baseLevelID, 0);

	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);




	// swap to r32f texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureHistoPyramid);

	/// Do other levels of histopyr
	for (int i = 1; i < 9; i++)
	{
		// https://stackoverflow.com/questions/17015132/compute-shader-not-modifying-3d-texture
		glBindImageTexture(0, m_textureHistoPyramid, i, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, m_textureHistoPyramid, i+1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F); // bind point 1 in shader, level 1 in pyr volume
		
		glm::uvec3 nthreads = GLHelper::divup(glm::uvec3((512 >> i ) / 2, (512 >> i) / 2, (512 >> i) / 2), lSize);
		glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_constructHPLevelID);
		glUniform1i(m_baseLevelID, i);

		glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	///// Read top of pyramid to get total number of triangles in volume
	//std::vector<uint32_t> sumData8((512 >> 8) * (512 >> 8) * (512 >> 8), 2);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, m_textureHistoPyramid);
	//glGetTexImage(GL_TEXTURE_3D, 8, GL_RED_INTEGER, GL_UNSIGNED_INT, sumData8.data());
	//glBindTexture(GL_TEXTURE_3D, 0);
	//glActiveTexture(0);

	//for (auto i : sumData8 )
	//	std::cout << i <<  " ";



	/// Read top of pyramid to get total number of triangles in volume
	std::vector<float> sumData((512 >> 9) * (512 >> 9) * (512 >> 9), 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureHistoPyramid);
	glGetTexImage(GL_TEXTURE_3D, 9, GL_RED, GL_FLOAT, sumData.data());
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(GL_TEXTURE0);


	m_totalSum = sumData[0] * 3; // total num verts

	//std::cout << "num of m_totalSum " << m_totalSum << std::endl;

	//for (size_t i = 0; i < outData0.size(); i += 1000)
	//{
	//	std::cout << outData0[i] << " ";
	//}



	traverseHistoPyramidsProg.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureHistoPyramid);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, m_textureEdgeTable);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D, m_textureTriTable);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, m_textureNrOfTriangles);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_1D, m_textureOffsets3);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_3D, m_textureviewHistoPyramid);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * m_totalSum * 4, NULL, GL_DYNAMIC_DRAW); // this is the * 9 buffer in FAST vec3 * 3

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferNorm);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * m_totalSum * 5, NULL, GL_DYNAMIC_DRAW);


	/// Traverse HP level
	glBindImageTexture(2, m_textureHistoPyramid, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16F);
	nthreads = GLHelper::divup(glm::uvec3(sumData[0], 1, 1), glm::uvec3(32, 1, 1));
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_traverseHPLevelID);
	glUniform1ui(m_totalSumID, sumData[0]);
	glUniform1f(m_isoLevel_TravID, m_isoLevel);

	glm:vec2 scaleVec(0.0f, 512.0f);
	glUniform2fv(m_scaleVecID, 1, glm::value_ptr(scaleVec));

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_bufferPosEncode);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferNorm);


	glDispatchCompute(nthreads.x, nthreads.y, nthreads.z);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	glEndQuery(GL_TIME_ELAPSED);
	GLuint available = 0;
	while (!available) {
		glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	}
	// elapsed time in nanoseconds
	GLuint64 elapsed;
	glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
	auto hpTime = elapsed / 1000000.0;

	//std::cout << "elapsed time : " << hpTime << std::endl;

	//std::vector<float> posData(512 * 512 * 512 * sizeof(uint32_t));
	//std::vector<uint32_t> posDataOri(512 * 512 * 512 * sizeof(uint32_t));
	//std::vector<uint32_t> pD(512 * 512 * 512 * sizeof(uint32_t));



	//std::vector<float> normData(512*512*512, 1);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferNorm);
	//void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(normData.data(), normData.size() * sizeof(float), ptr, normData.size() * sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPosEncode);
	//void *ptrEnc = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	//memcpy_s(pD.data(), pD.size() * sizeof(uint32_t), ptrEnc, pD.size() * sizeof(uint32_t));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//int j = 0;
	//for (int i = 0; i < pD.size(); i++, j += 3)
	//{
	//	posDataOri[j] = (pD[i] & (1023 << 20)) >> 20;
	//	posDataOri[j+1] = (pD[i] & (1023 << 10)) >> 10;
	//	posDataOri[j+2] = (pD[i] & (1023 << 0)) >> 0;

	//}

	//int j = 0;
	//for (int i = 0; i < pD.size(); i++, j += 3)
	//{
	//	posDataOri2[j] = (pD[i] & (1023 << 20)) >> 20;
	//	posDataOri2[j + 1] = (pD[i] & (1023 << 10)) >> 10;
	//	posDataOri2[j + 2] = (pD[i] & (1023 << 0)) >> 0;


	//}

	//std::string modelFileName = "data/meshes/marchingCubesBin.stl";


	//std::ofstream outFile(modelFileName, std::ios::out | std::ios::binary);

	//if (!outFile)
	//{
	//	//cerr << "Error opening output file: " << FileName << "!" << endl;
	//	printf("Error opening output file: %s!\n", modelFileName);
	//	exit(1);
	//}

	//char hdr[80];

	//int pointNum = totalSum;
	//uint32_t NumTri = totalSum / 3;
	//uint16_t attributeByteCount = 0;

	//outFile.write(hdr, 80);
	//outFile.write((char*)&NumTri, sizeof(uint32_t));

	//for (int i = 0; i < totalSum; i++) 
	//{

	//}

	//for (int pi = 0; pi < pointNum; pi += 3)
	//{
	//	const float* point = (float*)(&posData[pi]);
	//	outFile << point[0] << " " << point[1] << " " << point[2] << std::endl;
	//}



}

void MCubes::exportMesh()
{
	std::vector<float> posData(m_totalSum * 4);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPosEncode);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(posData.data(), posData.size() * sizeof(float), ptr, posData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	std::string modelFileName = "data/meshes/marchingCubesBin.ply";


	std::ofstream outFile(modelFileName, std::ios::out | std::ios::binary);

	if (!outFile)
	{
		//cerr << "Error opening output file: " << FileName << "!" << endl;
		printf("Error opening output file: %s!\n", modelFileName);
		exit(1);
	}

	int pointNum = m_totalSum;

	outFile << "ply" << std::endl;
	outFile << "format ascii 1.0" << std::endl;
	outFile << "element vertex " << pointNum << std::endl;
	outFile << "property float x" << std::endl;
	outFile << "property float y" << std::endl;
	outFile << "property float z" << std::endl;
	
	outFile << "end_header" << std::endl;
	for (int pi = 0; pi < pointNum * 4; pi += 4)
	{
		const float* point = (float*)(&posData[pi]);
		outFile << point[0] << " " << point[1] << " " << point[2] << std::endl;
	}
}


void MCubes::generateMarchingCubes()
{
	histoPyramids();


	//int threads = 128;

	//// CLASSIFY VOXEL
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//glBindImageTexture(2, m_textureTriTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	//// SET UNIFORMS
	//glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(m_mcubeConfiguration.gridSize));
	//glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(m_mcubeConfiguration.gridSizeShift));
	//glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(m_mcubeConfiguration.gridSizeMask));
	//glUniform1f(m_isoValueID, m_mcubeConfiguration.isoValue);
	//glUniform1ui(m_numVoxelsID, m_mcubeConfiguration.numVoxels);

	//int xthreads = divup(m_mcubeConfiguration.numVoxels, threads);
	//int ythreads = 1;
	//if (xthreads > 65535)
	//{
	//	ythreads = xthreads / 32768;
	//	xthreads = 32768;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_classifyVoxelID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);




	//// PREFIX SUM
	//m_mcubeConfiguration.activeVoxels = prefixSum(m_bufferVoxelOccupied, m_bufferVoxelOccupiedScan);
	////std::cout << "active voxels " << mcubeConfiguration.activeVoxels << std::endl;

	//// COMPACT VOXELS
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	//// BIND UNIFORMS
	//glUniform1ui(m_numVoxelsID, m_mcubeConfiguration.numVoxels);

	//xthreads = divup(m_mcubeConfiguration.numVoxels, threads);
	//ythreads = 1;
	//if (xthreads > 65535)
	//{
	//	ythreads = xthreads / 32768;
	//	xthreads = 32768;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_compactVoxelsID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//// PREFIX SUM
	//m_totalVerts = prefixSum(m_bufferVoxelVerts, m_bufferVoxelVertsScan);
	////std::cout << "total verts " << m_totalVerts << std::endl;

	//// GENERATE TRIANGLES
	//marchingCubesProg.use();
	//// BIND TEXTURES
	//glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	//glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//glBindImageTexture(2, m_textureTriTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	//// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferPos);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_bufferNorm);
	//// BIND UNIFORMS
	//glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(m_mcubeConfiguration.gridSize));
	//glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(m_mcubeConfiguration.gridSizeShift));
	//glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(m_mcubeConfiguration.gridSizeMask));
	//glUniform1f(m_isoValueID, m_mcubeConfiguration.isoValue);
	//glUniform1ui(m_numVoxelsID, m_mcubeConfiguration.numVoxels);
	//glUniform1ui(m_activeVoxelsID, m_mcubeConfiguration.activeVoxels);
	//glUniform1ui(m_maxVertsID, m_mcubeConfiguration.maxVerts);
	//glUniform3fv(m_voxelSizeID, 1, glm::value_ptr(m_mcubeConfiguration.voxelSize));

	//xthreads = divup(m_mcubeConfiguration.activeVoxels, threads);
	//ythreads = 1;
	//while (xthreads > 65535)
	//{
	//	xthreads /= 2;
	//	ythreads *= 2;
	//}
	//// LAUNCH SHADER
	//glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_generateTrianglesID);
	//glDispatchCompute(xthreads, ythreads, 1);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);
}











void MCubes::loadTables()
{
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_1D, m_textureEdgeTable);

	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED_INTEGER, GL_UNSIGNED_SHORT, &edgeTable[0]);

	glBindTexture(GL_TEXTURE_1D, m_textureTriTable);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256 * 16, GL_RED_INTEGER, GL_UNSIGNED_SHORT, &triTable[0]);

	glBindTexture(GL_TEXTURE_1D, m_textureNumVertsTable);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED_INTEGER, GL_UNSIGNED_SHORT, &numVertsTable[0]);

	glBindTexture(GL_TEXTURE_1D, m_textureNrOfTriangles);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &nrOfTriangles[0]);

	glBindTexture(GL_TEXTURE_1D, m_textureOffsets3);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 72, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &offsets3[0]);


	//std::vector<uint16_t> outData(256);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_1D, m_textureNumVertsTable);
	//glGetTexImage(GL_TEXTURE_1D, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, outData.data());
	//glBindTexture(GL_TEXTURE_1D, 0);
	//glActiveTexture(0);

	 


}