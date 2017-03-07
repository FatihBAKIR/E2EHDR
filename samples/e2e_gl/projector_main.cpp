//
// Created by Göksu Güvendiren on 07/03/2017.
//

//GL
#define GLEW_STATIC /*Define GLEW_STATIC for static linking*/
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "Window.h"
#include <opencv2/opencv.hpp>

//CPP
#include <chrono>
#include <iostream>

#include "Drawable.h"

using namespace e2e;

int main()
{

    auto image1 = cv::imread("/Users/goksu/Downloads/office.hdr", -1);
    cv::flip(image1, image1, 0);

    e2e::Window w(1080, 720);
    Texture tex1;
    tex1.createFloat(image1.cols, image1.rows, reinterpret_cast<float*>(image1.ptr()));

    GLSLProgram program;
    program.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
                         "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/LCD.vert");
    program.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                         "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/LCD.frag");
    program.link();

    Quad q;
    q.set_scale_factor(1, 1);
    q.set_position(0, 0);
    q.create();
    q.set_texture(tex1);
    q.set_program(program);

    while(!w.ShouldClose()){
        w.StartDraw();
        q.draw();
        w.EndDraw();
    }
}