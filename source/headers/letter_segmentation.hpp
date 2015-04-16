
#ifndef LETTER_SEGMENTATION_HPP
#define LETTER_SEGMENTATION_HPP


/*Included headers*/
/*---------------------------------------------*/
#include "CImg.h"
#include "morphology.hpp"
#include "canny_edge_detector.hpp"
/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glob.h>
#include <vector>
#include <map>
/*---------------------------------------------*/
namespace seg_consts {
	const int num_inputs = 100;
	const std::string folder_path = "../data/SD19/HSF_0/*";
}

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
	vertical_line_indexes.push_back(0);

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
				else if (binary_img(x,y) == grid_value || y == (height - border - 1)) {
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


void read_letters(CImg<unsigned char>& gridded_img, 
						std::vector<int>& vertical_line_indexes, 
						unsigned char grid_value,
						Net& nnet,
						const std::vector<unsigned int>& topology,
						std::string out_file) {
	int y_top;
	int y_bottom;
	int x_left;
	int x_right;
	int width = gridded_img.width();
	std::ofstream results (out_file);

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
			if ((gridded_img(x, (*it)+4) == grid_value) || 
				(x == 0 && gridded_img(x, (*it)+4) != grid_value) ||
				(x == width-1)) {
				if(x_left == -1) {
					while((gridded_img(x, (*it)+4) == grid_value) && x < width) {
						x++;
					}
					x_left = x;
				}
				else {
					x_right = x-1;

					int letter_dimension = sqrt(topology[0]);

					if (topology[0] % letter_dimension) {
						std::cout << "Topology dimension of: " << topology[0] << 
						" is invalid, must have a whole square root (ex. 100)\n" 
						<< std::endl;
						exit(0);
					}

					CImg<unsigned char> letter = gridded_img.get_crop(x_left, y_top+1, x_right, y_bottom-1);
					crop_empty_space(letter, 255, 0);
					letter.resize(letter_dimension, letter_dimension,-100,-100,5);
					for (int i = 0; i < letter.width(); i++) {
						for (int j = 0; j < letter.height(); j++) {
							if (letter(i,j)) {
								letter(i,j) = 255;
							}
						}
					}
					std::vector<double> network_input;
					
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

					std::vector<double> result_values;

					nnet.feedForward(network_input);
					nnet.getResults(result_values);
	
					for (auto& output : result_values) {
						results << output << ' ';
						
					}
					results << '\n';

					x_left = -1;
					x_right = -1;
				}
			}
		}
	}
	results.close();
}

void segment_letters(CImg<unsigned char>& gridded_img, 
						std::vector<int>& vertical_line_indexes, 
						unsigned char grid_value) {
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
			if ((gridded_img(x, (*it)+4) == grid_value) || 
				(x == 0 && gridded_img(x, (*it)+4) != grid_value) ||
				(x == width-1)) {
				if(x_left == -1) {
					while((gridded_img(x, (*it)+4) == grid_value) && x < width) {
						x++;
					}
					x_left = x;
				}
				else {
					x_right = x-1;
					std::ostringstream sstream;
					sstream << "../data/letters/"<< y_top << "x" << x_left << ".JPG";
					std::string filename = sstream.str();

					CImg<unsigned char> letter = gridded_img.get_crop(x_left, y_top+1, x_right, y_bottom-1);
					crop_empty_space(letter, 255, 0);
					letter.resize(seg_consts::num_inputs, seg_consts::num_inputs,-100,-100,5);
					for (int i = 0; i < letter.width(); i++) {
						for (int j = 0; j < letter.height(); j++) {
							if (letter(i,j)) {
								letter(i,j) = 255;
							}
						}
					}
					
					letter.save(filename.c_str());

					x_left = -1;
					x_right = -1;
				}
			}
		}
	}
}

