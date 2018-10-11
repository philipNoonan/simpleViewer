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

	for (uint32_t i = 0; i < tris.size() - 9; i+=9) {

		in.read((char *)points, 12 * sizeof(float));
		for (int j = 3; j < 12; j++) 
		{
			tris[i - 3 + j] = points[j];
		}
		in.read((char *)&attr, sizeof(uint16_t));

	}
	in.close();
}