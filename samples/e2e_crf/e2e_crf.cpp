#include "e2e_crf.h"
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/xphoto.hpp>

#include <random>

using namespace Eigen;
using namespace std;

cv::Mat Edges(const cv::Mat& image)
{
    cv::Mat contours;
    cv::Mat hsv;
    cv::cvtColor(image, hsv, CV_RGB2GRAY);
    cv::Canny(hsv, contours, 10, 350);

//    cv::imshow("edges", contours);
//    cv::waitKey(0);

    return contours;
}

cv::Mat e2e_crf::Sample(const Exposure& exp)
{
    cv::Mat image(exp.h, exp.w, CV_8UC3, &exp.data[0]);
    auto contours = Edges(image);

    return contours;
}

float e2e_crf::weight(int pixelVal)
{
	if (pixelVal <= (_zMin + _zMax) / 2){
		return pixelVal - _zMin;
	}
	else {
		return _zMax - pixelVal;
	}
}

void e2e_crf::LoadImage(gsl::span<e2e::byte> buffer, int w, int h)
{
	e2e_crf::Exposure exposure;
    exposure.data = buffer;
    exposure.w = w;
    exposure.h = h;

    _images.push_back(exposure);
}

vector<int> e2e_crf::SampleSelection(const Exposure& image)
{
	int w = image.w;
	int h = image.h;

	vector< pair<e2e::byte, int> > samples; // 0-255, pixelIndex
	vector<int> points;
	cv::Mat edgeSample = Sample(image);

	random_device rand_dev;
	mt19937 generator(rand_dev());
	uniform_int_distribution<int> randomWidth(1, w - 2);
	uniform_int_distribution<int> randomHeight(1, h - 2);

	int numFound = 0;
	int numOfSuccessiveFailures = 0;

	while(numFound < 100){
		int width = randomWidth(generator);
		int height = randomHeight(generator);

		bool nearEdge = false;
		bool isDistinct = true;
		int oneDimensionVal;

		for(int i = width - 1; i <= width + 1 && !nearEdge; i++){
			for(int j = height - 1; j <= height + 1 && !nearEdge; j++) {
				oneDimensionVal = w * j + i;
				if (edgeSample.data[oneDimensionVal] > 128) // an edge is present near the selected point.
					nearEdge = true;
			}
		}

		oneDimensionVal = w * height + width;

		for (int i = 0; i < samples.size(); i++){
			if (find_if(samples.begin(), samples.end(), [&](const auto& lhs) { return image.PixelAt(oneDimensionVal).green == lhs.first; }) == samples.end()){
				isDistinct = false;
				break;
			}
		}

		if (isDistinct && !nearEdge){
			numFound++;
			numOfSuccessiveFailures = 0;
			samples.push_back(make_pair(image.PixelAt(oneDimensionVal).green, oneDimensionVal));
			points.push_back(oneDimensionVal);
		}
		else {
			numOfSuccessiveFailures++;
		}
	}

	return points;
}

vector<int> e2e_crf::AllPixels(const Exposure& image)
{
    vector<int> pixels;
    for (int i = 0; i < image.h; i++){
        for (int j = 0; j < image.w; j+=5){
            pixels.push_back(j + i * image.w);
        }
    }

    return pixels;
}

void e2e_crf::SolveCV()
{
    vector<cv::Mat> alignedImageSequence;

    for (auto& exp : _images)
    {
        alignedImageSequence.push_back(cv::Mat(exp.h, exp.w, CV_8UC3, &exp.data[0]));
    }

    float exposure = 0.125f / 2;
    for (int i = 0; i < _images.size(); i++){
        _exposureTimes.push_back(exposure);
        exposure *= 2;
    }

    cv::Mat response;
    cv::Ptr<cv::CalibrateDebevec> calibrate = cv::createCalibrateDebevec();
    calibrate->process(alignedImageSequence, response, _exposureTimes);

    for (int i = 0; i < 256; ++i)
    {
        _blueCurve.push_back(response.at<cv::Vec3f>(i)[0]);
        _greenCurve.push_back(response.at<cv::Vec3f>(i)[1]);
        _redCurve.push_back(response.at<cv::Vec3f>(i)[2]);
    }
}

