#include "cta-viewer.h"

namespace fs = std::filesystem;

static void ShowMenuFile()
{
	ImGui::MenuItem("(...)", NULL, false, false);
	if (ImGui::MenuItem("New")) {}
	if (ImGui::BeginMenu("Open")) 
	{
		for (auto const & val : imageFiles)
		{
			auto filename = val.string();
				//.filename();
			//string tempStr = filename.string();
			if (ImGui::MenuItem(filename.c_str()))
			{
				imageVolumeFileName = filename;
				setVolume();
				std::cout << "clicked " + filename << std::endl;
			}
		}
		
		ImGui::EndMenu();
	}
	
	
}

void setVolume()
{
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	renderer.allocateTextures();

	loaderer.openFile(imageVolumeFileName);

	//void * imagePointer = loaderer.getImagePtr();
	vtkSmartPointer<vtkImageData> imageDataPtr = loaderer.getImagePtr();
	
	renderer.uploadImageData(imageDataPtr);
	int dims[3];
	imageDataPtr->GetDimensions(dims);


	mcconfig.gridSize = glm::uvec3(dims[0], dims[1], dims[2]);
	mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
	mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

	mcubes.setConfig(mcconfig);



	mcubes.setVolumeTexture(renderer.getVolumeTexture());
	mcubes.init();

	rcaster.setScreenWidth(display_w);
	rcaster.setScreenHeight(display_h);

	rcaster.init();
	rcaster.setVolumeTexture(renderer.getVolumeTexture());

	renderer.setRaycastTexture(rcaster.getVertexTexture());

	octree.init(); // move into the load volume when shaders are stable
	renderer.setOctlistBuffer(octree.getOctlistBuffer());
	rcaster.setOctreeTexture(octree.getOctreeTexture());

	renderer.setPosBuffer(mcubes.getPosBuffer());
	renderer.setNormBuffer(mcubes.getNormBuffer());
	renderer.allocateBuffersFromMarchingCubes();

	//renderer.setOctlistBuffer(octree.getOctlistBuffer());
    renderer.allocateBuffersForOctree();
	octree.setInputFloatVolume(renderer.getVolumeTexture());

	loaderer.~Loader();


}

