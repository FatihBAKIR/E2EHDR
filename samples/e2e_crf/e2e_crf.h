#ifndef __DEBEVEC_H__
#define __DEBEVEC_H__

#include <vector>
#include <Eigen/Dense>
#include <gsl/span>
#include <opencv2/core/mat.hpp>

using std::vector;
using namespace Eigen;

class e2e_crf
{
    struct Pixel
    {
        char red;
        char green;
        char blue;

        Pixel(char r, char g, char b) : red(r), green(g), blue(b) {}
    };

    struct Exposure
    {
        gsl::span<char> data;
        int w, h;
        Pixel PixelAt(int ind) const {
            return Pixel(data[ind], data[ind + 1], data[ind + 2]);
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
    void LoadImage(gsl::span<char> buffer, int w, int h);
    void SolveForCRF();

    vector<float> GetRedCRF() { return _redCurve; }
    vector<float> GetGreenCRF() { return _greenCurve; }
    vector<float> GetBlueCRF() { return _blueCurve; }
};

#endif // __DEBEVEC_H__