void train_network(std::string data_filename, Net& nnet) {
	std::ifstream data (data_filename);
	std::string input_line;
	std::string output_line;
	std::vector<double> target_values;
	std::vector<double> input_values;
	if(data.is_open()) {
		while(std::getline(data, input_line)) {
			std::getline(data, output_line);
			//TODO::DO NOT CONVERT HERE, JUST USE STRING COMPARISON!!!!!!!!!!!!!!!!!!!!!!!!!1
			for (auto& it : input_line) {
				if (!((it) == '\n')){
					int value = ((int)it -'0');
					if (value == 1) {
						input_values.push_back(0.5);
					}
					else {
						input_values.push_back(-0.5);
					}
				}
			}

			for (auto& it : output_line) {
				if (!((it) == '\n')){
					int value = ((int)it -'0');
					if (value == 1) {
						target_values.push_back(0.5);
					}
					else {
						target_values.push_back(-0.5);
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////
			nnet.feedForward(input_values);
			nnet.backProp(target_values);
			input_values.clear();
			target_values.clear();
		}
	}
	else {
		std::cout <<"Failed to open file: " << data_filename <<std::endl;
	}
	data.close();
}

std::vector<std::string> glob(const std::string& pat){
    glob_t glob_result;
    glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);
    std::vector<std::string> ret;
    for(unsigned int i=0; i<glob_result.gl_pathc; i++){
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}

std::vector<std::string> split_string(std::string text, std::string delimiter) {
	size_t pos = 0;
	std::vector<std::string> tokens;
	std::string token;
	while ((pos = text.find(delimiter)) != std::string::npos) {
	    token = text.substr(0, pos);
	    tokens.push_back(token);
	    text.erase(0, pos + delimiter.length());
	}
	return tokens;
}

void load_symbolmap(std::map<char, std::string>& symbolmap, const std::string& filename){
	std::ifstream symbols(filename);
	std::string line;

	while (getline(symbols, line)){
		symbolmap[line[0]] = line.substr (1);
	}
}

void generate_training_data_SD19(const std::vector<unsigned int>& topology,
							std::string filename,
							std::map<char,std::string>& symbolmap) {

	int letter_dimension = sqrt(topology[0]);

	std::ofstream training_data(filename);

	if (topology[0] % letter_dimension) {
		std::cout << "Topology dimension of: " << topology[0] << 
		" is invalid, must have a whole square root (ex. 100)\n" << std::endl;
		exit(0);
	}

	std::vector<std::string> folders = glob(seg_consts::folder_path);

	for (auto& folder : folders) {
		std::cout << "-- Generating data from folder: " << folder << std::endl;
		std::stringstream builder;
		builder << folder << "/HSF_0_*";
		const std::string image_path = builder.str();
		std::vector<std::string> images = glob(image_path);
		int filenr = 0;
		for (auto& file : images) {
			CImg<unsigned char> image(file.c_str());
			CImg<unsigned char> letter(image.width(), image.height(), image.depth(), 1);
			convert_to_greyscale(image, letter);
			convert_to_binary(letter, return_otsu_threshold(letter)); 
			crop_empty_space(letter, 0, 0);
			letter.resize(letter_dimension, letter_dimension, -100, -100, 5);
			convert_to_binary(letter, return_otsu_threshold(letter)); 

			std::stringstream namesave;
			namesave << "../data/letters/" << filenr << ".jpg"; 
			std::string filename_save = namesave.str();
			letter.save(filename_save.c_str());
			filenr ++;

			for(int letr_x = 0; letr_x < letter.width(); letr_x++) {
				for(int letr_y = 0; letr_y < letter.height(); letr_y++) {
					if (letter(letr_x,letr_y) == 0) {
						training_data << 1;
					}
					else {
						training_data << 0;
					}
				}
			}
			
			training_data << "\n";

			char value = split_string(file, "_")[9][0];

			training_data << symbolmap[value];

			training_data << "\n";
		}
	}
	training_data.close();
}


/*=============================================*/

#endif