#include <iostream>
#include <fstream>
#include <vector>

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>

#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>


class Loader
{
public:
	Loader() {};
	~Loader() {	};

	vtkSmartPointer<vtkImageData> getImagePtr()
	{
		return m_imageData; // m_inputImageVolume.data();
	}

	void openFile(std::string fileName)
	{
		if (fileName.substr(fileName.find_last_of(".") + 1) == "gz")
		{
			vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
			//vtkSmartPointer<vtkDICOMImageReader> reader =
			//	vtkSmartPointer<vtkDICOMImageReader>::New();
			reader->SetFileName(fileName.c_str());
			reader->Update();
			//int imType = reader->GetDataScalarType();
			

			//m_imageData = vtkSmartPointer<vtkImageData>::New();
			m_imageData = reader->GetOutput();

			//int dims[3];
			//imageData->GetDimensions(dims);
			
			//m_imagePtr = imageData->GetScalarPointer();

			//m_inputImageVolume.resize(dims[0] * dims[1] * dims[2]);
			//memcpy_s(m_inputImageVolume.data(), dims[0] * dims[1] * dims[2] * sizeof(float), imageData->GetScalarPointer(), dims[0] * dims[1] * dims[2] * sizeof(float));

			//std::vector<float>::const_iterator first = inputImageVolume.begin() + dims[0] * dims[1] * 30;
			//std::vector<float>::const_iterator last = inputImageVolume.begin() + dims[0] * dims[1] * 31;
			//std::vector<float> newVec(first, last);

			//cv::Mat niiftiVol(dims[0], dims[1], CV_32FC1, newVec.data());

			//cv::imshow("slice", niiftiVol / 1000.0f);
			//cv::waitKey(1);
			

			//return openZippedFile(fileName);
		}
		else if(fileName.substr(fileName.find_last_of(".") + 1) == "img")
		{
			//return openUnzippedFile(fileName);
		}
		else
		{
			//vtkSmartPointer<vtkNIFTIImageReader> reader =
			//	vtkSmartPointer<vtkNIFTIImageReader>::New();
			////vtkSmartPointer<vtkDICOMImageReader> reader =
			////	vtkSmartPointer<vtkDICOMImageReader>::New();
			//reader->SetFileName(fileName.c_str());
			//reader->Update();


		}
	}
	


private:
	std::vector<char> openUnzippedFile(std::string fileName);
	std::vector<char> openZippedFile(std::string fileName);
	vtkSmartPointer<vtkImageData> m_imageData;
	std::vector<float> m_inputImageVolume;


};