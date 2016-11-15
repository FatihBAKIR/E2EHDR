#include <iostream>
#include <fstream>
#include "e2e_crf.h"
#include <opencv2/core.hpp>
#include <jpeg/jpeg_decode.h>
#include <profiler/profiler.h>
#include <Frame.h>

using namespace std;

int main()
{
    constexpr int NUM = 5;

    init_profiler("lel");
    auto base = "/Users/goksu/Documents/E2EHDR/samples/e2e_crf/sample";

    vector<string> images;
    for (auto i = 0; i < NUM; i++){
        images.push_back(base + to_string(i + 1) + ".jpg");
    }

    ifstream inFile;
    cout << images[0] << endl;

    e2e::byte* oData[NUM];

    vector<size_t> sizes;
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
    CRF.LoadImage({reinterpret_cast<char*>(frame0.buffer().data()), frame0.buffer().size()}, frame0.width(), frame0.height());
    CRF.LoadImage({reinterpret_cast<char*>(frame1.buffer().data()), frame1.buffer().size()}, frame1.width(), frame1.height());
    CRF.LoadImage({reinterpret_cast<char*>(frame2.buffer().data()), frame2.buffer().size()}, frame2.width(), frame2.height());
    CRF.LoadImage({reinterpret_cast<char*>(frame3.buffer().data()), frame3.buffer().size()}, frame3.width(), frame3.height());
    CRF.LoadImage({reinterpret_cast<char*>(frame4.buffer().data()), frame4.buffer().size()}, frame4.width(), frame4.height());

    CRF.SolveForCRF();

    std::cout << frame0.width() << ',' << frame1.height() << '\n';

    return 0;
}