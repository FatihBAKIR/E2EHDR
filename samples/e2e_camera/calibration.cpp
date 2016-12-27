//
// Created by Mehmet Fatih BAKIR on 20/12/2016.
//

#include <string>
#include <cstdlib>
#include <vector>
#include <calibration.h>
#include <jpeg/jpeg_encode.h>
#include <fstream>

namespace e2e
{
namespace app
{
    void set_exif(const std::string &path, float time) {
        auto tstring = std::to_string(1.0f / time);
        auto cmd = "exiftool " + path + " -EXIF:ExposureTime=1/" + tstring;
        ::system(cmd.c_str());
    }

    crf parse_response(const std::string& path)
    {
        std::ifstream f{ path };

        std::vector<float> red (256);
        std::vector<float> green (256);
        std::vector<float> blue (256);

        auto read_channel = [&f](auto& out)
        {
            std::string ln;

            while (ln != "# columns: 3")
            {
                std::getline(f, ln);
            }

            float log_res, res;
            int index;

            for (auto i = 0; i < 256; ++i) {
                f >> log_res >> index >> res;
                if (i != index)
                {
                    throw std::runtime_error("Broken response file");
                }
                out[i] = res;
            }
        };

        read_channel(red);
        read_channel(green);
        read_channel(blue);

        return {red, green, blue};
    }

    crf recover_crf(const std::vector<camera_struct::FrameT>& ims, const std::vector<float> &times) {
        std::string tmp_dir = "/tmp/";

        std::vector<std::string> paths;
        for (int i = 0; i < ims.size(); ++i)
        {
            const auto& f = ims[i];
            float time = times[i];

            using namespace std::string_literals;
            auto tstring = std::to_string(1.0f / time);
            auto path = tmp_dir + "exposure_"s + tstring + ".jpg";
            e2e::save_jpeg(f.buffer().data(), f.width(), f.height(), path);

            e2e::app::set_exif(path, time);
            paths.push_back(std::move(path));
        }

        std::string paths_s;
        for (auto& p : paths)
        {
            paths_s += p + " ";
        }

        std::string out_file = tmp_dir + "response_" + std::to_string(std::rand()) + ".m";

        ::system(("pfsinme " + paths_s + "  | pfshdrcalibrate -v -s " + out_file + " > /dev/null").c_str());

        return parse_response(out_file);
    }
}
}
