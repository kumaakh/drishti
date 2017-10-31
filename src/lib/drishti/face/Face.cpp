/*! -*-c++-*-
  @file   Face.cpp
  @author David Hirvonen
  @brief  Internal implementation of a utility face model class.

  \copyright Copyright 2014-2016 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#include "drishti/face/Face.h"
#include "drishti/core/Shape.h"
#include "drishti/geometry/Primitives.h"
#include "drishti/geometry/motion.h"
#include "drishti/face/FaceIO.h"

#include <opencv2/videostab/global_motion.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/highgui.hpp>

#include <array>

DRISHTI_FACE_NAMESPACE_BEGIN

FaceModel::FaceModel() {} //  = default;
FaceModel::~FaceModel() = default;

float FaceModel::getInterPupillaryDistance() const
{
    float ipd = 0.f;
    if (eyeFullL.has && eyeFullR.has)
    {
        ipd = cv::norm(eyeFullL->irisEllipse.center - eyeFullR->irisEllipse.center);
    }
    return ipd;
}

bool FaceModel::getEyeRegions(cv::Rect2f& eyeR, cv::Rect2f& eyeL, float scale) const
{
    bool okay = false;
    if ((eyeLeft.size() && eyeRight.size()) || (eyeRightCenter.has && eyeLeftCenter.has))
    {
        okay = true;
        cv::Point2f pL = getEyeLeftCenter();
        cv::Point2f pR = getEyeRightCenter();
        cv::Point2f ratio(1.0, 3.0 / 4.0);
        cv::Point2f diag = ratio * cv::norm(pL - pR) * scale * 0.5f;
        eyeL = cv::Rect2f((pL - diag), (pL + diag));
        eyeR = cv::Rect2f((pR - diag), (pR + diag));
    }
    return okay;
}

cv::Point2f FaceModel::getEyeLeftCenter() const
{
    return eyeLeft.size() ? drishti::core::centroid(eyeLeft) : (eyeLeftCenter.has ? eyeLeftCenter.value : cv::Point2f(0, 0));
}

cv::Point2f FaceModel::getEyeRightCenter() const
{
    return eyeRight.size() ? drishti::core::centroid(eyeRight) : (eyeRightCenter.has ? eyeRightCenter.value : cv::Point2f(0, 0));
}

std::vector<cv::Point2f> asSpline(const std::vector<cv::Point2f>& points, int n, bool closed)
{
    std::vector<cv::Point2f> spline;
    drishti::core::fitSpline(points, spline, n, closed);
    if(closed)
    {
        spline.push_back(spline.front());
    }
    
    return spline;
}

std::vector<FaceModel::ContourVec> FaceModel::getFaceParts(bool fullEyes, bool browClosed) const
{
    bool useFull = (fullEyes && eyeFullR.has && eyeFullL.has);
    ContourVec eyeRight_ = useFull ? eyeFullR->getContours() : ContourVec(1, asSpline(eyeRight, 64, true));
    ContourVec eyeLeft_ = useFull ? eyeFullL->getContours() : ContourVec(1, asSpline(eyeLeft, 64, true));

    // clang-format off
    std::vector<ContourVec> features
    {
        { eyeRight_ },
        { eyeLeft_ },
        { asSpline(nose, 64, false) },
        { asSpline(eyebrowRight, 64, browClosed) },
        { asSpline(eyebrowLeft, 64, browClosed) },
        { asSpline(mouthOuter, 64, true) },
        { asSpline(mouthInner, 64, true) }

    };
    // clang-format on

    if (sideLeft.size() && sideRight.size())
    {
        auto cL = asSpline(sideLeft, 10, false);
        auto cR = asSpline(sideRight, 10, false);

        features.push_back({ cL });
        features.push_back({ cR });
    }

    return features;
};

bool checkForGoodMugShot(cv::Rect& cropRoi, const FaceModel& face, cv::Rect canvasSize)
{
	// convert roi into 4:3
	float a4by3 = 4.0f / 3.0f;
	float growBy = 1.3f;

	auto size = cv::Size(growBy*face.roi.value.width, growBy*face.roi.value.width*a4by3);

	float aspect = (1.0f*face.roi.value.width) / face.roi.value.height;

	if (aspect<1.0f)
		size = cv::Size(growBy*face.roi.value.height / a4by3, growBy*face.roi.value.height);

	//find center
	auto center = cv::Point(
		face.roi.value.x + (face.roi.value.width>> 1),
		face.roi.value.y + (face.roi.value.height>> 1));

	auto origin = center - cv::Point(size / 2);

	cropRoi = cv::Rect(origin, size);

	if ((cropRoi & canvasSize).area() < cropRoi.area())
		return false; //outside

	float grad = abs((face.eyeLeftCenter.value.y - face.eyeRightCenter.value.y) / (face.eyeLeftCenter.value.x - face.eyeRightCenter.value.x));

	if (grad>0.2) return false;

	/*float disparity= abs((face.eyeRightCenter.value.x+face.eyeLeftCenter.value.x- face.roi.value.width-2*face.roi.value.x) / face.roi.value.width);
	printf("x1=%f x2=%f disp=%f\n", face.eyeRightCenter.value.x, face.eyeLeftCenter.value.x, disparity);

	if (disparity > 0.07) return false;
*/
	// check how close are we to center, note center is flipped
	int x = center.x - (canvasSize.width/2); int y = center.y - (canvasSize.height/2);
	if (x*x + y*y > 0.05*canvasSize.width*canvasSize.width) return false; //off center

	//check how good the zoom is should be more than 20% of the captured image
	if (cropRoi.area() < 0.2*canvasSize.area())
		return false;

	return true;
}

