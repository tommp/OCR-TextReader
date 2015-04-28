#ifndef CANNY_EDGE_DETECTOR_HPP
#define CANNY_EDGE_DETECTOR_HPP

/*Macro definitions*/
/*---------------------------------------------*/
#define cimg_use_jpeg
/*---------------------------------------------*/

/*Included headers*/
/*---------------------------------------------*/
#include "errorlogger.hpp"
#include "CImg.h"
#include "morphology.hpp"
/*---------------------------------------------*/


/*Included dependencies*/
/*---------------------------------------------*/
#include <string>
#include <iostream>
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
	Point(int n_x, int n_y): x(n_x), y(n_y) {}
};

/*Header content*/
/*=============================================*/

void apply_gaussian_smoothing(CImg<unsigned char>& grayimage, double** gaussian, int kernel_size) {

	float gaussian_sum = 0.0;

	for (int x = 0; x < kernel_size; x++) {
		for (int y = 0; y < kernel_size; y++) {
			gaussian_sum += gaussian[x][y];
		} 
	}

	double pixel_sum = 0.0;

	int kernel_offset = kernel_size / 2;

	for (int i = 0; i < grayimage.width(); i++){
		for (int j = 0; j < grayimage.height(); j++){
			pixel_sum = 0.0;
			for (int k = -kernel_offset; k <= kernel_offset; k++){
				for (int l = -kernel_offset; l <= kernel_offset; l++) {
					if (i+k > grayimage.width() || i+k < 0 || j+l > grayimage.height() || j+l < 0) {
						continue;
					}
					else{
						pixel_sum += ((double)grayimage(i + k, j + l) * 
							gaussian[k + kernel_offset][l + kernel_offset]);
					}
				}
			}
			grayimage(i, j) = round(pixel_sum/gaussian_sum);
		}
	}
}

void calculate_gradient_magnitude_and_direction(CImg<unsigned char>& grayimage, 
												CImg<unsigned char>& direction, 
												CImg<unsigned char>& magnitude) {

	CImg<char> dx(grayimage.width(), grayimage.height(), grayimage.depth(), 1);
	CImg<char> dy(grayimage.width(), grayimage.height(), grayimage.depth(), 1);
	double aproxximate_direction = 0.0;
	for (int x = 1; x < grayimage.width()-1; x++){
		for (int y = 1; y < grayimage.height()-1; y++){
			dx(x,y) = (grayimage(x+1, y-1 ) * CImgconsts::GX[0][2] + grayimage(x-1, y-1 ) * CImgconsts::GX[0][0] + 
						grayimage(x+1, y ) * CImgconsts::GX[1][2] + grayimage(x-1, y ) * CImgconsts::GX[1][0] + 
						grayimage(x+1, y+1 ) * CImgconsts::GX[2][2] + grayimage(x-1, y+1 ) * CImgconsts::GX[2][0]);

			dy(x,y) = (grayimage(x-1, y+1 ) * CImgconsts::GY[2][0] + grayimage(x-1, y-1 ) * CImgconsts::GY[0][0] + 
						grayimage(x, y+1 ) * CImgconsts::GY[2][1] + grayimage(x, y-1 ) * CImgconsts::GY[0][1] + 
						grayimage(x+1, y+1 ) * CImgconsts::GY[2][2] + grayimage(x+1, y-1 ) * CImgconsts::GY[0][2]);
			magnitude(x,y) = round(sqrt(((double)dx(x,y)*(double)dx(x,y)) + ((double)dy(x,y)*(double)dy(x,y))));
			
			if (dx(x,y) == 0) {
				if (dy(x,y) == 0) {
					direction(x,y) = 0;
				}
				else{
					direction(x,y) = 20;
				}
			}
			else{
				aproxximate_direction = (atan2((double)dy(x,y), (double)dx(x,y))) * 180.f/CImgconsts::PI;
				if((aproxximate_direction < 23 && aproxximate_direction >= -22) ||
					 (aproxximate_direction >= 158 && aproxximate_direction < -157)) {
					direction(x,y) = 0;
				}
				else if((aproxximate_direction >= 23 && aproxximate_direction < 68) || 
					(aproxximate_direction >= -157 && aproxximate_direction < -112)) {
					direction(x,y) = 10;//45
				}
				else if((aproxximate_direction >= 68 && aproxximate_direction < 113) || 
					(aproxximate_direction >= -112 && aproxximate_direction < -67)) {
					direction(x,y) = 20;//90
				}
				else if((aproxximate_direction >= 113 && aproxximate_direction < 158) || 
					(aproxximate_direction >= -67 && aproxximate_direction < -22)) {
					direction(x,y) = 30;//135
				}
			}
		}
	}
}

