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

	const char* nirFilePath = "/home/ricky/Desktop/MDP/Work1/arizona/banda4.tif";
	const char* redFilePath = "/home/ricky/Desktop/MDP/Work1/arizona/banda3.tif";
	const char* outFilePath = "out_ndvi.pmg";

	cout << "NIR channel filename: " << nirFilePath << endl;
	cout << "Red channel filename: " << redFilePath << endl;
	cout << "Output NDVI filename: " << outFilePath << endl;
	GDALDataset *red, *nir, *ndvi;
	GDALDriver *geoTiff;
	int nRows, nCols;
	double noData;
	double transform[6];

	// Load input images (NIR and RED bands)
	nir = (GDALDataset*)GDALOpen(nirFilePath, GA_ReadOnly);
	red = (GDALDataset*)GDALOpen(redFilePath, GA_ReadOnly);

	// Get raster info to use as parameters
	nRows = nir->GetRasterBand(1)->GetYSize();
	nCols = nir->GetRasterBand(1)->GetXSize();
	noData = nir->GetRasterBand(1)->GetNoDataValue();
	nir->GetGeoTransform(transform);
	cout << "Number of Rows: " << nRows << endl;
	cout << "Number of Columns: " << nCols << endl;

	// Allocate memories.
	float *nir_Row = (float*)CPLMalloc(sizeof(float)*nCols);
	float *red_Row = (float*)CPLMalloc(sizeof(float)*nCols);
	float *upper_Row = (float*)CPLMalloc(sizeof(float)*nCols);
	float *lower_Row = (float*)CPLMalloc(sizeof(float)*nCols);
	float *ndvi_Row = (float*)CPLMalloc(sizeof(float)*nCols);

	cout << "--Creating Files--" << endl;
	// Create pmg FILE
	Mat ndvi_image=Mat::zeros(nRows,nCols,CV_8UC1), color_image;
    
	
	cout << "--Loop through pixels--" << endl;
	for (int i = 0; i < nRows; i++)
	{
		(void)nir->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, nir_Row, nCols, 1, GDT_Float32, 0, 0);
		(void)red->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, red_Row, nCols, 1, GDT_Float32, 0, 0);
		for (int j = 0; j < nCols; j++)
		{
			if (nir_Row[j] == noData)
			{
				upper_Row[j] = noData;
				lower_Row[j] = noData;
				ndvi_Row[j] = noData;
				cout << "-----------------no data------------"<< endl;
			}
			else
			{
				upper_Row[j] = nir_Row[j] - red_Row[j];
				lower_Row[j] = nir_Row[j] + red_Row[j];
				ndvi_Row[j] = (upper_Row[j] / lower_Row[j]);
			}
			int extended_index = ((ndvi_Row[j] + 1) * 255) / 2 <= 255? ((ndvi_Row[j] + 1) * 255) / 2:255;
			extended_index = extended_index < 0 ? 0: extended_index;
			ndvi_image.at<uchar>(i,j) = extended_index; //G
		}
	}
	cout << "apply color map" << endl;
	applyColorMap( ndvi_image,color_image, COLORMAP_RAINBOW);
	imwrite("ndvi_visualization.tif",color_image);
	// delete buffers
	CPLFree(nir_Row);
	CPLFree(red_Row);
	CPLFree(upper_Row);
	CPLFree(lower_Row);
	CPLFree(ndvi_Row);

	// delete datasets
	GDALClose(red);
	GDALClose(nir);
	GDALDestroyDriverManager();
	cout << "Generation Success" << endl;

	return 0;
}