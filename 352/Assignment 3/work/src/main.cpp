
// std
#include <iostream>
#include <vector>

#include <chrono>
#include <ctime>  

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


// project
#include "FeatureMatch.h"

using namespace cv;
using namespace std;

// main program
int main( int argc, char** argv ) {

	// check we have exactly one additional argument
	// eg. res/vgc-logo.png
	if( argc != 2) {
		cerr << "Usage: cgra352 <Image>" << endl;
		abort();
	}


	//// read the file
	//Mat image;
	//image = imread(argv[1], CV_LOAD_IMAGE_COLOR); 
	//
	//// check for invalid input
	//if(!image.data ) {
	//	cerr << "Could not open or find the image" << std::endl;
	//	abort();
	//}

	// take current time
	auto start = std::chrono::system_clock::now();

	Mat input08 = imread("../work/res/frames/Input08.jpg", CV_LOAD_IMAGE_COLOR);
	Mat input11 = imread("../work/res/frames/Input11.jpg", CV_LOAD_IMAGE_COLOR);;

	// output image
	Mat output;
	vconcat(input08, input11, output);

	FeatureMatch(input08, input11, output);

	// display image
	//namedWindow("Output", WINDOW_AUTOSIZE);
	imshow("Output", output);

	// take after time then output total time taken
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	std::cout << "finished computation at " << std::ctime(&end_time)
		<< "elapsed time: " << elapsed_seconds.count() << "s\n";

	// wait for a keystroke in the window before exiting
	waitKey(0);
}