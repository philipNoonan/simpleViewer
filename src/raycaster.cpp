#include "raycaster.h"

void RCaster::init()
{
	compileAndLinkShader();
	setLocations();
	allocateTextures();
	allocateBuffers();
}


void RCaster::compileAndLinkShader()
{
	try {
		raycastProg.compileShader("shaders/raycast.cs");
		raycastProg.link();
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}
void RCaster::setLocations()
{
	m_invModelID_r = glGetUniformLocation(raycastProg.getHandle(), "invModel");
	m_invViewID_r = glGetUniformLocation(raycastProg.getHandle(), "invView");
	m_invProjID_r = glGetUniformLocation(raycastProg.getHandle(), "invProj");

	m_nearPlaneID = glGetUniformLocation(raycastProg.getHandle(), "nearPlane");
	m_farPlaneID = glGetUniformLocation(raycastProg.getHandle(), "farPlane");
	m_stepID = glGetUniformLocation(raycastProg.getHandle(), "step");
	m_largeStepID = glGetUniformLocation(raycastProg.getHandle(), "largeStep");
	m_volDimID_r = glGetUniformLocation(raycastProg.getHandle(), "volDim");
	m_volSizeID_r = glGetUniformLocation(raycastProg.getHandle(), "volSize");
	m_screenSizeID = glGetUniformLocation(raycastProg.getHandle(), "screenSize");

}
void RCaster::allocateTextures()
{
	m_textureVertices = GLHelper::createTexture(m_textureVertices, GL_TEXTURE_2D, 1, m_screenWidth, m_screenHeight, 1, GL_RGBA32F);
	m_textureNormals = GLHelper::createTexture(m_textureNormals, GL_TEXTURE_2D, 1, m_screenWidth, m_screenHeight, 1, GL_RGBA32F);

}
void RCaster::allocateBuffers()
{

}




void RCaster::raycast()
{
	//glBeginQuery(GL_TIME_ELAPSED, query[3]);




	raycastProg.use();

	//glm::mat4 view = m_invPose * m_invK;

	//view[3][0] = view[3][0];// *128.0f + 256.0f; // plus...
	//view[3][1] = view[3][1];// *128.0f + 256.0f; // pluss ... offset?
	//view[3][2] = view[3][2];// *128.0f + 256.0f;


	//std::cout << m_invPose[3][0] << " " << m_invPose[3][1] << " " << m_invPose[3][2] << std::endl;

	float step = 1.0f;

	// bind uniforms
	glUniformMatrix4fv(m_invViewID_r, 1, GL_FALSE, glm::value_ptr(m_invView));
	glUniformMatrix4fv(m_invProjID_r, 1, GL_FALSE, glm::value_ptr(m_invProj));
	glUniformMatrix4fv(m_invModelID_r, 1, GL_FALSE, glm::value_ptr(m_invModel));

	
	/*glUniform1f(m_nearPlaneID, 0.1);
	glUniform1f(m_farPlaneID, 1000.0);
	glUniform1f(m_stepID, step);
	glUniform1f(m_largeStepID, 1.0f);
	glUniform3fv(m_volDimID_r, 1, glm::value_ptr(glm::vec3(1.0f)));
	glUniform3fv(m_volSizeID_r, 1, glm::value_ptr(glm::vec3(512.0f)));
	glUniform2uiv(m_screenSizeID, 1, glm::value_ptr(glm::uvec2(m_screenWidth, m_screenHeight)));*/

	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	glBindImageTexture(1, m_textureVertices, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureNormals, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	//glBindImageTexture(3, m_textureTestImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);



	//glBindImageTexture(1, m_textureVolumeSliceNorm, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(2, m_textureVolumeColor, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// bind the volume texture for 3D sampling
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolumeColor);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textureColor);*/

	int xWidth;
	int yWidth;


	xWidth = GLHelper::divup(1024, 32);
	yWidth = GLHelper::divup(1024, 32);


	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[3], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[3], GL_QUERY_RESULT, &elapsed);
	//raycastTime = elapsed / 1000000.0;

	//std::cout << raycastTime << std::endl;

	//cv::Mat testIm(m_screenHeight, m_screenWidth, CV_32FC4);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureVertices);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testIm.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::Mat image0[4];
	//cv::split(testIm, image0);


	//cv::imshow("0", image0[0]);
	//cv::imshow("1", image0[1]);
	//cv::imshow("2", image0[2]);

}