/*! -*-c++-*-
  @file   videoio/VideoSinkCVDisplay.cpp
  @author Akhil Kumar
  @brief  Live viewer for frames using opencv imshow.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}
*/

#include "videoio/VideoSinkCVDisplay.h"
#include "videoio/drishti_videoio.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "drishti/core/make_unique.h"

DRISHTI_VIDEOIO_NAMESPACE_BEGIN

VideoSinkCVDisplay::VideoSinkCVDisplay(const std::string& windowTitle):m_windowTitle(windowTitle){
	
}
VideoSinkCVDisplay::~VideoSinkCVDisplay() {}

bool VideoSinkCVDisplay::good() { return true; }
bool VideoSinkCVDisplay::begin() { 
	cv::namedWindow(m_windowTitle, cv::WINDOW_AUTOSIZE);
	return true;
}
bool VideoSinkCVDisplay::operator()(const cv::Mat& image) {
	cv::imshow(m_windowTitle, image);
	cv::waitKey(30);
	return true;
}
bool VideoSinkCVDisplay::end(const CompletionHandler& handler) {
	cv::destroyWindow(m_windowTitle);
	handler();
	return true;
}
void VideoSinkCVDisplay::setProperties(const Properties& properties) {

}



DRISHTI_VIDEOIO_NAMESPACE_END
