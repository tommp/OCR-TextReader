
#ifndef MORPHOLOGY_HPP
#define MORPHOLOGY_HPP

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
#include <iostream>
#include <time.h>
#include <algorithm>
#include <assert.h>
#include <limits>
#include <fstream>
/*---------------------------------------------*/

namespace morphology {
	const float RED_SCALE = 0.21;
	const float GREEN_SCALE = 0.71;
	const float BLUE_SCALE = 0.07;
}
using namespace cimg_library;
class STRMask;

/*Header content*/
/*=============================================*/

class STRMask {
private:
	std::vector<std::vector<char>> m_mask;
	int m_size_x;
	int m_size_y;
	int m_center_x;
	int m_center_y;
public:
	STRMask(std::string infile);
	const char& operator()(const int x_pos, const int y_pos)const;
	int get_sx()const {return m_size_x;}
	int get_sy()const {return m_size_y;}
	int get_cx()const {return m_center_x;}
	int get_cy()const {return m_center_y;}
};

STRMask::STRMask(std::string infile) {
	std::ifstream mask(infile);
	if (! mask.is_open()) {
		std::cout << "Failed to open file: " << infile << ", mask not initialized!\n" << std::endl;  
	}
	else {
		int size_x;
		int size_y;

		mask >> size_x >> size_y;
		assert(size_x > 0 && size_y > 0);

		m_size_x = size_x;
		m_size_y = size_y;
		m_center_x = size_x/2;
		m_center_y = size_y/2;

		m_mask.resize(size_x);
		for (int i = 0; i < size_x; i++) {
			m_mask[i].resize(size_y);
		}
		char temp = 0;
		for (auto& x : m_mask) {
			for (unsigned int y = 0; y < x.size(); y++) {
				mask >> temp;
				x[y] = temp;
			}
		}
	}	
}

const char& STRMask::operator()(const int x_pos, const int y_pos)const{
	assert(x_pos >= 0 && x_pos < m_size_x);
	assert(y_pos >= 0 && y_pos < m_size_y);

	return m_mask[x_pos][y_pos];
}

void dialate_image(CImg<unsigned char>& target_image, const STRMask& mask){
	CImg<unsigned char> image(target_image);

	unsigned char current_val = 0;

	for (int x = 0; x < image.width(); x++) {
		for (int y = 0; y < image.height(); y++) {
			for (int mx = -mask.get_cx(); mx < mask.get_sx() - mask.get_cx(); mx++) {
				for (int my = -mask.get_cy(); my < mask.get_sy() - mask.get_cy(); my++) {
					int curr_offset_x = x + mx;
					int curr_offset_y = y + my;

					if(curr_offset_y < 0 || curr_offset_x < 0) {
						continue;
					}
					else if (curr_offset_x > image.width() || curr_offset_y > image.height()) {
						continue;
					}
					else if (mask(mx+mask.get_cx(), my+mask.get_cy()) && 
						image(curr_offset_x, curr_offset_y) > current_val){
						current_val = image(curr_offset_x, curr_offset_y);
					}
				}
			}

			target_image(x, y) = current_val;
			current_val = 0;
		}
	}
}

void erode_image(CImg<unsigned char>& target_image, const STRMask& mask){ 
	CImg<unsigned char> image(target_image);

	unsigned char current_val = std::numeric_limits<unsigned char>::max();

	for (int x = 0; x < image.width(); x++) {
		for (int y = 0; y < image.height(); y++) {
			for (int mx = -mask.get_cx(); mx < mask.get_sx() - mask.get_cx(); mx++) {
				for (int my = -mask.get_cy(); my < mask.get_sy() - mask.get_cy(); my++) {
					int curr_offset_x = x + mx;
					int curr_offset_y = y + my;

					if(curr_offset_y < 0 || curr_offset_x < 0) {
						continue;
					}
					else if (curr_offset_x > image.width() || curr_offset_y > image.height()) {
						continue;
					}
					else if (mask(mx+mask.get_cx(), my+mask.get_cy()) && 
						image(curr_offset_x, curr_offset_y) < current_val){
						current_val = image(curr_offset_x, curr_offset_y);
					}
				}
			}

			target_image(x, y) = current_val;
			current_val = std::numeric_limits<unsigned char>::max();
		}
	}
}

void close_image(CImg<unsigned char>& target_image, const STRMask& mask) {
	dialate_image(target_image, mask);
	erode_image(target_image, mask);
}

void open_image(CImg<unsigned char>& target_image, const STRMask& mask) {
	erode_image(target_image, mask);
	dialate_image(target_image, mask);
}

void bottom_hat_trans(CImg<unsigned char>& target_image, const STRMask& mask){
	CImg<unsigned char> image(target_image);
	close_image(target_image, mask);

	for (int x = 0; x < target_image.width(); x++) {
		for (int y = 0; y < target_image.height(); y++) {
			target_image(x,y) -= image(x,y);
		} 
	}
}

void top_hat_trans(CImg<unsigned char>& target_image, const STRMask& mask){
	CImg<unsigned char> image(target_image);
	open_image(image, mask);
	for (int x = 0; x < target_image.width(); x++) {
		for (int y = 0; y < target_image.height(); y++) {
			target_image(x,y) -= image(x,y);
		} 
	}
}

void convert_to_binary(CImg<unsigned char>& target_image, int threshold) {
	for (int x = 0; x < target_image.width(); x++) {
		for (int y = 0; y < target_image.height(); y++) {
			if (target_image(x,y) < threshold) {
				target_image(x,y) = 0;
			}
			else{
				target_image(x,y) = 255;
			}
		} 
	} 
}

void rescale_image(CImg<unsigned char>& image) {
	int scale = 0;
	if(image.width() > image.height()){
		scale -= std::min((int)round(1800.0/(float)image.width()*100.0), 100);
	}
	else {
		scale -= std::min((int)round(1800.0/(float)image.height()*100.0), 100);
	}
	if (scale > -45) {
		scale = -45;
	}
	std::cout<<"Scaling set to: "<< abs(scale) <<" %" <<std::endl;

	// nearest-neighbor interpolation for speed, switch last option to 5 for lossless
	image.resize(scale, scale, -100, -100, 1);
}

void convert_to_greyscale(CImg<unsigned char>& image, CImg<unsigned char>& gray){
	double red, green, blue;
	for (int x = 0; x < image.width(); x++){
		for (int y = 0; y < image.height(); y++){
			red = image(x,y,0, 0);
			green = image(x,y,0, 1);
			blue = image(x,y,0, 2);
			gray(x,y) = round((red * morphology::RED_SCALE + 
				green * morphology::GREEN_SCALE + 
				blue * morphology::BLUE_SCALE)/3);
		}
	}
}

/*=============================================*/

#endif