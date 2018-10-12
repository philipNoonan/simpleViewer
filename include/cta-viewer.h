

#include "render.h"
#include "loader.h"
#include "mcubes.h"
#include "raycaster.h"
#include "octree.h"
#include "voxelizer.h"
#include "meshModel.h"

#include "camera.hpp"


#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"


#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>

#include <algorithm>

#include <filesystem>

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

Render renderer;
Loader loaderer;

MCubes mcubes;
mCubeConfig mcconfig;

RCaster rcaster;

Octree octree;

Voxelizer voxelizer;

Camera camera;

GLFWwindow * window;

void setVolume();

int m_level = 0;
int m_cutoff = 0;
float m_x_slice = 0.5f;
float m_y_slice = 0.5f;
float m_z_slice = 0.5f;

float m_isoLevel = 1500.0f;

glm::vec3 rotation = glm::vec3();
glm::vec3 cameraPos = glm::vec3();
glm::vec2 mousePos = glm::vec2();

bool renderOrtho = true;
bool performRaytrace = false; bool performFastRaytrace = false;
bool performMarchingCubes = false;
bool performOctree = false;

std::vector<std::filesystem::path> imageFiles;
string imageVolumeFileName;
