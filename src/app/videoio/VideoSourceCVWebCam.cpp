/*! -*-c++-*-
 @file   videoio/VideoSourceCVWebCam.cpp
 @author Akhil Kumar
 @brief  Simple implementation of a WebCam VideoSource.
 
 \copyright Copyright 2017 Elucideye, Inc. All rights reserved.
 \license{This project is released under the 3 Clause BSD License.}
 
 */

#include "videoio/VideoSourceCVWebCam.h"

#include "drishti/core/make_unique.h"
#include "drishti/testlib/drishti_cli.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "drishti/core/Logger.h"

using LoggerPtr = std::shared_ptr<spdlog::logger>;

DRISHTI_VIDEOIO_NAMESPACE_BEGIN

class VideoSourceCVWebCam::Impl
{
public:
    Impl(const std::string& webCamName)
        : m_count(0)
    {
		auto logger = drishti::core::Logger::create("VideoSourceCVWebCam");
		//parse m_webCam string for index,w,h,format that we need
		int idx=atoi(webCamName.c_str());
		m_cap.open(idx);
		if (!m_cap.isOpened())
		{
			logger->error("Could not open webcam");
		}
    }


    ~Impl() {}

    std::size_t count() const
    {
        return m_count;
    }

    VideoSourceCV::Frame operator()(int i = -1)
    {
		cv::Mat frm;
		m_cap >> frm;
        return VideoSourceCV::Frame(frm);
    }
	cv::VideoCapture m_cap;
	std::size_t m_count;
};

VideoSourceCVWebCam::VideoSourceCVWebCam(const std::string& filename)
{
    m_impl = drishti::core::make_unique<Impl>(filename);
}

VideoSourceCVWebCam::~VideoSourceCVWebCam()
{
}

std::size_t VideoSourceCVWebCam::count() const
{
    return m_impl->count();
}

VideoSourceCVWebCam::Frame VideoSourceCVWebCam::operator()(int i)
{
    return (*m_impl)(i);
}

DRISHTI_VIDEOIO_NAMESPACE_END
