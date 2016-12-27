//
// Created by Mehmet Fatih BAKIR on 20/12/2016.
//

#pragma once

#include <string>
#include <vector>
#include <camera_struct.h>

namespace e2e
{
namespace app
{
    void set_exif(const std::string& path, float time);
    crf recover_crf(const std::vector<typename camera_struct::FrameT>& ims, const std::vector<float>& times);
    crf parse_response(const std::string& path);
}
}
