
#ifndef LETTER_SEGMENTATION_HPP
#define LETTER_SEGMENTATION_HPP


/*Included headers*/
/*---------------------------------------------*/
#include "CImg.h"
/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
/*---------------------------------------------*/

/*Header content*/
/*=============================================*/
using namespace cimg_library;

void create_grid_separation(CImg<unsigned char>& binary_img, 
							std::vector<int>& vertical_line_indexes,
							unsigned char grid_value, 
							unsigned char plus_value, 
							int border) {

	int width = binary_img.width();
	int height = binary_img.height();
	bool prev_was_gridline = false;

	vertical_line_indexes.clear();

	for (int y = border; y < height - border; y++) {
		for (int x = border; x < width; x++) {
			if (binary_img(x,y) == plus_value) {
				prev_was_gridline = false;
				break;
			}
			else if (x == width-1 && !prev_was_gridline) {
				prev_was_gridline = true;
				vertical_line_indexes.push_back(y);
				for (int x_draw = border; x_draw < width - border; x_draw++) {
					binary_img(x_draw,y) = grid_value;
				}
			}
		}
	}
	for (std::vector<int>::iterator it = vertical_line_indexes.begin(); it != vertical_line_indexes.end(); ++it) {
		for (int x = border; x < width - border; x++) {
			for (int y = *it+1; y < height - border; y++) {
				if (binary_img(x,y) == plus_value) {
					break;
				}
				else if (binary_img(x,y) == grid_value) {
					for (int y_draw = *it; y_draw < y; y_draw++) {
						binary_img(x,y_draw) = grid_value;
					}
					break;
				}
			}
		}
	}
}

void remove_single_artifacts(CImg<unsigned char>& binary_img, unsigned char negative_value) {
	int has_neighbour = false;
	for (int x = 1; x < binary_img.width()-1; x++) {
		for (int y = 1; y < binary_img.height()-1; y++) {

			if (binary_img(x,y-1) == negative_value && 
				binary_img(x-1,y-1) == negative_value && 
				binary_img(x-1,y) == negative_value &&
				binary_img(x-1,y+1) == negative_value && 
				binary_img(x,y+1) == negative_value &&
				binary_img(x+1,y+1) == negative_value &&
				binary_img(x+1,y) == negative_value &&
				binary_img(x+1,y-1) == negative_value) {

				has_neighbour = false;
			}
			else{
				has_neighbour = true;
			}

			if(!has_neighbour) {
				binary_img(x,y) = negative_value;
			}
		}
	}
}

void crop_empty_space(CImg<unsigned char>& binary_img, unsigned char plus_value, int border) {
	int width = binary_img.width();
	int height = binary_img.height();

	int x_left_crop = 0;
	int y_top_crop = 0;
	int x_right_crop = width;
	int y_bottom_crop = height;


	for (int x = border; x < width - border; x++) {
		for (int y = border; y < height - border; y++) {
			if (binary_img(x,y) == plus_value) {
				x_left_crop = x;
				goto x_right;
			}
		}
	}

	x_right:
	for (int x = width - border; x > border; x--) {
		for ( int y = border; y < height - border; y++) {
			if (binary_img(x,y) == plus_value) {
				x_right_crop = x;
				goto y_top;
			}
		}
	}

	y_top:

	for (int y = border; y < height - border; y++) {
		for ( int x = border; x < width - border; x++) {
			if (binary_img(x,y) == plus_value) {
				y_top_crop = y;
				goto y_bottom;
			}
		}
	}

	y_bottom:

	//TODO::BUGGED HERE; FIND OUT WHY -1 IS NECESSARY, CANT FIND ANY ARTIFACTS!!!!!!!
	for (int y = height - border-1; y > border-1; y--) {
		for ( int x = border; x < width - border; x++) {
			if (binary_img(x,y) == plus_value) {
				y_bottom_crop = y;
				goto end;
			}
		}
	}
	end:
	binary_img.crop(x_right_crop, y_top_crop, x_left_crop, y_bottom_crop);
}

