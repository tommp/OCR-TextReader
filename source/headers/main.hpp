
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
#include "canny_edge_detector.hpp"
/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <set>
/*---------------------------------------------*/

using namespace cimg_library;

namespace SDLconsts {
	const unsigned int window_length = 500;
	const unsigned int window_height = 500;
}




/*Header content*/
/*=============================================*/
/*=============================================*/

#endif