#ifndef CANNY_EDGE_DETECTOR_HPP
#define CANNY_EDGE_DETECTOR_HPP

/*Macro definitions*/
/*---------------------------------------------*/
#define cimg_use_jpeg
/*---------------------------------------------*/

/*---------------------------------------------*/
#include "errorlogger.hpp"
#include "CImg.h"
/*---------------------------------------------*/


/*Included dependencies*/
/*---------------------------------------------*/
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <fstream>
#include <unistd.h>
#include <set>
/*---------------------------------------------*/
//LUMINOSITY METHOD: (0.21 R + 0.71 G + 0.07 B)
using namespace cimg_library;

namespace CImgconsts {
	const int levels = 255;
	const float HIGH_THRESHOLD_SCALE = 1.f;
	const float LOW_THRESHOLD_SCALE = HIGH_THRESHOLD_SCALE*0.30;
	const float RED_SCALE = 0.21;
	const float GREEN_SCALE = 0.71;
	const float BLUE_SCALE = 0.07;
	const int GAUSSIAN_SIZE = 5;
	const double GAUSSIAN_SIGMA = 1.5;

	const double PI = 3.14159265358979323846;

	/* Convolution mask (x direction) for gradient calculation (sobel operator) */
	const char GX[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};

	/* Convolution mask (y direction) for gradient calculation (sobel operator) */
	const char GY[3][3] = {
		{-1, -2, -1},
		{0, 0, 0},
		{1, 2, 1}
	};
}
	
class Point{
public:
	int x;
	int y;
	bool operator <(const Point& p) const {return this->x < p.x;}
};

/*Header content*/
/*=============================================*/

void convert_to_greyscale(CImg<unsigned char>& image, CImg<unsigned char>& gray);

void apply_gaussian_smoothing(CImg<unsigned char>& grayimage, double** gaussian, int kernel_size);

void calculate_gradient_magnitude_and_direction(CImg<unsigned char>& grayimage, 
												CImg<unsigned char>& direction, 
												CImg<double>& magnitude);

void apply_non_maximum_suppress(CImg<unsigned char>& grayimage, 
								CImg<unsigned char>& direction, 
								CImg<double>& magnitude);

bool check_if_a_neghbour_is_upper_threshold(int xpos, 
											int ypos, 
											CImg<unsigned char>& supressed,
											int high_threshold, 
											int low_threshold,
											std::set<Point>& visited_pixels);

void perform_hysteresis(CImg<unsigned char>& edges, CImg<unsigned char>& supressed, int high_threshold);

int return_otsu_threshold(const CImg<unsigned char>& grayscale);

void delete_gaussian_kernel(double** kernel, int kernel_size);

double** return_gaussian_kernel(int kernel_size, double sigma);

void run_canny_edge_detection(CImg<unsigned char>& image, 
								CImg<unsigned char>& edges, 
								int gaussian_size, 
								double sigma,
								double high_threshold_scale);

/*=============================================*/
#endif