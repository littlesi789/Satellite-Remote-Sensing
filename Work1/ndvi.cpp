#include <iostream>
#include <fstream>
#include <gdal/gdal_priv.h>
#include <opencv/cv.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

// Very small values ​​(0.1 or less) of the NDVI function correspond to empty 
// areas of rocks, sand or snow. Moderate values ​​(from 0.2 to 0.3) represent 
// shrubs and meadows, while large values ​​(from 0.6 to 0.8) indicate temperate 
// and tropical forests.

int main() {
    GDALAllRegister();

	const char* nirFilePath = "arizona/banda4.tif";
	const char* redFilePath = "arizona/banda3.tif";
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
	Mat ndvi_image=Mat::zeros(nRows,nCols,CV_8UC1);
    
	
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


	Mat color_image, smooth_1, smooth_2, mask, step, detected_edges, detected_edges_step;
	imwrite("ndvi_visualization.tif",ndvi_image);
	cout << "apply color map" << endl;
	medianBlur(ndvi_image, smooth_1, 5); //Blurs an image using the median filter. Remove noise.
	medianBlur(smooth_1, smooth_2, 9);
	applyColorMap(smooth_1, color_image, COLORMAP_RAINBOW);
	imwrite("ndvi_visualization_denoise_1.tif",color_image);
	applyColorMap(smooth_2, color_image, COLORMAP_RAINBOW);
	imwrite("ndvi_visualization_denoise_2.tif",color_image);
	
	// Mask different levels of NDVI and perform edge detection.
	int lowThreshold = 0;
	const int ratio = 3;
	const int kernel_size = 3;

	inRange(smooth_2, 153, 256, mask);
	step = Mat::zeros(mask.size(), CV_8UC1);
	step.setTo(100, mask == 255);
	cout << "Write image: edge w/ 0.1 NDVI" << endl;
	Canny(step, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
	imwrite("ndvi_visualization_edge_100.tif",detected_edges);

	inRange(smooth_2, 178, 256, mask);
	step.setTo(180, mask == 255);
	inRange(smooth_2, 204, 256, mask);
	step.setTo(255, mask == 255);
	cout << "Write image: step" << endl;
	imwrite("ndvi_visualization_step.tif",step);

	
	Canny(step, detected_edges_step, lowThreshold, lowThreshold*ratio, kernel_size );
	cout << "Write image: edge step" << endl;
	imwrite("ndvi_visualization_edge_step.tif",detected_edges_step);

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