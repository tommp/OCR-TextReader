
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
	const float RED_SCALE = 0.2989;
	const float GREEN_SCALE = 0.5870;
	const float BLUE_SCALE = 0.1140;
	const int GAUSSIAN_SIZE = 5;

	/* Discrete approximation of a 2D gaussian function, stolen from wikipedia, standard deviation is  0.84089642 */
	/* This is not neccessarily optimal, room for improvement here */
	const float GAUSSIAN[5][5] = {
		{0.017391304, 0.034782608, 0.04347826, 0.034782608, 0.017391304},
		{0.034782608, 0.078260869, 0.104347826, 0.078260869, 0.034782608},
		{0.04347826, 0.104347826, 0.130434782, 0.104347826, 0.04347826},
		{0.034782608, 0.078260869, 0.104347826, 0.078260869, 0.034782608},
		{0.017391304, 0.034782608, 0.04347826, 0.034782608, 0.017391304}
	};

	/* Convolution mask (x direction) for gradient calculation (sobel operator) */
	const double GX[3][3] = {
		{-1.0, 0.0, 1.0},
		{-2.0, 0.0, 2.0},
		{-1.0, 0.0, 1.0}
	};

	/* Convolution mask (y direction) for gradient calculation (sobel operator) */
	const double GY[3][3] = {
		{-1.0, -2.0, -1.0},
		{0.0, 0.0, 0.0},
		{1.0, 2.0, 1.0}
	};
	const int GAUSSIAN_OFFSET_FROM_CENTER = 2;
}
	





/*Header content*/
/*=============================================*/
//LUMINOSITY METHOD: (0.21 R + 0.71 G + 0.07 B)
void convert_to_greyscale(CImg<double>& image, CImg<double>& gray);
void apply_gaussian_smoothing(CImg<double>& grayimage);
void calculate_gradient_magnitude_and_direction(CImg<double>& grayimage, 
												CImg<unsigned char>& direction, 
												CImg<double>& magnitude);
void apply_non_maximum_suppress(CImg<double>& grayimage, 
								CImg<unsigned char>& direction, 
								CImg<double>& magnitude);

void pollevent(bool& var);

void waitForEvent();
/*=============================================*/

#endif