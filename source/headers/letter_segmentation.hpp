
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
	const int num_inputs = 1024;
	const std::string folder_path = "../data/SD19/HSF_0/*";
	const int min_dim = 4;
	const int max_dim = 200;
}

/*Header content*/
/*=============================================*/
using namespace cimg_library;

int calculate_row_height(CImg<unsigned char>& binary_img, char pos_value) {
	int width = binary_img.width();
	int height = binary_img.height();
	bool prev_was_gridline = false;
	bool prev_was_letter = false;
	int prev_line_start = 0;
	int prev_line_end = 0;
	int avg_line_height = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 5; x < width; x++) {//TODO::MORE DUMB STUFF DUE TO BAD BORDER HANDLING, FIX LATER
			if (binary_img(x,y) == pos_value) {
 				prev_was_gridline = false;
 				if (!prev_was_letter) {
 					prev_line_start = y;
 					prev_was_letter = true;
 				}
				break;
			}
			else if (x == width-1 && !prev_was_gridline) {
				prev_was_gridline = true;
				prev_was_letter = false;
				prev_line_end = y;
				if (prev_line_start == 0) {
					avg_line_height = prev_line_end - prev_line_start;
				}
				else {
					avg_line_height += prev_line_end - prev_line_start;
					avg_line_height /= 2;
				}
				for (int x_draw = 0; x_draw < width; x_draw++) {
					binary_img(x_draw,y) = 125;
				}
			}
		}
	}
	return avg_line_height;
}

void create_grid_separation(CImg<unsigned char>& binary_img, 
							std::vector<int>& vertical_line_indexes,
							unsigned char grid_value, 
							unsigned char plus_value) {

	int width = binary_img.width();
	int height = binary_img.height();
	bool prev_was_gridline = false;

	vertical_line_indexes.clear();
	vertical_line_indexes.push_back(0);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (binary_img(x,y) == plus_value) {
				prev_was_gridline = false;
				break;
			}
			else if (x == width-1 && !prev_was_gridline) {
				prev_was_gridline = true;
				vertical_line_indexes.push_back(y);
				for (int x_draw = 0; x_draw < width; x_draw++) {
					binary_img(x_draw,y) = grid_value;
				}
			}
		}
	}
	for (std::vector<int>::iterator it = vertical_line_indexes.begin(); it != vertical_line_indexes.end(); ++it) {
		for (int x = 0; x < width; x++) {
			for (int y = *it+1; y < height; y++) {
				if (binary_img(x,y) == plus_value) {
					break;
				}
				else if (binary_img(x,y) == grid_value || y == (height - 1)) {
					for (int y_draw = *it; y_draw < y; y_draw++) {
						binary_img(x,y_draw) = grid_value;
					}
					break;
				}
			}
		}
	}
}


//THIS IS BROKEN, FIX LATER
void crop_empty_space(CImg<unsigned char>& binary_img, unsigned char plus_value) {
	int width = binary_img.width();
	int height = binary_img.height();

	int x_left_crop = 0;
	int y_top_crop = 0;
	int x_right_crop = width;
	int y_bottom_crop = height;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (binary_img(x,y) == plus_value) {
				x_left_crop = x;
				goto x_right;
			}
		}
	}

	x_right:
	for (int x = width; x > 0; x--) {
		for ( int y = 0; y < height; y++) {
			if (binary_img(x,y) == plus_value) {
				x_right_crop = x;
				goto y_top;
			}
		}
	}

	y_top:

	for (int y = 0; y < height; y++) {
		for ( int x = 0; x < width; x++) {
			if (binary_img(x,y) == plus_value) {
				y_top_crop = y;
				goto y_bottom;
			}
		}
	}

	y_bottom:

	for (int y = height; y > 0; y--) {
		for ( int x = 0; x < width; x++) {
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
					crop_empty_space(letter, 255);
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

					nnet.feed_forward(network_input);
					nnet.get_results(result_values);
	
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
					if (it == '1') {
						input_values.push_back(1.0);
					}
					else {
						input_values.push_back(-1.0);
					}
				}
			}

			for (auto& it : output_line) {
				if (!((it) == '\n')){
					if (it == '1') {
						target_values.push_back(1.0);
					}
					else {
						target_values.push_back(-1.0);
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////
			nnet.feed_forward(input_values);
			nnet.backpropogate(target_values);
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

void segment_letters(CImg<unsigned char>& binary_img, unsigned char pos_value) {

	int number_of_too_small_objects = 0;
	int number_of_too_big_objects = 0;

	CImg<unsigned char> mask(binary_img.width(), binary_img.height(), binary_img.depth(), 1, 0);
	std::vector<Point> queue;
	for (int x = 0; x < binary_img.width(); x++) {
		for (int y = 0; y < binary_img.height(); y++) {
			if((binary_img(x, y) == pos_value) && (mask(x, y) != 1)) {
				int max_x = x;
				int max_y = y;
				int min_x = x;
				int min_y = y;
				queue.push_back(Point(x, y));
				mask(x, y) = 1;
				while (!queue.empty()) {
					Point current = queue.back();
					queue.pop_back();
					if (current.x < min_x) {
						min_x = current.x;
					}
					else if (current.x > max_x) {
						max_x = current.x;
					}
					if (current.y < min_y) {
						min_y = current.y;
					}
					else if (current.y > max_y) {
						max_y = current.y;
					}
					for (int x_inner = -1; x_inner < 2; x_inner++) {
						for (int y_inner = -1; y_inner < 2; y_inner++) {
							if((x_inner + current.x < 0) || 
								(y_inner + current.y < 0) || 
								(x_inner + current.x > binary_img.width()) || 
								(y_inner + current.y > binary_img.height())) {
								continue;
							}
							else if ( x_inner == 0 && y_inner == 0) {
								continue;
							}
							else {
								if ((binary_img(current.x + x_inner, current.y + y_inner) == pos_value) && 
													(mask(current.x + x_inner, current.y + y_inner) != 1)) {
									mask(current.x + x_inner, current.y + y_inner) = 1;
									queue.push_back(Point(current.x + x_inner, current.y + y_inner));
								}
							}
						}
					}
				}
				if (((max_x - min_x) < seg_consts::min_dim) || ((max_y - min_y) < seg_consts::min_dim)) {
					number_of_too_small_objects++;
					continue;
				}
				else if (((max_x - min_x) > seg_consts::max_dim) || ((max_y - min_y) > seg_consts::max_dim)) {
					number_of_too_big_objects++;
					continue;
				}
				else{
					CImg<unsigned char> letter = binary_img.get_crop(min_x, min_y, max_x, max_y);
					for (int i = min_x; i < max_x; i++) {
						binary_img(i, min_y) = 120;
						binary_img(i, max_y) = 120;
					}
					for (int i = min_y; i < max_y; i++) {
						binary_img(min_x, i) = 120;
						binary_img(max_x, i) = 120;
					}
					std::ostringstream sstream;
					sstream << "../data/letters/"<< min_x << "x" << min_y << ".JPG";
					std::string filename = sstream.str();
					letter.save(filename.c_str());
				}
			}
		}
	}
	std::cout << "Segmentation finished!" << std::endl;
	std::cout << "Number og objects elected too small to segment: " << 
				number_of_too_small_objects << std::endl;
	std::cout << "Number og objects elected too big to segment: " << 
				number_of_too_big_objects << std::endl;
	if (number_of_too_small_objects || number_of_too_big_objects) {
		std::cout << "Increase thresholds for object size to segment all objects" << std::endl;
	}
}

/*=============================================*/

#endif