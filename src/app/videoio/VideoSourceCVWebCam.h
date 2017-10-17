/*! -*-c++-*-
 @file   videoio/VideoSourceCVWebCam.h
 @author Akhil Kumar
 @brief  Simple declaration of a webcam VideoSource.
 
 \copyright Copyright 2017 Elucideye, Inc. All rights reserved.
 \license{This project is released under the 3 Clause BSD License.}
 
 */

#ifndef __videoio_VideoSourceCVWebCam_h__
#define __videoio_VideoSourceCVWebCam_h__

#include "videoio/VideoSourceCV.h"
#include "videoio/drishti_videoio.h"

#include <string>
#include <memory>

DRISHTI_VIDEOIO_NAMESPACE_BEGIN

class VideoSourceCVWebCam : public VideoSourceCV
{
public:
    class Impl;

	VideoSourceCVWebCam(const std::string& webCamName);
    ~VideoSourceCVWebCam();
    virtual Frame operator()(int i = -1);
    virtual std::size_t count() const;
    virtual bool isRandomAccess() const { return false; }

protected:
    std::unique_ptr<Impl> m_impl;
};

DRISHTI_VIDEOIO_NAMESPACE_END

#endif // __videoio_VideoSourceCVWebCam_h__
