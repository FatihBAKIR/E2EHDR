#include <iostream>
#include <fstream>
#include "debevec.h"
#include <opencv2/core/core.hpp>
#include <jpeg/jpeg_decode.h>
#include <profiler/profiler.h>

using namespace std;

int main()
{
    init_profiler("lel");

    auto img = "/home/fatih/Downloads/jigsaw.jpg";
    ifstream inFile;
    size_t size = 0;
    inFile.open(img, ios::in|ios::binary|ios::ate);

    e2e::byte* oData = 0;

    if(inFile.is_open()) {
        inFile.seekg(0, ios::end);
        size = inFile.tellg();
        cout << size << endl;
        inFile.seekg(0, ios::beg);

        oData = new e2e::byte[size+1];
        inFile.read(reinterpret_cast<char*>(oData), size);
    }

    e2e::JpgDecoder decoder {gsl::span<e2e::byte>{oData, (long)size}};

    auto frame = decoder.decode(gsl::span<e2e::byte>{oData, (long)size});

    std::cout << frame.width() << ',' << frame.height() << '\n';

    return 0;
}