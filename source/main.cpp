#include "./headers/main.hpp"



int main(int argc, char** argv){
	CImg<unsigned char> image(argv[1]);
	
	CImg<unsigned char> edges(image.width(), image.height(), image.depth(), 1);

	run_canny_edge_detection(image, edges);
	
	edges.save("edges.jpg");
		
	(image, edges).display("Edge detection", false);
}

void pollevent(bool& var){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch (event.type){
			case SDL_QUIT:
				exit(0);
				break;

			case SDL_KEYDOWN:
				var = true;
				break;
			default:
				break;
		}
	}
}


void waitForEvent(){
	SDL_Event event;
	bool done = false;
	while((!done) && (SDL_WaitEvent(&event))) {
        switch(event.type) {
    
            case SDL_KEYDOWN:
                done = true;
                break;


            case SDL_QUIT:
                done = true;
                exit(0);
                break;
                
            default:
                break;
        } 
            
    }
}
