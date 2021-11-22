
// std
#include <iostream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// project
#include "invert.hpp"


using namespace cv;
using namespace std;


// main program
// 
int main( int argc, char** argv ) {

	// check we have exactly one additional argument
	// eg. res/vgc-logo.png
	if( argc != 2) {
		cerr << "Usage: cgra352 <Image>" << endl;
		abort();
	}


	// read the file
	Mat image;
	image = imread(argv[1], CV_LOAD_IMAGE_COLOR); 

	// check for invalid input
	if(!image.data ) {
		cerr << "Could not open or find the image" << std::endl;
		abort();
	}


	// use our function to invert it
	//
	Mat inverted = cgraInvertImage(image);


	// save image
	imwrite("output/image.png", image);
	imwrite("output/inverted.png", image);


	// create a window for display and show our image inside it
	string img_display = "Image Display";
	namedWindow(img_display, WINDOW_AUTOSIZE);
	imshow(img_display, image);

	string inv_img_display = "Inverted Image Display";
	namedWindow(inv_img_display, WINDOW_AUTOSIZE);
	imshow(inv_img_display, inverted);


	// wait for a keystroke in the window before exiting
	waitKey(0);
}