int main()
{
	std::vector<float> meshData(1, 0);
	std::vector<float> boxBot(3, 0);
	std::vector<float> boxTop(3, 0);






	// start GLWF context
	window = renderer.loadGLFWWindow();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);


	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	renderer.SetCallbackFunctions();
	renderer.compileAndLinkShader();
	renderer.setLocations();
	renderer.setVertPositions();
	
	renderer.allocateBuffers();

	// Camera
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(45.0f, float(display_w) / float(display_h), 0.01, 1000.0f);

	renderer.setCamera(&camera);

	voxelizer.init(); // MOVE ME WHEN THE SHADERS WORK

		
	//voxelizer.voxelize(true);


	//octree.init(); // move into the load volume when shaders are stable
	octree.setInputFloatVolume(voxelizer.getVolumeTexture());
	//renderer.setOctlistBuffer(octree.getOctlistBuffer());
	//renderer.allocateBuffersForOctree();


	glEnable(GL_DEPTH_TEST);



	imageFiles.resize(0);
	fs::path currPath = fs::current_path();
	if (fs::exists(currPath /= "resources"))
	{
		for (const auto& entry : fs::directory_iterator(currPath))
		{
			auto filename = entry.path().filename();
			std::cout << filename << std::endl;
			imageFiles.push_back(entry);
		}
	}

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();




		// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(0, 0, 0, 255);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		renderer.setNumTrianglesMC(mcubes.getNumberTriangles());

		renderer.bindTexturesForRendering();

		renderer.setRenderOthroFlag(renderOrtho);
		renderer.setRenderMarchingCubesFlag(performMarchingCubes);
		renderer.setRenderRaytraceFlag(performRaytrace);
		renderer.setRenderOctlistFlag(performOctree || performVoxelization);

		renderer.render();

		rcaster.setScreenHeight(display_h);
		rcaster.setScreenWidth(display_w);
		rcaster.setInverseProjection(renderer.getInverseProjection());
		rcaster.setInverseModel(renderer.getInverseModel());
		rcaster.setInverseView(glm::inverse(camera.matrices.view));
		rcaster.setFastRaytraceFlag(performFastRaytrace);
		rcaster.setThresh(m_isoLevel);

		if (performRaytrace)
		{
			rcaster.raycast();
		}







		ImGui::SetNextWindowPos(ImVec2(32, 32));
		ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		GLint total_mem_kb = 0;
		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
			&total_mem_kb);

		GLint cur_avail_mem_kb = 0;
		glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
			&cur_avail_mem_kb);



		bool showGUI = true;
		bool imguiFocus = false;
		ImGui::Begin("Menu", &showGUI, window_flags);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				ShowMenuFile();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("GPU Mem %d MB out of %d (%.1f %%)", (total_mem_kb - cur_avail_mem_kb) / 1024, total_mem_kb / 1024, 100.0f * (1.0f - (float)cur_avail_mem_kb / (float)total_mem_kb));
		ImGui::Separator();

		if (ImGui::Button("Rescan"))
		{
			imageFiles.resize(0);
			fs::path currPath = fs::current_path();
			if (fs::exists(currPath /= "resources"))
			{
				for (const auto& entry : fs::directory_iterator(currPath))
				{
					auto filename = entry.path().filename();
					std::cout << filename << std::endl;
					imageFiles.push_back(entry);
				}
			}

		}

		
		//bool showWin = true;
		//{
		//	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		//	ImGui::ShowDemoWindow(&showWin);
		//}

		float oldIso = m_isoLevel;
		ImGui::SliderFloat("isolevel", &m_isoLevel, 0.1f, 2000.0f);
		if (m_isoLevel != oldIso)
		{
			mcubes.setIsolevel(m_isoLevel);

			mcubes.generateMarchingCubes();

			octree.setIsoLevel(m_isoLevel);
			octree.setCutoff(m_cutoff);

			octree.buildTree();
			octree.createList();
			renderer.setOctlistCount(octree.getLength());


		}
		if (ImGui::Button("Ortho"))			renderOrtho ^= 1;		ImGui::SameLine(); ImGui::Checkbox("", &renderOrtho);

		if (ImGui::Button("Raytrace"))			performRaytrace ^= 1;		ImGui::SameLine(); ImGui::Checkbox("", &performRaytrace); 	ImGui::SameLine();	if (ImGui::Button("Fast"))			performFastRaytrace ^= 1;		ImGui::SameLine(); ImGui::Checkbox("", &performFastRaytrace);

		if (ImGui::Button("Marching Cubes"))
		{
			mcubes.setIsolevel(m_isoLevel);
			mcubes.generateMarchingCubes();
			performMarchingCubes ^= 1;
		}
		
		ImGui::SameLine(); ImGui::Checkbox("", &performMarchingCubes);
		
		if (ImGui::Button("Octree"))
		{
			performOctree ^= 1;
			octree.setInputFloatVolume(renderer.getVolumeTexture());
			octree.setIsoLevel(m_isoLevel);
			octree.setCutoff(m_cutoff);

			octree.buildTree();
			octree.createList();
			renderer.setOctlistCount(octree.getLength());

		}
		ImGui::SameLine(); ImGui::Checkbox("", &performOctree);

		if (ImGui::Button("Voxelizer"))
		{
			performVoxelization ^= 1;
			if (performVoxelization)
			{
				loadBinarySTLToVertArray("resources/brainBin.stl", meshData);
				getBoundingBox(meshData, boxTop, boxBot);
				float longestEdge = boxTop[0] - boxBot[0];
				if (boxTop[1] - boxBot[1] > longestEdge) longestEdge = boxTop[1] - boxBot[1];
				if (boxTop[2] - boxBot[2] > longestEdge) longestEdge = boxTop[2] - boxBot[2];

				voxelizer.setVertexArray(meshData);
				voxelizer.configInfo(1.0f / (512.0f / longestEdge), meshData.size() / 9, boxBot, 512);

				voxelizer.voxelize(true);

				octree.setInputFloatVolume(voxelizer.getVolumeTexture());
				octree.setIsoLevel(-1.0f);

				octree.buildTree();
				octree.createList();
				renderer.setOctlistCount(octree.getLength());
			}


		}
		ImGui::SameLine(); ImGui::Checkbox("", &performVoxelization);


		if (ImGui::Button("Voxels"))
		{
			renderVoxels = !renderVoxels;

			renderer.setRenderVoxels(renderVoxels);

		}
		ImGui::SameLine(); ImGui::Checkbox("", &renderVoxels);




		if (ImGui::Button("Export Mesh"))
		{
			//mcubes.generateMarchingCubes();
			mcubes.exportMesh();
		}

		ImGui::Text("Octree Options");
		{
			
			if (ImGui::SliderInt("cutoff", &m_cutoff, 0, 8))
			{
				octree.setIsoLevel(m_isoLevel);
				octree.setCutoff(m_cutoff);

				octree.buildTree();
				octree.createList();
				renderer.setOctlistCount(octree.getLength());
			}


		}

		ImGui::Text("Render Options");
		{
			ImGui::BeginGroup();
			ImGui::SliderInt("pyr level", &m_level, 0, 9);



			ImGui::SliderFloat("sliceX", &m_x_slice, 0.0f, 1.0f);
			ImGui::SliderFloat("sliceY", &m_y_slice, 0.0f, 1.0f);
			ImGui::SliderFloat("sliceZ", &m_z_slice, 0.0f, 1.0f);
			imguiFocus = ImGui::IsAnyItemActive();
			ImGui::EndGroup();
			renderer.setLevel(m_level);
			renderer.setOrthoVerts(m_x_slice, m_y_slice, m_z_slice);
		}




		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) && imguiFocus == false)
		{
			ImVec2 mPos = ImGui::GetMousePos();
			mousePos.x = mPos.x;
			mousePos.y = mPos.y;
		}

		if (ImGui::IsMouseDragging(0) && imguiFocus == false)
		{
			// get mouse dragging states and pixel locations
			ImVec2 mPos = ImGui::GetMousePos();
			
			rotation.x += (mousePos.y - mPos.y) * 1.25f * 0.1f; // 1.0f == rotation speed change for faster 
			rotation.y -= (mousePos.x - mPos.x) * 1.25f * 0.1f;
			renderer.setRotation(rotation);

			mousePos.x = mPos.x;
			mousePos.y = mPos.y;
			//camera.rotate(glm::vec3((mousePos.y - (float)posy) * camera.rotationSpeed, -(mousePos.x - (float)posx) * camera.rotationSpeed, 0.0f));
			//mousePos = glm::vec2((float)posx, (float)posy);
			//viewUpdated = true;

		}

		if (ImGui::IsMouseDragging(1) && imguiFocus == false)
		{
			ImVec2 mPos = ImGui::GetMousePos();

			cameraPos.x -= (mousePos.x - mPos.x) * 0.01f;
			cameraPos.y += (mousePos.y - mPos.y) * 0.01f;
			renderer.setCameraPos(cameraPos);
			mousePos.x = mPos.x;
			mousePos.y = mPos.y;
		}
		if (io.MouseWheel != 0.0f)
		{
			renderer.setZoom(io.MouseWheel);
		}



		ImGui::End();
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		camera.update(0.01f);

		//std::cout << camera.matrices.view[3][0] << std::endl;

		glfwSwapBuffers(window);



	}



	// Cleanup

	
	renderer.cleanup();
	mcubes.cleanup();
	octree.cleanup();

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();





	return 0;
}





