#include "./headers/main.hpp"

int main(int argc, char** argv){
	clock_t t1,t2,t3;
	t1 = clock();

	CImg<unsigned char> image(argv[1]);
	
	
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

	std::cout<<"Scaling set to: "<< scale <<" %%" <<std::endl;

	/* nearest-neighbor interpolation for speed, switch last option to 5 for lossless (no lol)*/
	image.resize(scale, scale,-100,-100,1);
	CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);
	
    t2=clock();
    std::cout<<"Runtime of image loading and resizing: "<<((float)t2-(float)t1)/CLOCKS_PER_SEC<<std::endl;

	run_canny_edge_detection(image, 
							edges, 
							CImgconsts::GAUSSIAN_SIZE, 
							CImgconsts::GAUSSIAN_SIGMA, 
							CImgconsts::HIGH_THRESHOLD_SCALE);
	t3=clock();
	edges.save("edges.jpg");
    std::cout<<"Runtime of canny edge detector: "<<((float)t3-(float)t2)/CLOCKS_PER_SEC<<std::endl;
	std::cout<<"Runtime in total: "<<((float)t3-(float)t1)/CLOCKS_PER_SEC<<std::endl;

	(image, edges).display("Edge detection", false);
}