cv::Rect flipVRect(const cv::Rect& r, const cv::Size& canvas)
{
	return cv::Rect(canvas.width - r.x - r.width, r.y, r.width, r.height);
}


bool FaceModel::checkForGoodMugShotAndDraw(cv::Rect& cropRoi, cv::Mat& canvas) const
{
	// ### Do the actual drawing ###
	//cv::rectangle(canvas, flipVRect(roi, canvas.size()), { 255 ,0,0 }, 2, 8);
	bool bGood = checkForGoodMugShot(cropRoi, *this, cv::Rect(cv::Point(0, 0), canvas.size()));

	cv::Scalar color = { 0, 255, 0 };
	if (!bGood) { color = { 0, 0, 255 }; }

	cv::ellipse(canvas, cv::RotatedRect(cv::Point2f((canvas.cols >> 1), (canvas.rows >> 1)), cv::Size2f(300, 400), 0), color, 2);

	//cv::rectangle(canvas, flipVRect(cropRoi, canvas.size()), color, 2, 8);

	return bGood;
}


void FaceModel::draw(cv::Mat& canvas, int width, bool fullEyes, bool allPoints) const
{
    static std::vector<cv::Vec3b> rainbow{
        { 0, 0, 255 },     // red
        { 0, 127, 255 },   // orange
        { 0, 200, 200 },   // yellow
        { 0, 255, 0 },     // green
        { 255, 0, 0 },     // blue
        { 130, 0, 75 },    // indigo
        { 255, 0, 139 },   // violet
        { 127, 127, 127 }, // white
    };

    if (allPoints && points.has)
    {
        for (auto& p : *points)
        {
            cv::circle(canvas, p, width, { 0, 255, 0 }, -1, 8);
        }
    }

    const bool browClosed = false;
    auto features = getFaceParts(fullEyes, browClosed);

    // Draw the nose tip estimate:
    std::vector<const core::Field<cv::Point2f>*> things{
        &noseTip,
        &eyebrowLeftInner,
        &eyebrowRightInner,
        &mouthCornerLeft,
        &mouthCornerRight
    };

    if (!fullEyes)
    {
        things.push_back(&eyeLeftInner);
        things.push_back(&eyeLeftOuter);
        things.push_back(&eyeRightInner);
        things.push_back(&eyeRightOuter);
    }

    // ### Do the actual drawing ###
    cv::rectangle(canvas, roi, { 0, 255, 0 }, width, 8);
    for (int i = 0; i < features.size(); i++)
    {
        auto& f = features[i];
        if (f.size())
        {
            std::vector<std::vector<cv::Point>> points(f.size());
            for (int j = 0; j < f.size(); j++)
            {
                std::copy(f[j].begin(), f[j].end(), std::back_inserter(points[j]));
            }
            cv::polylines(canvas, points, false, rainbow[i % rainbow.size()], width);
        }
    }

    for (auto& p : things)
    {
        if (p->has)
        {
            cv::circle(canvas, p->value, 4, { 0, 255, 0 }, -1, 8);
        }
    }
}

