
// project
#include "invert.hpp"

using namespace cv;

Mat cgraInvertImage(const Mat& m) {
	// NOTE: this implemnetation is only suitable
	// for a 3-channel ubyte-format image

	Mat r = m.clone();

	// manually iterate over all pixels
	for (int i = 0; i < r.rows; ++i) { // rows
		for (int j = 0; j < r.rows; ++j) { // cols
			r.at<Vec3b>(i, j) = Vec3b(255) - r.at<Vec3b>(i, j);
		}
	}

	return r;
}