void apply_non_maximum_suppress(CImg<unsigned char>& grayimage, 
								CImg<unsigned char>& direction, 
								CImg<unsigned char>& magnitude){

	for (int x = 1; x < grayimage.width()-1; x++){
		for (int y = 1; y < grayimage.height()-1; y++){
			switch(direction(x,y)){
				case 0:
					if(magnitude(x+1,y) > magnitude(x,y) || magnitude(x-1,y) > magnitude(x,y)){
						grayimage(x,y) = 0;
					}
					else{
						grayimage(x,y) = magnitude(x,y);
					}
					break;
				case 10:
					if(magnitude(x+1,y+1) > magnitude(x,y) || magnitude(x-1,y-1) > magnitude(x,y)){
						grayimage(x,y) = 0;
					}
					else{
						grayimage(x,y) = magnitude(x,y);
					}
					break;
				case 20:
					if(magnitude(x,y-1) > magnitude(x,y) || magnitude(x,y+1) > magnitude(x,y)){
						grayimage(x,y) = 0;
					}
					else{
						grayimage(x,y) = magnitude(x,y);
					}
					break;
				case 30:
					if(magnitude(x+1,y-1) > magnitude(x,y) || magnitude(x-1,y+1) > magnitude(x,y)){
						grayimage(x,y) = 0;
					}
					else{
						grayimage(x,y) = magnitude(x,y);
					}
					break;
			}
		}
	}
}

bool check_if_a_neghbour_is_upper_threshold(int xpos, 
											int ypos, 
											CImg<unsigned char>& supressed,
											int high_threshold, 
											int low_threshold,
											std::set<Point>& visited_pixels){
	Point this_pixel = {xpos, ypos};
	visited_pixels.insert(this_pixel);
	for (int i = -1; i <= 1; i++){
		for (int j = -1; j <= 1; j++){
			if(xpos+i < 1 || ypos+j < 1 || xpos+i > supressed.width()-1 || ypos+j > supressed.height()-1){
				continue;
			}
			Point current = {xpos + i, ypos + j};
			if(visited_pixels.find(current) != visited_pixels.end()){
				continue;
			}
			else if(supressed(xpos + i, ypos + j) > high_threshold){
				return true;
			}
			else if (supressed(xpos + i, ypos + j) < low_threshold){
				visited_pixels.insert(current);
				continue;
			}
			else{
				if(check_if_a_neghbour_is_upper_threshold(xpos + i, 
														ypos + j, 
														supressed, 
														high_threshold, 
														low_threshold, 
														visited_pixels)){
					return true;
				}
			}
		}
	}
	return false;
}

void perform_hysteresis(CImg<unsigned char>& edges, CImg<unsigned char>& supressed, int high_threshold){
	int low_threshold = high_threshold*CImgconsts::LOW_THRESHOLD_SCALE;
	for (int x = 0; x < supressed.width(); x++){
		for (int y = 0; y < supressed.height(); y++){
			if ((x == 0) || (y == 0) || (x == supressed.width()-1) || (y == supressed.height()-1)){
				edges(x,y) = 0;
			}
			else if ((x == 1) || (y == 1) || (x == supressed.width()-2) || (y == supressed.height()-2)){
				edges(x,y) = 0;
			}
			else if (supressed(x,y) > high_threshold){
				edges(x,y) = 255;
			}
			else if (supressed(x,y) < low_threshold) {
				edges(x,y) = 0;
			}
			else{
				std::set<Point> visited_pixels;
				if(check_if_a_neghbour_is_upper_threshold(x, 
														y, 
														supressed, 
														high_threshold, 
														low_threshold, 
														visited_pixels)){
					edges(x,y) = 255;
				}
				else{
					edges(x,y) = 0;
				}
			}
		}
	}
}

int return_otsu_threshold(const CImg<unsigned char>& grayscale){
	int histogram[CImgconsts::levels] = {0};
	for (int x = 1; x < grayscale.width(); x++){
		for (int y = 1; y < grayscale.height(); y++){
			histogram[grayscale(x,y)]++;
		}
	}

	double Wb_counter = 0.f;
	double Wf_counter = 0.f;
	double mb_counter = 0.f;
	double mf_counter = 0.f;
	double number_of_pixels = grayscale.width()*grayscale.height();
	double current_bcw = 0;
	double max_bcw = -1;
	int threshold = -1;

	for (int i = 0; i < CImgconsts::levels; i++){
		Wb_counter = 0.f;
		Wf_counter = 0.f;
		mb_counter = 0.f;
		mf_counter = 0.f;
		for (int j = 0; j < i; j++){
			Wb_counter += histogram[j];
			mb_counter += histogram[j]*j;
		}
		for (int j = i; j < CImgconsts::levels; j++){
			Wf_counter += histogram[j];
			mf_counter += histogram[j]*j;
		}

		if(!(Wf_counter >= 1.f) || !(Wb_counter >= 1.f)){
			continue;
		}
		else{
			current_bcw = ((Wb_counter/number_of_pixels)*(Wf_counter/number_of_pixels)*
									(mf_counter/Wf_counter - mb_counter/Wb_counter)*
									(mf_counter/Wf_counter - mb_counter/Wb_counter));
			if(max_bcw < current_bcw){
				max_bcw = current_bcw;
				threshold = i;
			}
		}
	}
	if(threshold == -1){
		std::cout << " ---Otsu thresholding failed, setting to: " << CImgconsts::levels/2 <<
		 ", by default." << std::endl;
		return CImgconsts::levels/2;
	}
	else{
		return threshold;
	}
}

