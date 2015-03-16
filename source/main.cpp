#include "./headers/main.hpp"

int main(int argc, char** argv){
	clock_t t1,t2,t3,t4;
	srand (time(NULL));
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


	std::cout<<"Scaling set to: "<< abs(scale) <<" %" <<std::endl;

	/* nearest-neighbor interpolation for speed, switch last option to 5 for lossless (no lol)*/
	image.resize(scale, scale,-100,-100,1);
	CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);
	
    t2=clock();
    std::cout<<"Runtime of image loading and resizing: "<<((float)t2-(float)t1)/CLOCKS_PER_SEC<<std::endl;

	run_canny_edge_detection(image, 
							edges, 
							3, 
							1, 
							CImgconsts::HIGH_THRESHOLD_SCALE);
	edges.save("edges.jpg");

	t3=clock();
	std::cout<<"Runtime of canny edge detector: "<<((float)t3-(float)t2)/CLOCKS_PER_SEC<<std::endl;

	std::vector<int> vertical_line_indexes;

	/* Network */
	std::vector<unsigned int> topology;
	topology.push_back(2200);/* Input nodes */
	topology.push_back(1500);/* Hidden nodes */
	topology.push_back(133);/* Output nodes, all ascii symbols plus norwegian letters */
	Network nnet(topology);

	std::vector<int> output_neuron_indexes

	for (int i = 0; i < 6; i++) {
		output_neuron_indexes.push_back(i);
	}

	remove_single_artifacts(edges, 0);
	crop_empty_space(edges, 255, 0);
	create_grid_separation(edges, vertical_line_indexes, 155, 255, 0);
	segment_letters(edges, vertical_line_indexes, 155, nnet);

	t4=clock();
	std::cout<<"Runtime of image segmentation: "<<((float)t4-(float)t3)/CLOCKS_PER_SEC<<std::endl;

	
	
	std::cout<<"Runtime in total: "<<((float)t4-(float)t1)/CLOCKS_PER_SEC<<std::endl;

	(image, edges).display("Edge detection", false);
}
