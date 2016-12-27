//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#pragma once
#include <glsl_program.h>

class camera_struct;
struct crf;

e2e::GLSLProgram make_preview_shader(const camera_struct& cam, const crf& response);
void make_merge_shader(e2e::GLSLProgram& hdr, const camera_struct& cam1, const crf& response1, const camera_struct& cam2, const crf& response2);
void make_undistort_shader(e2e::GLSLProgram& hdr, const camera_struct& cam1, const crf& response1);