cv::Mat getSimilarityMotionFromEyes(const FaceModel& a, const FaceModel& b)
{
    std::array<cv::Point2f, 2> ptsA{ { a.eyeFullR->irisEllipse.center, a.eyeFullL->irisEllipse.center } };
    std::array<cv::Point2f, 2> ptsB{ { b.getEyeRightCenter(), b.getEyeLeftCenter() } };
    //std::array<cv::Point2f,2> ptsB {{ b.eyeFullR->irisEllipse.center, b.eyeFullL->irisEllipse.center }};

    return cv::Mat(transformation::estimateSimilarity(ptsA, ptsB));
}

cv::Mat getSimilarityMotion(const FaceModel& a, const FaceModel& b)
{
    std::array<cv::Point2f, 2> ptsA{ { a.getEyeRightCenter(), a.getEyeLeftCenter() } };
    std::array<cv::Point2f, 2> ptsB{ { b.getEyeRightCenter(), b.getEyeLeftCenter() } };
    return cv::Mat(transformation::estimateSimilarity(ptsA, ptsB));
}

cv::Mat getAffineMotion(const FaceModel& a, const FaceModel& b)
{
    CV_Assert(a.eyeLeftCenter.has);
    CV_Assert(a.eyeRightCenter.has);
    CV_Assert(a.noseTip.has);
    CV_Assert(a.mouthCornerLeft.has);
    CV_Assert(a.mouthCornerRight.has);

    cv::Point2f ptsA[3] = { a.eyeRightCenter.value, a.eyeLeftCenter.value, (a.mouthCornerRight.value + a.mouthCornerLeft.value) * 0.5f };

    CV_Assert(b.eyeLeftCenter.has);
    CV_Assert(b.eyeRightCenter.has);
    CV_Assert(b.noseTip.has);
    CV_Assert(b.mouthCornerLeft.has);
    CV_Assert(b.mouthCornerRight.has);

    cv::Point2f ptsB[3] = { b.eyeRightCenter.value, b.eyeLeftCenter.value, (b.mouthCornerRight.value + b.mouthCornerLeft.value) * 0.5f };

    cv::Mat H = cv::getAffineTransform(&ptsA[0], &ptsB[0]);
    return H;
}

cv::Mat estimateMotionLeastSquares(const FaceModel& a, const FaceModel& b)
{
    CV_Assert(a.eyeLeftCenter.has);
    CV_Assert(a.eyeRightCenter.has);
    CV_Assert(a.noseTip.has);
    CV_Assert(a.mouthCornerLeft.has);
    CV_Assert(a.mouthCornerRight.has);
    std::vector<cv::Point2f> ptsA{ a.eyeRightCenter.value, a.eyeLeftCenter.value, (a.mouthCornerRight.value + a.mouthCornerLeft.value) * 0.5f };

    CV_Assert(b.eyeLeftCenter.has);
    CV_Assert(b.eyeRightCenter.has);
    CV_Assert(b.noseTip.has);
    CV_Assert(b.mouthCornerLeft.has);
    CV_Assert(b.mouthCornerRight.has);
    std::vector<cv::Point2f> ptsB{ b.eyeRightCenter.value, b.eyeLeftCenter.value, (b.mouthCornerRight.value + b.mouthCornerLeft.value) * 0.5f };

    cv::Mat H = cv::videostab::estimateGlobalMotionLeastSquares(ptsA, ptsB, cv::videostab::MM_SIMILARITY);
    if (!H.empty())
    {
        H.convertTo(H, CV_64F);
    }
    return H;
}

DRISHTI_FACE_NAMESPACE_END
