//
// Created by Mehmet Fatih BAKIR on 24/10/2016.
//

#ifndef CAMERA_PROFILER_H
#define CAMERA_PROFILER_H

#include <chrono>
#include <boost/current_function.hpp>
#include "hash.h"
#include <algorithm>

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
    long            min_us = 1000 * 1000;
    long            max_us = 0;

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

template <int file_hash, int line>
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
        data.min_us = std::min<long>(data.min_us, us);
        data.max_us = std::max<long>(data.max_us, us);
        ++data.total_hits;
        current_prof = data.parent;
    }
};

template <int file_hash, int line>
thread_local profiler_data profiler<file_hash, line>::data {line};

#if !defined(NDEBUG) || (defined(DO_PROFILE) && DO_PROFILE)
#define scope_profile() profiler<WSID(__FILE__), __LINE__> __p__ {__FILE__, BOOST_CURRENT_FUNCTION};
#define named_profile(name) profiler<WSID(__FILE__), __LINE__> __p__ {__FILE__, (name)};
#else
#define scope_profile()
#define named_profile(name)
#endif

void print_tree();
void init_profiler(const char* name);

const profiler_data& current_profile();
#endif //CAMERA_PROFILER_H
