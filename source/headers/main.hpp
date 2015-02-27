
#ifndef MAIN_HPP
#define MAIN_HPP

/*Macro definitions*/
/*---------------------------------------------*/
#define cimg_use_jpeg
/*---------------------------------------------*/

/*Included headers*/
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
/*---------------------------------------------*/

using namespace cimg_library;

namespace SDLconsts {
	const unsigned int window_length = 500;
	const unsigned int window_height = 500;
}

namespace CImgconsts {
	const float RED_SCALE = 0.21;
	const float GREEN_SCALE = 0.71;
	const float BLUE_SCALE = 0.07;
	const int GAUSSIAN_SIZE = 5;

	/* Discrete approximation of a 2D gaussian function, stolen from wikipedia, standard deviation is  0.84089642 */
	/* This is not neccessarily optimal, room for improvement here */
	const float GAUSSIAN[5][5] = {
		{0.00078634, 0.00655965, 0.01330373, 0.00655965, 0.00078633},
		{0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965},
		{0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373},
		{0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965},
		{0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633}
	};

	/* Convolution mask (x direction) for gradient calculation (sobel operator) */
	const int GX[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};

	/* Convolution mask (y direction) for gradient calculation (sobel operator) */
	const int GY[3][3] = {
		{-1, -2, -1},
		{0, 0, 0},
		{1, 2, 1}
	};
	const int GAUSSIAN_OFFSET_FROM_CENTER = 2;
}
	





/*Header content*/
/*=============================================*/
//LUMINOSITY METHOD: (0.21 R + 0.71 G + 0.07 B)
void convert_to_greyscale(CImg<unsigned char>& image, CImg<unsigned char>& gray);
void apply_gaussian_smoothing(CImg<unsigned char>& grayimage);
void calculate_gradient_magnitude_and_direction(CImg<unsigned char>& grayimage, 
												CImg<unsigned char>& direction, 
												CImg<double>& magnitude);
void apply_non_maximum_suppress(CImg<unsigned char>& grayimage, 
								CImg<unsigned char>& direction, 
								CImg<double>& magnitude);

void pollevent(bool& var);

void waitForEvent();
/*=============================================*/

#endif