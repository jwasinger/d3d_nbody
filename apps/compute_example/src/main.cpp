//#include <Windows.h>

#include <iostream>
#include <vector>
#include <tuple>
#include <memory>
#include <opencv.hpp>
#include <math.h>
#include <D3dx9tex.h>
#include "BufType.h"
#include "ComputeShader.h"

#define USE_TEXTURE

using namespace std;
using namespace cv;


int main(int argc, wchar_t** argv)
{
	ComputeShader cs;

	if (!cs.CompileShader(L"ComputeShader.hlsl", "main", 32, 32, 1))
		return 0;
	
	Mat img = imread("test.jpg");
	if (!img.isContinuous())
		return 0;

	Mat imgfloat3(cvSize(img.cols, img.rows), CV_32FC3);

	img.convertTo(imgfloat3, CV_32FC3);

	vector<tuple<void *, unsigned int, unsigned int>> InputData;
	vector<tuple<unsigned int, unsigned int>> OutputData;
	vector<tuple<void *, unsigned int, unsigned int>> ConstantData;

	ImageDims dimImage = { img.rows, img.cols };

	BlurKernel blurKernel = 
	{
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f,
		0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f, 0.01234f
	};

	InputData.push_back(make_tuple(imgfloat3.data, (UINT) sizeof(PixelColor), img.rows * img.cols));
	OutputData.push_back(make_tuple((UINT) sizeof(PixelColor), img.rows * img.cols));
	ConstantData.push_back(make_tuple(&dimImage, (UINT) sizeof(dimImage), 1));
	ConstantData.push_back(make_tuple(&blurKernel, (UINT) sizeof(blurKernel), 1));

	if (!cs.RunShader(80, 50, 1, InputData, OutputData, ConstantData))
		return 0;

	cout << "Executed in " << cs.GetExecutionTime() << endl;
	//cs.RunComputeShader(450, 275, 1, InputData, OutputData, ConstantData);
	imgfloat3.data = cs.Result<uchar>(0);

	imgfloat3.convertTo(img, CV_8UC3);
	imwrite("./output.jpg", img);

	imshow("Blurred Image", img);
	waitKey(0);

	return 0;
}