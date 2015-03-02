#include "./headers/main.hpp"

int main(int argc, char** argv){
	CImg<unsigned char> image(argv[1]);
	
	CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);

	run_canny_edge_detection(image, edges);
	
	edges.save("edges.jpg");
		
	(image, edges).display("Edge detection", false);
}
