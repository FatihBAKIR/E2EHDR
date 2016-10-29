//
// Created by Mehmet Fatih BAKIR on 24/10/2016.
//

#ifndef CAMERA_PROFILER_H
#define CAMERA_PROFILER_H

#include <chrono>
#include <boost/current_function.hpp>
#include "hash.h"

struct profiler_data
{
    using time_point = std::chrono::high_resolution_clock::time_point;

    const char*     file;
    const char*     name;
    const unsigned  line;

    time_point      cur_begin;
    long            total_duration;
    unsigned long   total_hits;
    long            last_us;

    profiler_data*  parent;
    profiler_data*  child;
    profiler_data*  sibling;

    profiler_data(unsigned line) :
            line{line}, total_duration{0}, total_hits{0}, parent{nullptr}, child{nullptr}, sibling{nullptr}
    {}

    profiler_data(const profiler_data&) = delete;
    profiler_data(profiler_data&&) = default;
};

extern thread_local profiler_data* current_prof;

template <unsigned file_hash, unsigned line>
class profiler
{
    static thread_local profiler_data data;

public:
    profiler(const char* file, const char* name)
    {
        static thread_local auto _ = ([&]{
            data.file = file;
            data.name = name;
            data.sibling = current_prof->child;
            data.parent = current_prof;
            current_prof->child = &data;
            return 0;
        })();
        (void)_;
        data.cur_begin = std::chrono::high_resolution_clock::now();
        current_prof = &data;
    }
    profiler(const profiler&) = delete;
    ~profiler()
    {
        const auto diff = std::chrono::high_resolution_clock::now() - data.cur_begin;
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();

        data.total_duration += us;
        data.last_us = us;
        ++data.total_hits;
        current_prof = data.parent;
    }
};

template <unsigned file_hash, unsigned line>
thread_local profiler_data profiler<file_hash, line>::data {line};

#define scope_profile() profiler<WSID(__FILE__), __LINE__> __p__ {__FILE__, BOOST_CURRENT_FUNCTION};
#define named_profile(name) profiler<WSID(__FILE__), __LINE__> __p__ {__FILE__, (name)};

void print_tree();
void init_profiler(const char* name);

const profiler_data& current_profile();
#endif //CAMERA_PROFILER_H