void localized_thresholding(CImg<unsigned char>& grayscale, int grid_width, int grid_height) {
	int stepsize_w = grayscale.width()/grid_width;
	int stepsize_h = grayscale.height()/grid_height;
	for (int x = 0; x < grid_width; x++) {
		for (int y = 0; y < grid_height; y++) {
			CImg<unsigned char> piece = grayscale.get_crop(x*stepsize_w, 
				y*stepsize_h, 
				(x+1)*stepsize_w, (y+1)*stepsize_h);
			convert_to_binary(piece, return_otsu_threshold(piece));
			for (int piece_x = 0; piece_x < piece.width(); piece_x++) {
				for (int piece_y = 0; piece_y < piece.height(); piece_y++) {
					grayscale(x*stepsize_w + piece_x, y*stepsize_h + piece_y) = piece(piece_x, piece_y);				
				}
			}
		}
	}
}

void localized_pooled_thresholding(CImg<unsigned char>& grayscale, int pixel_square, int init_threshold) {
	int grid_width = grayscale.width()/pixel_square;
	int grid_height = grayscale.height()/pixel_square;
	if (grid_width == 0 && grid_height == 0) {
		convert_to_binary(grayscale, return_otsu_threshold(grayscale));
	}
	else{
		CImg<unsigned char> piece;
		int threshold = init_threshold;
		for (int x = 0; x < grid_width; x++) {
			for (int y = 0; y < grid_height; y++) {

				if ((x == grid_width - 1) && (y == grid_height - 1)) {
					piece = grayscale.get_crop(x*pixel_square, 
												y*pixel_square, 
												grayscale.width(), 
												grayscale.height());
				}
				else if (y == (grid_height - 1)) {
					piece = grayscale.get_crop(x*pixel_square, 
												y*pixel_square, 
												(x+1)*pixel_square, 
												grayscale.height()-1);//TODO:: FIX THIS; SAME AS IN CROPPER!
				}
				else if (x == (grid_width - 1)) {
					piece = grayscale.get_crop(x*pixel_square, 
												y*pixel_square, 
												grayscale.width(), 
												(y+1)*pixel_square);
				}
				else {
					piece = grayscale.get_crop(x*pixel_square, 
												y*pixel_square, 
												(x+1)*pixel_square, 
												(y+1)*pixel_square);
				}
				if (threshold == -1) {
					threshold = return_otsu_threshold(piece);
				}
				else {
					threshold += return_otsu_threshold(piece);
					threshold /= 2;
				}
				convert_to_binary(piece, threshold);
				for (int piece_x = 0; piece_x < piece.width(); piece_x++) {
					for (int piece_y = 0; piece_y < piece.height(); piece_y++) {
						grayscale(x*pixel_square + piece_x, y*pixel_square + piece_y) = piece(piece_x, piece_y);				
					}
				}
			}
		}
	}
}


/* Consider replacing with vector or dynarray (std) or put it in a class with its respective variables */
double** return_gaussian_kernel(int kernel_size, double sigma){
	double** kernel = new double*[kernel_size];
	for (int i = 0; i < kernel_size; i++){
		kernel[i] = new double[kernel_size];
	}

	double mean = kernel_size/2;
	double kernel_sum = 0.0;
	for (int x = 0; x < kernel_size; x++) {
		for (int y = 0; y < kernel_size; y++) {
			kernel[x][y] = exp( -0.5 * (pow((x-mean)/sigma, 2.0) + pow((y-mean)/sigma,2.0)) )
							 / (2 * CImgconsts::PI * sigma * sigma);
			kernel_sum += kernel[x][y];
		}
	}


	for (int x = 0; x < kernel_size; ++x){ 
		for (int y = 0; y < kernel_size; ++y){
			kernel[x][y] /= kernel_sum;
		}
	}
	return kernel;
}

void delete_gaussian_kernel(double** kernel, int kernel_size) {
	for (int i = 0; i < kernel_size; i++){
		delete[] kernel[i];
	}
	delete[] kernel;
}
/*------------------------------------------------------------------------*/

void run_canny_edge_detection(CImg<unsigned char>& image, 
								CImg<unsigned char>& edges, 
								int gaussian_size, 
								double sigma,
								double high_threshold_scale){
	CImg<unsigned char> grayscale(image.width(), image.height(), image.depth(), 1);
	CImg<unsigned char> direction(image.width(), image.height(), image.depth(), 1);
	CImg<unsigned char> magnitude(image.width(), image.height(), image.depth(), 1);

	convert_to_greyscale(image, grayscale);

	double** kernel = return_gaussian_kernel(gaussian_size, sigma);

	apply_gaussian_smoothing(grayscale, kernel, gaussian_size);

	delete_gaussian_kernel(kernel, gaussian_size);

	calculate_gradient_magnitude_and_direction(grayscale, direction, magnitude);

	apply_non_maximum_suppress(grayscale, direction, magnitude);

	perform_hysteresis(edges, grayscale, round(high_threshold_scale*return_otsu_threshold(grayscale)));
}

/*=============================================*/
#endif