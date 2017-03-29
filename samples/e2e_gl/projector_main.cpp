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

//			int count;
//			GLFWmonitor** monitors = glfwGetMonitors(&count);
//			assert(count == 2);
//			const GLFWvidmode* sec_mode = glfwGetVideoMode(monitors[1]);

    e2e::Window w_prj(1920, 1080, nullptr, nullptr, "E2EHDRPrimary");
    e2e::Window w_LCD(1920, 1080, nullptr, w_prj.get_window(), "E2EHDRSecondary");

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
    while(!w_prj.ShouldClose() || !w_LCD.ShouldClose()){
        w_prj.StartDraw();
        w_LCD.StartDraw();

        auto primary = w_prj.get_window();
        glfwMakeContextCurrent(primary);
        q_projector.draw();

        auto secondary = w_LCD.get_window();
        glfwMakeContextCurrent(secondary);
        q_LCD.draw();

        w_prj.EndDraw();
        w_LCD.EndDraw();
    }
}