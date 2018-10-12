#include <fstream>
#include <iostream>
#include <vector>



void loadBinarySTLToVertArray(std::string file, std::vector<float> &tris) {
	tris.clear();
	char buffer[1024];
	float points[12]; // norm and verts
	uint16_t attr; // should be blank
	std::ifstream in(file, std::ios::in | std::ios::binary);
	in.read(buffer, 80 * sizeof(uint8_t));	// UINT8[80] -- Header
	in.read(buffer, sizeof(uint32_t));		// UINT32 -- number of triangles
	uint32_t numTriangles = *(uint32_t *)buffer;
	tris.resize(numTriangles * 9,0);

	for (uint32_t i = 0; i < tris.size(); i+=9) {

		in.read((char *)points, 12 * sizeof(float));
		for (int j = 3; j < 12; j++) 
		{
			tris[i - 3 + j] = points[j];
		}
		in.read((char *)&attr, sizeof(uint16_t));

	}
	in.close();
}


void getBoundingBox(const std::vector<float> vs, std::vector<float> &top, std::vector<float> &bot) {
	top[0] = bot[0] = vs[0];
	top[1] = bot[1] = vs[1];
	top[2] = bot[2] = vs[2];

	for (std::vector<float>::const_iterator it = vs.cbegin(); it != vs.cend(); it+=3) {
		if (*(it + 0) > top[0]) top[0] = *(it + 0);
		if (*(it + 1) > top[1]) top[1] = *(it + 1);
		if (*(it + 2) > top[2]) top[2] = *(it + 2);
		if (*(it + 0) < bot[0]) bot[0] = *(it + 0);
		if (*(it + 1) < bot[1]) bot[1] = *(it + 1);
		if (*(it + 2) < bot[2]) bot[2] = *(it + 2);
	}
}