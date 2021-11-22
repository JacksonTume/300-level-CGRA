// std
#include <iostream>
#include <chrono>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define ARRAY_LENGTH 17

// declare vectors to store the lf images and the uv-coords
std::vector<std::vector<Mat>> lf;
std::vector<std::vector<Vec2f>> uvCoords;

// forward declare methods
Vec3b GetPointOnImage(int row, int col, int t, int s);

// main program
int main(int argc, char** argv)
{
	// resize the vectors and give initial values
	lf.resize(ARRAY_LENGTH, std::vector<Mat>(ARRAY_LENGTH, Mat()));
	uvCoords.resize(ARRAY_LENGTH, std::vector<Vec2f>(ARRAY_LENGTH, Vec2f()));

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	// parse all images
	std::cout << "Loading light field ... (takes 4 - 10 seconds)" << std::endl;
	std::vector<cv::String> lf_imgs;
	cv::glob(argv[1], lf_imgs);

	for (cv::String cv_str : lf_imgs)
	{
		// get the filepath
		std::string filepath(cv_str);
		size_t pos = filepath.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			// replace "_" with " " 
			std::string filename = filepath.substr(pos + 1);
			pos = 0;
			while ((pos = filename.find("_", pos)) != std::string::npos)
			{
				filename.replace(pos, 1, " "); pos++;
			}
			// parse for values 
			std::istringstream ss(filename);
			std::string name;
			int row, col;
			float v, u;
			ss >> name >> row >> col >> v >> u;
			if (ss.good())
			{
				// TODO something with the image file "filepath" 
				// TODO something with the coordinates: row, col, v, u 

				// load the images into 2d array of Mat
				lf[row][col] = imread(filepath, IMREAD_COLOR);

				// load the uv-coords 2d array
				uvCoords[row][col] = Vec2f(v, u);

				continue;
			}
		}
		// throw error otherwise 
		std::cerr << "Filepath error with : " << filepath << std::endl;
		std::cerr << "Expected in the form : [prefix]/[name]_[row]_[col]_[v]_[u][suffix]";
		abort();
	}
	std::cout << "Finished loading light field" << std::endl;
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time taken to load in LightField = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl << std::endl;

	std::cout << "Core 1: Loading and Accessing the Light Field" << std::endl;
	std::cout << "Point on Image at 4D Point (7, 10, 384, 768): " << GetPointOnImage(7, 10, 384, 768) << std::endl << std::endl;

	// UV-array = 17x17 (ST-images)
	// ST-images = 512x1024 pixels

	// ST-array = 100x100 (UV-images)
	// UV-Image = 17x17 pixels

	/*
	 aperture centre
	 (-776.880371, 533.057190)
	 apeture a radius of 75
	 apeture b radius of 40

	 distance = sqrt((Va-Vi)^2 + (Ua-Ui)^2)
	 if distance < radius{image is inside aperture(visible)}
	 else {image is outside aperture (black)}
	*/

	begin = std::chrono::steady_clock::now();

	std::cout << "Reconstructing ST-array from UV-array ..." << std::endl;

	// ST-array of UV-images, 100x100 array
	std::vector<std::vector<Mat>> stArray;
	stArray.resize(100, std::vector<Mat>(100));

	int radius = 40;
	for (int stRow = 205; stRow < 305; stRow++)
	{
		for (int stCol = 770; stCol < 870; stCol++)
		{
			stArray[stRow - 205][stCol - 770] = Mat(Size(17, 17), lf[0][0].type(), Scalar(0, 0, 0));

			// UV-images in the ST-array, each image 17x17
			for (int uvRow = 0; uvRow < ARRAY_LENGTH; uvRow++)
			{
				for (int uvCol = 0; uvCol < ARRAY_LENGTH; uvCol++)
				{
					// get uv coords
					float v = uvCoords[uvRow][uvCol][0];
					float u = uvCoords[uvRow][uvCol][1];
					Vec2f apertureCentre(-776.880371, 533.057190);

					// calculate distance
					float distance = sqrt(
						pow((apertureCentre[0] - v), 2) +
						pow((apertureCentre[1] - u), 2));

					// if distance is inside the radius of aperture, then we use that pixel
					if (distance < radius)
					{
						stArray[stRow - 205][stCol - 770].at<Vec3b>(uvRow, uvCol) = GetPointOnImage(uvRow, uvCol, stRow, stCol);
					}
					else
					{
						// otherwise the pixel is outside the radius, and we leave it black
						stArray[stRow - 205][stCol - 770].at<Vec3b>(uvRow, uvCol) = Vec3b(0, 0, 0);
					}

				}
			}
		}
	}

	/*
	concatenate all the 100x100(of size 17x17) images into
	one 1700x1700 image.

	does not show an image, my display isnt big enough and it looks dumb =(
	but if saved the output is correct
	*/
	Mat output(Size(1700, 1700), lf[0][0].type(), Scalar(0));
	std::vector<Mat>outputs(100);

	for (int i = 0; i < 100; i++)
	{
		hconcat(stArray[i], outputs[i]);
	}
	vconcat(outputs, output);
	end = std::chrono::steady_clock::now();
	std::cout << "Time taken to reconstruct ST-array = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl << std::endl;

	/* Focal Stack Stuff */
	begin = std::chrono::steady_clock::now();
	std::cout << "Generating Focal Stack ..." << std::endl;

	// change focal length in range from about 2.5-0.7 to get different results
	float focalLength = 1;
	float cv = -776.880371;
	float cu = 533.057190;

	Mat focalLengthImage(512, 1024, lf[0][0].type());

	for (int i = 0; i < 512; i++)
	{
		for (int j = 0; j < 1024; j++)
		{
			int count = 0;
			Vec3i pixel(0, 0, 0);
			for (int k = 0; k < ARRAY_LENGTH; k++)
			{
				for (int l = 0; l < ARRAY_LENGTH; l++)
				{
					float dv = uvCoords[k][l][0] - cv;
					float du = uvCoords[k][l][1] - cu;

					int t = dv + (((focalLength * i) - dv) / focalLength);
					int s = du + (((focalLength * j) - du) / focalLength);

					if (t < 0) t = 0;
					if (t > 512) t = 512;
					if (s < 0) s = 0;
					if (s > 1024) s = 1024;

					pixel += GetPointOnImage(k, l, t, s);
					count++;
				}
			}
			focalLengthImage.at<Vec3b>(i, j) = pixel / count;
		}
	}

	end = std::chrono::steady_clock::now();
	std::cout << "Time taken to generate focal stack = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl << std::endl;

	imshow("Length", focalLengthImage);

	// wait for a keystroke in the window before exiting
	waitKey(0);
}

Vec3b GetPointOnImage(int row, int col, int t, int s)
{
	return lf[row][col].at<Vec3b>(t, s);
}
 