//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#ifndef HDR_CAMERA_CONFIGURATION_H
#define HDR_CAMERA_CONFIGURATION_H

#include <string>
#include <vector>
#include <json.hpp>

struct crf
{
    std::vector<float> red;
    std::vector<float> green;
    std::vector<float> blue;
};

struct undistort
{
    std::vector<float> focal;
    std::vector<float> optical;

    std::vector<float> coeffs;

    std::vector<float> im_size;
};

nlohmann::json load_camera_conf(const std::string&);
crf load_crf(const std::string& file);
void save_crf(const crf& res, const std::string& out);
undistort get_undistort(const nlohmann::json& config);

#endif //HDR_CAMERA_CONFIGURATION_H
