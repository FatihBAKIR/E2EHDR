//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#include "configuration.h"
#include <fstream>

crf load_crf(const std::string& file)
{
  std::ifstream is {file};
  nlohmann::json j;

  is >> j;

  auto red = j["red"];
  auto green =  j["green"];
  auto blue = j["blue"];

  return crf { red, green, blue };
}

nlohmann::json load_camera_conf(const std::string& file)
{
  nlohmann::json j;
  std::ifstream is {file};
  is >> j;
  return j;
}

undistort get_undistort(const nlohmann::json &config) {
  undistort u;
  auto f = config["undistort"]["focal_length"];
  auto o = config["undistort"]["optical_center"];
  auto c = config["undistort"]["distort_coeffs"];
  return undistort{{f["x"], f["y"]}, {o["x"], o["y"]}, c};
}