void e2e_crf::SolveForCRF()
{
	float exposure = 0.125f / 2;

	// _exposureTimes.push_back(1.0f/15);
	// _exposureTimes.push_back(1.0f/30);
	// _exposureTimes.push_back(1.0f/60);
	// _exposureTimes.push_back(1.0f/100);
	// _exposureTimes.push_back(1.0f/250);

	for (int i = 0; i < _images.size(); i++){
		_exposureTimes.push_back(exposure);
		exposure *= 2;
	}

	int N = _images[0].PixelNum();
	int P = _images.size();

	vector<int> samples = AllPixels(_images[P / 2 + 2]);
	auto sampleCount = samples.size();

	int column = sampleCount + 256;
	int row = sampleCount * P + 257;

	_redSolution = MatrixXf::Zero(row, column);
	_greenSolution = MatrixXf::Zero(row, column);
	_blueSolution = MatrixXf::Zero(row, column);

	_redMatrix = VectorXf::Zero(row);
	_greenMatrix = VectorXf::Zero(row);
	_blueMatrix = VectorXf::Zero(row);

    // cv::Mat image(_images[2].h, _images[2].w, CV_8UC3, &_images[2].data[0]);
    // cv::imshow("win", image);
    // cv::waitKey(0);

	int t = 0;
	for (int i = 0; i < P; i++){			// Number of pixels
		for (int j = 0; j < samples.size(); j++){		// Number of images.
            auto val = samples[j];
            auto r = _images[i].PixelAt(samples[j]).red;
            auto g = _images[i].PixelAt(samples[j]).green;
            auto b = _images[i].PixelAt(samples[j]).blue;

			float weightRed = weight(_images[i].PixelAt(samples[j]).red);		// do the same for green and blue channels
			_redSolution(t, _images[i].PixelAt(samples[j]).red) = weightRed;
			_redSolution(t, _zMax + j) = -weightRed;

			_redMatrix(t) = weightRed * log(_exposureTimes[i]);

			float weightGreen = weight(_images[i].PixelAt(samples[j]).green);		// do the same for green and blue channels
			_greenSolution(t, _images[i].PixelAt(samples[j]).green) = weightGreen;
			_greenSolution(t, _zMax + 1 + j) = -weightGreen;

			_greenMatrix(t) = weightGreen * log(_exposureTimes[i]);

			float weightBlue = weight(_images[i].PixelAt(samples[j]).blue);		// do the same for green and blue channels
			_blueSolution(t, _images[i].PixelAt(samples[j]).blue) = weightBlue;
			_blueSolution(t, _zMax + 1 + j) = -weightBlue;

			_blueMatrix(t) = weightBlue * log(_exposureTimes[i]);
			t++;
		}
	}

	_redSolution(t, 128)   = 1;	// put the weight of 1 to the middle color value.
	_greenSolution(t, 128) = 1;	// put the weight of 1 to the middle color value.
	_blueSolution(t, 128)  = 1;	// put the weight of 1 to the middle color value.

	t++;

	float lamb = 250;

	for (int i = 0; i < 254; i++){	// curve is not differentiable in its edge values, so discard 0 and N - 1
		_redSolution(t, i)		     = lamb * weight(i);
		_redSolution(t, i + 1) 	     = - 2 * lamb * weight(i);
		_redSolution(t, i + 2) 	     = lamb * weight(i);

		_greenSolution(t, i)		 = lamb * weight(i);
		_greenSolution(t, i + 1) 	 = - 2 * lamb * weight(i);
		_greenSolution(t, i + 2) 	 = lamb * weight(i);

		_blueSolution(t, i)			 = lamb * weight(i);
		_blueSolution(t, i + 1) 	 = - 2 * lamb * weight(i);
		_blueSolution(t, i + 2) 	 = lamb * weight(i);
		t++;
	}

	JacobiSVD<MatrixXf> svdR(_redSolution,   ComputeThinU | ComputeThinV);
	JacobiSVD<MatrixXf> svdG(_greenSolution, ComputeThinU | ComputeThinV);
	JacobiSVD<MatrixXf> svdB(_blueSolution,  ComputeThinU | ComputeThinV);

	auto r = svdR.solve(_redMatrix);
	auto g = svdG.solve(_greenMatrix);
	auto b = svdB.solve(_blueMatrix);

	for (int i = 0; i < 256; i++){
		_redCurve.push_back(exp(r[i]));
		_greenCurve.push_back(exp(g[i]));
		_blueCurve.push_back(exp(b[i]));
	}
}
