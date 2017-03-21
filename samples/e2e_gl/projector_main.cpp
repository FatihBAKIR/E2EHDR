//FRAMEWORK
#include "glsl_program.h"
#include "quad.h"
#include "Window.h"
#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>

using namespace e2e;

int main()
{
    auto image1 = cv::imread("/Users/goksu/Downloads/office.hdr", -1);
    cv::flip(image1, image1, 0);

//    GLFWwindow* window = glfwCreateWindow(1080, 720, "My Title", nullptr, nullptr);

    e2e::Window w(1080 / 2, 720 / 2);

//    e2e::Window w(1080, 720);
    Texture tex1;
    tex1.createFloat(image1.cols, image1.rows, reinterpret_cast<float*>(image1.ptr()));

    GLSLProgram program_projector;
    program_projector.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
                         "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/projector.vert");
    program_projector.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                         "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/projector.frag");


    GLSLProgram program_LCD;
    program_LCD.attachShader(e2e::GLSLProgram::ShaderType::VERTEX_SHADER,
                                   "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/LCD.vert");
    program_LCD.attachShader(e2e::GLSLProgram::ShaderType::FRAGMENT_SHADER,
                                   "/Users/goksu/Documents/E2EHDR/samples/e2e_gl/shaders/LCD.frag");


    program_LCD.link();
    program_projector.link();

    Quad q_LCD;
    q_LCD.set_scale_factor(1, 1);
    q_LCD.set_position(0, 0);
    q_LCD.create();
    q_LCD.set_texture(tex1);


    Quad q_projector;
    q_projector.set_scale_factor(1, 1);
    q_projector.set_position(0, 0);
    q_projector.create();
    q_projector.set_texture(tex1);


    q_LCD.set_program(program_LCD);
    q_projector.set_program(program_projector);


    bool window = false;
    while(!w.ShouldClose()){
        w.StartDraw();

        auto primary = w.get_window();
        glfwMakeContextCurrent(primary);
        q_LCD.draw();

//        auto secondary = w.get_secondary_window();
//        glfwMakeContextCurrent(secondary);
//        q_projector.draw();

        w.EndDraw();
    }
}