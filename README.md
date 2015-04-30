# OCR-TextReader
A C++ application for reading text from images

Run it from the source folder, command structure is: 
../tr <threshold type> <image file path> <draw boxes> <save objects> <read> 

Options:
	<threshold type> can be c or z, c runs the canny edge detector, z runs with localized pooled thresholding.
	<image file path> path to the image to process.
	<draw boxes> can be 0 or 1, if 1, draws the boundary boxes on the output image.
	<save objects> can be 0 or 1, if 1, saves each segmented object in ../data/letters/
	<read> can be 0 or 1, if 1, reads the segmented leters and stores the result in results.txt

Example command: 

../tr z ../data/hello_world.JPG 1 1 1

This will run the system with thrasholding on the image hello_world.JPG stored in ../data/. It will draw the boundary boxes, save each individually segmented symbol in ../data/letters and read the letters, storing the values in results.txt.

Specialized commands:
	../tr g : This will generate training data from the ../data/SD19/HSF_0/F0* folders. Stores the textual representation of the data in training_data.txt.

	../tr t : Trains the network with the training data stored in training_data.txt (WARNING: This will overwrite the previously stored weights, might be clever to stash them somwere first).


