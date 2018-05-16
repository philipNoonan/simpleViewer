#include "loader.h"

std::vector<char> Loader::openZippedFile(std::string fileName)
{
	// using zlib gzip file access functions
	//char outbuffer[348];
	//gzFile infile = (gzFile)gzopen(fileName.c_str(), "rb");

	std::vector<char> outfile;
	//gzrewind(infile);
	//while (!gzeof(infile))
	//{
	//	int len = gzread(infile, outbuffer, sizeof(outbuffer));
	//	outfile.insert(outfile.end(), outbuffer, outbuffer + len);

	//	// USE ME https://github.com/GuillaumeGas/NiftiReader/blob/master/src/Nifti.hpp
	//}
	//gzclose(infile);

	return outfile;

}

std::vector<char> Loader::openUnzippedFile(std::string fileName)
{

	std::vector<char> outfile(1,0);


	return outfile;
}