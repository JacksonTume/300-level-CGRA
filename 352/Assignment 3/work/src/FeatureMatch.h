#pragma once

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d.hpp>

#include <opencv2/calib3d.hpp>

// std
#include <vector>
#include<iostream>

#define EPSILON 15

// typedef of Edge needs to be defined here so it can be passed into ComputeHomography
using Edge = std::pair<cv::Point2f, cv::Point2f>;

void FeatureMatch(cv::Mat& firstImage, cv::Mat& secondImage, cv::Mat& outputImage);

void ComputeFeatures(cv::Mat& firstImage, cv::Mat& secondImage, cv::Mat& outputImage);

cv::Mat ComputeHomography(cv::Mat& firstImage, cv::Mat& secondImage, std::vector<Edge> edges);