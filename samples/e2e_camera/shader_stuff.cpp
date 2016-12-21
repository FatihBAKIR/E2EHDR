//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#include <shader_stuff.h>
#include <iostream>
#include <camera_struct.h>

e2e::GLSLProgram make_preview_shader(const camera_struct& cam)
{
    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/preview.frag");
    hdr.link();

    auto copy_camera = [&hdr](const auto& cam, const std::string& pref)
    {
        auto crf = cam.get_response();
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

    copy_camera(cam, "camera");
    return hdr;
}


e2e::GLSLProgram make_merge_shader()
{
    e2e::GLSLProgram hdr;
    hdr.attachShader(e2e::GLSLProgram::VERTEX_SHADER, "shaders/hdr.vert");
    hdr.attachShader(e2e::GLSLProgram::FRAGMENT_SHADER, "shaders/merge.frag");
    hdr.link();

    auto copy_camera = [&hdr](auto& cam, const std::string& pref)
    {
        auto crf_left = cam.get_response();
        auto undis_left = cam.get_undistort();

        hdr.setUniformArray(pref + ".response.red", crf_left.red);
        hdr.setUniformArray(pref + ".response.green", crf_left.green);
        hdr.setUniformArray(pref + ".response.blue", crf_left.blue);
        hdr.setUniformArray(pref + ".undis.dist_coeffs", undis_left.coeffs);

        hdr.setUniformFVar(pref + ".exposure", { cam.get_exposure() });

        hdr.setUniformArray(pref + ".undis.focal_length", undis_left.focal);
        hdr.setUniformArray(pref + ".undis.optical_center", undis_left.optical);

        hdr.setUniformArray(pref + ".undis.image_size", {1280, 720});
    };

    //copy_camera(left_cam, "left");
    //copy_camera(right_cam, "right");

    return hdr;
}