//
// Created by Mehmet Fatih BAKIR on 24/10/2016.
//

#ifndef CAMERA_PROFILER_H
#define CAMERA_PROFILER_H

#include <chrono>
#include "hash.h"
#include <spdlog/spdlog.h>

template <unsigned fname_hash, unsigned line>
class profiler {
    static std::chrono::high_resolution_clock::time_point begin;

public:
    profiler();
    profiler(const profiler&) = delete;
    ~profiler();
};

template <unsigned hash, unsigned line>
profiler<hash, line>::profiler() {
    begin = std::chrono::high_resolution_clock::now();
}

template <unsigned hash, unsigned line>
profiler<hash, line>::~profiler() {
    auto diff = std::chrono::high_resolution_clock::now() - begin;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}
template <unsigned hash, unsigned line>
std::chrono::high_resolution_clock::time_point profiler<hash, line>::begin;

#define profile() profiler<WSID(__FILE__), __LINE__> __p__;

#endif //CAMERA_PROFILER_H
