//
// Created by Mehmet Fatih BAKIR on 19/12/2016.
//

#pragma once
#include <glsl_program.h>

class camera_struct;
class crf;

e2e::GLSLProgram make_preview_shader(const camera_struct& cam, const crf& response);
e2e::GLSLProgram make_merge_shader();