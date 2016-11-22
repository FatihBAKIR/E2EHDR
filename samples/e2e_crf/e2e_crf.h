#ifndef __DEBEVEC_H__
#define __DEBEVEC_H__

#include <vector>
#include <Eigen/Dense>
#include <gsl/span>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <util.h>

using std::vector;
using namespace Eigen;

class e2e_crf
{
    struct Pixel
    {
        e2e::byte red;
        e2e::byte green;
        e2e::byte blue;

        Pixel(e2e::byte r, e2e::byte g, e2e::byte b) : red(r), green(g), blue(b) {}
    };

    struct Exposure
    {
        gsl::span<e2e::byte> data;
        int w, h;
        Pixel PixelAt(int ind) const {
            return Pixel(data[ind * 3], data[ind * 3 + 1], data[ind * 3 + 2]);
        }

        int PixelNum() const {
            return w * h;
        }
    };

	const int _zMin = 0;
	const int _zMax = 255;

	MatrixXf _redSolution;
	VectorXf _redMatrix;
	vector<float> _redCurve;

	MatrixXf _greenSolution;
	VectorXf _greenMatrix;
	vector<float> _greenCurve;

	MatrixXf _blueSolution;
	VectorXf _blueMatrix;
	vector<float> _blueCurve;
	int _n = 256;

	vector<Exposure> _images;
	vector<float> _exposureTimes;

	float weight(int pixelVal);
    vector<int> SampleSelection(const Exposure& image);
    cv::Mat Sample(const Exposure& image);

public:
    void LoadImage(gsl::span<e2e::byte> buffer, int w, int h);
    void SolveForCRF();

    vector<float> GetRedCRF() { return _redCurve; }
    vector<float> GetGreenCRF() { return _greenCurve; }
    vector<float> GetBlueCRF() { return _blueCurve; }
};

#endif // __DEBEVEC_H__
