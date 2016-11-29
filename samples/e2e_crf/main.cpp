#include <iostream>
#include <fstream>
#include "e2e_crf.h"
#include <opencv2/core.hpp>
#include <jpeg/jpeg_decode.h>
#include <profiler/profiler.h>
#include <Frame.h>

using namespace std;

int main(int argc, const char** argv)
{
    constexpr int NUM = 5;

    init_profiler("lel");
    auto base = string(argv[1]);
    std::cout << base << std::endl;

    vector<string> images;
    for (auto i = 0; i < NUM; i++){
        images.push_back(base + to_string(i) + ".jpg");
    }

    ifstream inFile;
    cout << images[0] << endl;

    e2e::byte* oData[NUM];

    vector<long> sizes;
    sizes.reserve(5);

    for (auto i = 0; i < NUM; i++) {
        inFile.open(images[i], ios::in|ios::binary|ios::ate);
        inFile.clear();
        if (inFile.is_open()) {
            inFile.seekg(0, ios::end);
            sizes[i] = inFile.tellg();
            cout << sizes[i] << endl;
            inFile.seekg(0, ios::beg);

            oData[i] = new e2e::byte[sizes[i] + 1];
            inFile.read(reinterpret_cast<char *>(oData[i]), sizes[i]);
        }
    }

    e2e::JpgDecoder decoder {gsl::span<e2e::byte>{oData[0], (long)sizes[0]}};

    auto frame0 = decoder.decode(gsl::span<e2e::byte>{oData[0], (long)sizes[0]});
    auto frame1 = decoder.decode(gsl::span<e2e::byte>{oData[1], (long)sizes[1]});
    auto frame2 = decoder.decode(gsl::span<e2e::byte>{oData[2], (long)sizes[2]});
    auto frame3 = decoder.decode(gsl::span<e2e::byte>{oData[3], (long)sizes[3]});
    auto frame4 = decoder.decode(gsl::span<e2e::byte>{oData[4], (long)sizes[4]});

    e2e_crf CRF;
    //CRF.LoadImage(reinterpret_cast<char*>(frame.buffer().data()), frame.width(), frame.height());
    CRF.LoadImage(frame0.buffer(), frame0.width(), frame0.height());
    CRF.LoadImage(frame1.buffer(), frame1.width(), frame1.height());
    CRF.LoadImage(frame2.buffer(), frame2.width(), frame2.height());
    CRF.LoadImage(frame3.buffer(), frame3.width(), frame3.height());
    CRF.LoadImage(frame4.buffer(), frame4.width(), frame4.height());

    CRF.SolveCV();

    auto redCRF = CRF.GetRedCRF();

    int x = 1;
    std::for_each(redCRF.begin(), redCRF.end(), [&](float i) {cout << x << " " << i << endl; x++;});

    std::cout << frame0.width() << ',' << frame1.height() << '\n';

    return 0;
}