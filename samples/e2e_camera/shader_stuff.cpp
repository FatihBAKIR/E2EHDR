//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#include <shader_stuff.h>
#include <iostream>
#include <camera_struct.h>

e2e::GLSLProgram make_preview_shader(const camera_struct& cam, const crf& response)
{
    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/preview.frag");
    hdr.link();

    auto copy_camera = [&hdr](const auto& cam, const crf& crf, const std::string& pref)
    {
        auto undis = cam.get_undistort();

        std::cout << "copying response\n";
        hdr.setUniformArray(pref + ".response.red",  crf.red);
        hdr.setUniformArray(pref + ".response.green", crf.green);
        hdr.setUniformArray(pref + ".response.blue", crf.blue);

        hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });

        hdr.setUniformFVar(pref + ".undis.focal_length", {undis.focal[0], undis.focal[1]});
        hdr.setUniformFVar(pref + ".undis.optical_center", {undis.optical[0], undis.optical[1]});
        hdr.setUniformArray(pref + ".undis.dist_coeffs", undis.coeffs);
        hdr.setUniformFVar(pref + ".undis.image_size",  {undis.im_size[0], undis.im_size[1]});
    };

    copy_camera(cam, response, "camera");
    return hdr;
}


void make_merge_shader(e2e::GLSLProgram& hdr, const camera_struct& cam1, const camera_struct& cam2)
{
	auto copy_camera = [&hdr](const auto& cam, const std::string& pref)
	{
		hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });
	};

    copy_camera(cam1, "left");
    copy_camera(cam2, "right");   
}

void make_undistort_shader(e2e::GLSLProgram& hdr, const camera_struct& cam1, const crf& response1)
{
	auto copy_camera = [&hdr](const auto& cam, const crf& crf, const std::string& pref)
	{
		auto undis = cam.get_undistort();

		std::cout << "copying response\n";
		hdr.setUniformArray(pref + ".response.red", crf.red);
		hdr.setUniformArray(pref + ".response.green", crf.green);
		hdr.setUniformArray(pref + ".response.blue", crf.blue);

		hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });

		hdr.setUniformFVar(pref + ".undis.focal_length", { undis.focal[0], undis.focal[1] });
		hdr.setUniformFVar(pref + ".undis.optical_center", { undis.optical[0], undis.optical[1] });
		hdr.setUniformArray(pref + ".undis.dist_coeffs", undis.coeffs);
		hdr.setUniformFVar(pref + ".undis.image_size", { undis.im_size[0], undis.im_size[1] });
	};

	copy_camera(cam1, response1, "param");
}

e2e::GLSLProgram make_tonemap_shader_()
{
    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/tonemap.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/tonemap.frag");
    hdr.link();

    auto copy_camera = [&hdr](auto& cam, const std::string& pref)
    {
        /*hdr.setUniformFVar(pref + "Yw" , { static_cast<GL_FLOAT>(1e12f) });
        hdr.setUniformFVar(pref + "key", { static_cast<GL_FLOAT>(0.18f) });
        hdr.setUniformFVar(pref + "sat", { static_cast<GL_FLOAT>(1.0f) });*/
    };

    return hdr;
}