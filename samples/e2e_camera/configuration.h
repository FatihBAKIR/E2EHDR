//
// Created by Mehmet Fatih BAKIR on 06/12/2016.
//

#ifndef HDR_CAMERA_CONFIGURATION_H
#define HDR_CAMERA_CONFIGURATION_H

#include <string>
#include <vector>
#include <tuple>
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
std::vector<std::pair<std::string, crf>> find_profiles();

namespace e2e
{
namespace app
{
    inline crf extract_crf(const nlohmann::json& meta)
    {
        static auto profs = []{
            std::map<std::string, crf> tmp;
            for (auto& i : find_profiles())
                tmp.insert(i);
            return tmp;
        }();
        return profs[meta["profile"]];
    }

    inline undistort extract_undistort(const nlohmann::json& meta)
    {
        return ::get_undistort(meta);
    }

    inline float extract_exposure(const nlohmann::json& meta)
    {
        return meta["exposure"];
    }
}
}


#endif //HDR_CAMERA_CONFIGURATION_H
