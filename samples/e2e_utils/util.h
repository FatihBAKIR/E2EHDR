//
// Created by Mehmet Fatih BAKIR on 19/10/2016.
//

#pragma once

#include <memory>
#include <gsl/gsl>

namespace e2e
{
    using byte = unsigned char;

    struct capture_error : public std::runtime_error
    {
        capture_error(const char* str) : std::runtime_error(str) {}
    };
}
