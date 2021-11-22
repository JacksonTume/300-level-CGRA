#include "FeatureMatch.h"

using namespace cv;

RNG rng;

vector<DMatch> matches;
vector<Edge> edges;
vector<bool> bestInliers;

void FeatureMatch(cv::Mat& firstImage, cv::Mat& secondImage, cv::Mat& outputImage)
{
	Mat input08 = firstImage.clone();
	Mat input11 = secondImage.clone();
	// compute features
	ComputeFeatures(input08, input11, outputImage);
	// compute homography transform
	Mat homographyTransform = ComputeHomography(input08, input11, edges);
	// align images
	Mat warpedImage, input08Border, input11Border;
	Vec3b black(0, 0, 0);

	copyMakeBorder(input08, input08Border, 50, 50, 50, 50, BORDER_CONSTANT, Scalar(black));
	copyMakeBorder(input11, input11Border, 50, 50, 50, 50, BORDER_CONSTANT, Scalar(black));

	warpPerspective(input08Border, warpedImage, homographyTransform, input08Border.size(), 1, BORDER_CONSTANT);

	// go through the image11 and paste the warped image where it fits
	for (int i = 0; i < input11Border.rows; i++)
	{
		for (int j = 0; j < input11Border.cols; j++)
		{
			Vec3b warpedPoint = warpedImage.at<Vec3b>(i, j);
			Vec3b input11Point = input11Border.at<Vec3b>(i, j);
			// only put the warped image on if the image11 is solid black and the warped image is not solid black
			if (warpedPoint != black && input11Point == black)
			{
				input11Border.at<Vec3b>(i, j) = warpedPoint;
			}
		}
	}
	imshow("border11", input11Border);

	// uncomment below for drawing the core2 lines
	for (int i = 0; i < bestInliers.size(); i++)
	{
		Edge e = edges.at(i);
		if (bestInliers.at(i))
		{
			line(outputImage, Point(e.first), Point(e.second) + Point(0, input11.rows), Scalar(0, 255, 0), 1, CV_AA);
	
		}
		else
		{
			line(outputImage, Point(e.first), Point(e.second) + Point(0, input11.rows), Scalar(0, 0, 255), 1, CV_AA);
		}
	}
	imwrite("../output/core3.png", input11Border);

}

void ComputeFeatures(Mat& firstImage, Mat& secondImage, Mat& outputImage)
{
	Mat input08 = firstImage.clone();
	Mat input11 = secondImage.clone();

	// identify features
	SiftFeatureDetector detector(400);
	vector<KeyPoint> kpoints1, kpoints2;
	detector.detect(input08, kpoints1);
	detector.detect(input11, kpoints2);

	// get descripters for the features
	SiftDescriptorExtractor extractor;
	Mat des1, des2;
	extractor.compute(input08, kpoints1, des1);
	extractor.compute(input11, kpoints2, des2);

	// match the features between images
	BFMatcher matcher(NORM_L2, true);
	matcher.match(des1, des2, matches);

	for (DMatch m : matches)
	{
		KeyPoint kp1 = kpoints1[m.queryIdx]; // first image
		KeyPoint kp2 = kpoints2[m.trainIdx]; // second image

		edges.push_back(Edge(kp1.pt, kp2.pt));
	}

	/* Uncomment to draw feature lines */
	//for (Edge e : edges)
	//{
	//	line(outputImage, Point(e.first), Point(e.second) + Point(0, input11.rows), Scalar(0, 255, 0), 1, CV_AA);
	//}
	//
	//imwrite("../output/core1.png", outputImage);
}

Mat ComputeHomography(Mat& firstImage, Mat& secondImage, std::vector<Edge> edges)
{
	Mat input08 = firstImage.clone();
	Mat input11 = secondImage.clone();

	int bestPairs = -1;

	// estimate homography transformation over 100 iterations
	for (int i = 0; i < 100; i++)
	{
		int numPairs = 0;

		vector<Point2f> firstImagePoints(4);
		vector<Point2f> secondImagePoints(4);

		vector<bool> inliers(edges.size(), false);

		// get 4 pairs, selected randomly
		for (int j = 0; j < 4; j++)
		{
			int index = rng.uniform(0, edges.size() - 1);
			firstImagePoints[j] = edges.at(index).first;
			secondImagePoints[j] = edges.at(index).second;
		}
		// compute homography transform for the 4 pairs
		Mat homographyTransform = findHomography(firstImagePoints, secondImagePoints);

		for (int j = 0; j < edges.size(); j++)
		{
			Point2f firstPoint = edges.at(j).first;
			
			double x = ((homographyTransform.at<double>(0, 0) * firstPoint.x) + 
						(homographyTransform.at<double>(0, 1) * firstPoint.y) +
						(homographyTransform.at<double>(0, 2)));

			double y = ((homographyTransform.at<double>(1, 0) * firstPoint.x) + 
						(homographyTransform.at<double>(1, 1) * firstPoint.y) + 
						(homographyTransform.at<double>(1, 2)));

			double z = ((homographyTransform.at<double>(2, 0) * firstPoint.x) + 
						(homographyTransform.at<double>(2, 1) * firstPoint.y) + 
						(homographyTransform.at<double>(2, 2)));
			
			Point2f secondPoint(x/z , y/z);

			double error = sqrt(pow(edges.at(j).second.x - secondPoint.x, 2) - pow(edges.at(j).second.y - secondPoint.y, 2));
			// compute inliers when error is less than epsilon
			if (error < EPSILON)
			{
				inliers.at(j) = true;
				numPairs++;
			}
		}
		// save the most number of pairs
		if (numPairs > bestPairs)
		{
			bestPairs = numPairs;
			bestInliers = inliers;
		}
	}
	// recompute homography transform with best inliers
	vector<Point2f> inliersFirstImage(bestPairs);
	vector<Point2f> inliersSecondImage(bestPairs);
	for (int i = 0; i < edges.size(); i++)
	{
		if (bestInliers.at(i))
		{
			inliersFirstImage.push_back(edges.at(i).first);
			inliersSecondImage.push_back(edges.at(i).second);
		}
	}

	return findHomography(inliersFirstImage, inliersSecondImage, CV_RANSAC);
}
