/*! -*-c++-*-
  @file   videoio/VideoSinkCVDisplay.h
  @author Akhil Kumar
  @brief  Live viewer for frames using opencv imshow.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}
*/

#ifndef __videoio_VideoSinkCVDisplay_h__
#define __VideoSinkCVDisplay_h__

#include "videoio/VideoSinkCV.h"
#include "videoio/drishti_videoio.h"

#include <string>
#include <memory>

DRISHTI_VIDEOIO_NAMESPACE_BEGIN

class VideoSinkCVDisplay : public VideoSinkCV
{
public:
	VideoSinkCVDisplay(const std::string& windowTitle);
    ~VideoSinkCVDisplay();

    virtual bool good();
    virtual bool begin();
    virtual bool operator()(const cv::Mat& image);
    virtual bool end(const CompletionHandler& handler);
    virtual void setProperties(const Properties& properties);
protected:
	std::string m_windowTitle;
};

DRISHTI_VIDEOIO_NAMESPACE_END

#endif // __videoio_VideoSinkCVDisplay_h__
