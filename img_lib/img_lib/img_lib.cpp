// img_lib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "img_lib.h"

#include<opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


double* CapImg(int *width, int *height) {
	std::cout << "enter dll\n";
	VideoCapture capture(0);
	while (true) {
		if (13 == waitKey(20)) {
			return nullptr;
		}
		Mat frame;
		capture >> frame;
		if (32 == waitKey(20)) {
			imwrite("cap.jpg", frame);
			cout << "capture\n";
			Mat gray;
			cvtColor(frame, gray, CV_BGR2GRAY);
			*width = gray.cols;
			*height = gray.rows;
			double *data = new double[gray.cols * gray.rows];
			int idx = 0;
			for (int i = 0; i < *height; i++)
			{
				for (int j = 0; j < *width; j++)
				{
					data[idx++] = double(gray.at<uchar>(i,j));
				}
			}
			return data;
		}
		imshow("cap", frame);
	}
}


void DLLFree(void *data) {
	delete[](data);
}



