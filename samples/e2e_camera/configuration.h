//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#ifndef HDR_CAMERA_CONFIGURATION_H
#define HDR_CAMERA_CONFIGURATION_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <vector>

struct crf
{
    std::vector<float> red;
    std::vector<float> green;
    std::vector<float> blue;
};

boost::property_tree::ptree load_camera_conf(const std::string&);
crf load_crf(const std::string& file);

#endif //HDR_CAMERA_CONFIGURATION_H