void segment_letters(CImg<unsigned char>& gridded_img, 
						std::vector<int>& vertical_line_indexes, 
						unsigned char grid_value,
						Network& nnet,
						const std::vector<unsigned int>& topology) {
	int y_top;
	int y_bottom;
	int x_left;
	int x_right;
	int width = gridded_img.width();

	for (std::vector<int>::iterator it = vertical_line_indexes.begin(); it != vertical_line_indexes.end(); it++) {
		y_top = *it;
		if( it+1 == vertical_line_indexes.end()) {
			y_bottom = gridded_img.height();
		} 
		else{
			y_bottom = *(it+1);
		}

		x_left = -1;
		x_right = -1;

		for (int x = 0; x < width; x++) {
			if (gridded_img(x, (*it)+4) == grid_value) {
				if(x_left == -1) {
					while((gridded_img(x, (*it)+4) == grid_value) && x < width) {
						x++;
					}
					x_left = x;
				}
				else {
					x_right = x-1;
					/* GET CROPPED IMAGE, CURRENTLY TESTING; MAKE PRETTY LATER*/
					std::ostringstream sstream;
					sstream << "../data/letters/"<< x_left << "x" << y_top << ".jpg";
					std::string filename = sstream.str();

					CImg<unsigned char> letter = gridded_img.get_crop(x_left, y_top+1, x_right, y_bottom-1);
					crop_empty_space(letter, 255, 0);
					
					
					std::vector<float> network_input;
					int letter_size = letter.width() * letter.height();
					
					for(int letr_x = 0; letr_x < letter.width(); letr_x++) {
						for(int letr_y = 0; letr_y < letter.height(); letr_y++) {
							if (letter(letr_x,letr_y) == 255) {
								network_input.push_back(1.0);
							}
							else {
								network_input.push_back(0.0);
							}
						}
					}
					for (int neuron_number = 0; neuron_number < topology[0]-letter_size; neuron_number++) {
						network_input.push_back(0.0);
					}
					

					nnet.feed_forward(network_input);
					
					
					letter.save(filename.c_str());

					x_left = -1;
					x_right = -1;
				}
			}
		}
	}
}

void generate_training_data(CImg<unsigned char>& gridded_img,
							std::vector<int>& vertical_line_indexes, 
							unsigned char grid_value,
							const std::vector<int>& output_neuron_indexes,
							const std::vector<unsigned int>& topology) {
	std::ofstream training_data;
	training_data.open ("trdata.txt");

	int y_top;
	int y_bottom;
	int x_left;
	int x_right;
	int width = gridded_img.width();
	int current_row = 0;

	for (std::vector<int>::iterator it = vertical_line_indexes.begin(); it != vertical_line_indexes.end(); it++) {
		y_top = *it;
		if( it+1 == vertical_line_indexes.end()) {
			y_bottom = gridded_img.height();
		} 
		else{
			y_bottom = *(it+1);
		}

		x_left = -1;
		x_right = -1;

		for (int x = 0; x < width; x++) {
			if (gridded_img(x, (*it)+4) == grid_value) {
				if(x_left == -1) {
					while((gridded_img(x, (*it)+4) == grid_value) && x < width) {
						x++;
					}
					x_left = x;
				}
				else {
					x_right = x-1;

					CImg<unsigned char> letter = gridded_img.get_crop(x_left, y_top+1, x_right, y_bottom-1);
					crop_empty_space(letter, 255, 0);
					
					std::vector<float> network_input;
					int letter_size = letter.width() * letter.height();
					
					for(int letr_x = 0; letr_x < letter.width(); letr_x++) {
						for(int letr_y = 0; letr_y < letter.height(); letr_y++) {
							if (letter(letr_x,letr_y) == 255) {
								training_data << 1.0;
							}
							else {
								training_data << 0.0;
							}
						}
					}
					for (int neuron_number = 0; neuron_number < topology[0]-letter_size; neuron_number++) {
						training_data << 0.0;
					}
					
					training_data << "\n";

					for (int i = 0; i < topology.back(); i++) {
						if (i == output_neuron_indexes[current_row]) {
							training_data << 1.0;
						}
						else {
							training_data << 0.0;
						}
					}

					training_data << "\n";

					x_left = -1;
					x_right = -1;
				}
			}
		}
		current_row++;
	}
}


/*=============================================*/

#endif