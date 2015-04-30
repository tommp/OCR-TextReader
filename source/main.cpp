#include "./headers/main.hpp"

int main(int argc, char** argv){

	if (argc < 2) {
		std::cout << "Invalid number of inputs: " << argc << std::endl;
		return -1;
	}

	srand (time(NULL));

	std::vector<unsigned int> topology;
	topology.push_back(1024);// Input nodes
	topology.push_back(400);
	topology.push_back(62);// Output nodes 
	Net nnet(topology);

	std::map<char, std::string> symbolmap;
	std::map<std::vector<int>, char> valuemap;
	std::map<char,std::vector<unsigned int>> template_histogram;

	load_symbolmap(symbolmap, valuemap, "letter_values.txt");
	load_templates(template_histogram, "template_data.txt");

	if (argv[1][0] == 'g') {
		std::cout << "Generating training data from SD19 data set, this could take a while..." << std::endl;
		generate_training_data_SD19(topology, "training_data.txt", "template_data.txt", symbolmap);
		std::cout << "Training data generated!" << std::endl;
	}
	else if (argv[1][0] == 't'){
		clock_t t1, t2;
		std::cout<<"Restoring weights..."<<std::endl;
		try {
			nnet.load_weights("weights.txt");
			std::cout<<"Weights restored, commencing training!"<<std::endl;
		}
		catch(...) {
			std::cout << "Failed to load weights" << std::endl;
		}
		
		for (int i = 0; i < 1000000; i++) {
			std::cout << "======= Started epoch number " << i + 1 << " =======" << std::endl;
			t1 = clock();
			train_network("training_data.txt", nnet);
			nnet.store_weights("weights.txt");
			t2 = clock();
			std::cout << "======= Epoch number " << i + 1 << " completed =======" << std::endl;
			std::cout << "Time spent training: " <<((float)t2-(float)t1)/CLOCKS_PER_SEC << " seconds"<< std::endl;
			std::cout <<"Recent average error: " << nnet.get_recent_average_error() << std::endl;
			std::cout <<"Last iteration error: " << nnet.get_error() << std::endl;
			std::cout << "======================================\n\n" << std::endl;
			if (nnet.get_recent_average_error() < 0.05) {
				break;
			}
		}
	}
	else if(argv[1][0] == 'z') {
		if (argc < 7) {
			std::cout << "Invalid number of threshold inputs: " << argc << std::endl;
			return -1;
		}
		clock_t t1,t2,t3,t4;
		t1 = clock();

		CImg<unsigned char> image(argv[2]);
		std::cout<<"Restoring weights..."<<std::endl;
		nnet.load_weights("weights.txt");
		std::cout<<"Weights restored, commencing processing!"<<std::endl;

		rescale_image(image);
		
	    t2=clock();
	    std::cout<<"Runtime of image loading and resizing: "<<((float)t2-(float)t1)/CLOCKS_PER_SEC<<std::endl;
	    
	    CImg<unsigned char> grayscale(image.width(), image.height(), image.depth(), 1, 255);
	    convert_to_greyscale(image, grayscale);

	    localized_pooled_thresholding(grayscale, std::stoi(argv[3], nullptr), -1);

		t3=clock();
		std::cout<<"Runtime of binary conversion: "<<((float)t3-(float)t2)/CLOCKS_PER_SEC<<std::endl;

	 	std::cout << "Average line height: " << calculate_row_height(grayscale, 0) << " pixels" << std::endl;
	 	read_letters(grayscale, 
			 		nnet, 
			 		0, 
			 		"results.txt", 
			 		topology, 
			 		valuemap, 
			 		template_histogram, 
			 		std::stoi(argv[4], nullptr), 
			 		std::stoi(argv[5], nullptr), 
			 		std::stoi(argv[6], nullptr));
		t4=clock();
		std::cout<<"Runtime of image segmentation and reading: "<<((float)t4-(float)t3)/CLOCKS_PER_SEC<<std::endl;
		std::cout<<"Runtime in total: "<<((float)t4-(float)t1)/CLOCKS_PER_SEC<<std::endl;

		(image, grayscale).display("Thresholding detection", false);
	}
	else if(argv[1][0] == 'c') {
		if (argc < 6) {
			std::cout << "Invalid number of canny inputs: " << argc << std::endl;
			return -1;
		}
		clock_t t1,t2,t3,t4;
		t1 = clock();

		CImg<unsigned char> image(argv[2]);
		std::cout<<"Restoring weights..."<<std::endl;
		nnet.load_weights("weights.txt");
		std::cout<<"Weights restored, commencing processing!"<<std::endl;

		rescale_image(image);
		
	    t2=clock();
	    std::cout<<"Runtime of image loading and resizing: "<<((float)t2-(float)t1)/CLOCKS_PER_SEC<<std::endl;
	    
	    CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);
		run_canny_edge_detection(image, edges, CImgconsts::GAUSSIAN_SIZE, CImgconsts::GAUSSIAN_SIGMA, CImgconsts::HIGH_THRESHOLD_SCALE);

		t3=clock();
		std::cout<<"Runtime of canny edge detector: "<<((float)t3-(float)t2)/CLOCKS_PER_SEC<<std::endl;

	 	crop_empty_space(edges, 255, 1);

		read_letters(edges, 
					nnet, 
					255, 
					"results.txt", 
					topology, 
					valuemap, 
					template_histogram, 
					std::stoi(argv[3], nullptr), 
					std::stoi(argv[4], nullptr), 
					std::stoi(argv[5], nullptr));

		t4=clock();
		std::cout<<"Runtime of image segmentation and reading: "<<((float)t4-(float)t3)/CLOCKS_PER_SEC<<std::endl;
		std::cout<<"Runtime in total: "<<((float)t4-(float)t1)/CLOCKS_PER_SEC<<std::endl;

		(image, edges).display("Edge detection", false);
	}
	else {
		std::cout << "Invalid input!" << std::endl;
	}
}