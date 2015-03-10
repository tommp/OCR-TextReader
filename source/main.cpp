#include "./headers/main.hpp"

int main(int argc, char** argv){
	CImg<unsigned char> image(argv[1]);
	
	CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);
	clock_t t1,t2;
    t1=clock();
	run_canny_edge_detection(image, 
							edges, 
							CImgconsts::GAUSSIAN_SIZE, 
							CImgconsts::GAUSSIAN_SIGMA, 
							CImgconsts::HIGH_THRESHOLD_SCALE);
	t2=clock();
	edges.save("edges.jpg");

	float diff ((float)t2-(float)t1);
    std::cout<<"Runtime of canny edge detector: "<<diff/CLOCKS_PER_SEC<<std::endl;
		
	(image, edges).display("Edge detection", false);
}
