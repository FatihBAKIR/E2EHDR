//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#include "configuration.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <array>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;


template <typename T>
std::vector<T> as_vector(ptree const& pt, ptree::key_type const& key)
{
  std::vector<T> r;
  for (auto& item : pt.get_child(key))
    r.push_back(item.second.get_value<T>());
  return r;
}

crf load_crf(const std::string& file)
{
  std::ifstream is {file};
  ptree pt2;

  read_json (is, pt2);

  auto red = as_vector<float>(pt2, "red");
  auto green = as_vector<float>(pt2, "red");
  auto blue = as_vector<float>(pt2, "blue");

  return crf { red, green, blue };
}

ptree load_camera_conf(const std::string& file)
{
  ptree pt2;
  std::ifstream is {file};
  read_json (is, pt2);
  return pt2;
}