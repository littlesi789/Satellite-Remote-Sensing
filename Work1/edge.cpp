#include <iostream>
#include <fstream>
#include <gdal/gdal_priv.h>
#include <opencv/cv.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

int main() {
    GDALAllRegister();

	const char* imgFilePath = "/home/ricky/Desktop/MDP/satellite-Remote-Sensing/Work1/arizona/rgb.jpg";

	cout << "NIR channel filename: " << imgFilePath << endl;
	GDALDataset *rgb;
	int nRows, nCols;
	double noData;
	double transform[6];

	// Load input images (NIR and mir bands)
	rgb = (GDALDataset*)GDALOpen(imgFilePath, GA_ReadOnly);

	// Get raster info to use as parameters
	nRows = rgb->GetRasterBand(1)->GetYSize();
	nCols = rgb->GetRasterBand(1)->GetXSize();
	noData = rgb->GetRasterBand(1)->GetNoDataValue();
	rgb->GetGeoTransform(transform);
	cout << "Number of Rows: " << nRows << endl;
	cout << "Number of Columns: " << nCols << endl;

	// Allocate memories.
	float *rgb_Row = (float*)CPLMalloc(sizeof(float)*nCols);
	float *edge_Row = (float*)CPLMalloc(sizeof(float)*nCols);

	cout << "--Creating Files--" << endl;
	// Create pmg FILE
	Mat edge_image=Mat::zeros(nRows,nCols,CV_8UC1), color_image;
    
	
	cout << "--Loop through pixels--" << endl;
	for (int i = 1; i < nRows - 1; i++)
	{
		(void)rgb->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, rgb_Row, nCols, 1, GDT_Float32, 0, 0);
		for (int j = 0; j < nCols; j++)
		{
			if (rgb_Row[j] == noData)
			{
				edge_Row[j] = noData;
				cout << "-----------------no data------------"<< endl;
			}
			else
			{
				edge_Row[j] = rgb_Row[j-1] - rgb_Row[j+1];
			}
			// int extended_index = ((edge_Row[j] + 1) * 255) / 2 <= 255? ((edge_Row[j] + 1) * 255) / 2:255;
			// extended_index = extended_index >= 0 ? extended_index : 0;
			// edge_image.at<uchar>(i,j) = extended_index; //G
			edge_image.at<uchar>(i,j) = edge_Row[j];
		}
	}
	cout << "output image" << endl;
	imwrite("edge_visualization.tif",edge_image);
	// delete buffers
	CPLFree(rgb_Row);
	CPLFree(edge_Row);

	// delete datasets
	GDALClose(rgb);
	GDALDestroyDriverManager();
	cout << "Generation Success" << endl;

	return 0;
}