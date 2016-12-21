//FRAMEWORK
#include "gui.h"

//OTHER
#include <imgui.h>

namespace e2e {
    namespace gui {
        //Argument might be given as a structure such as CameraControl in the future.
        void displayCameraControl(float &exposure1, float &exposure2) {
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoResize;
            window_flags |= ImGuiWindowFlags_NoScrollbar;
            window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
            window_flags |= ImGuiWindowFlags_NoCollapse;

            //ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            static bool open1 = true;
            if (!ImGui::Begin("Camera 1", &open1, window_flags)) {
                ImGui::End();
                return;
            }

            static bool auto_exposure1 = false;
            ImGui::Checkbox("Auto Exposure", &auto_exposure1);
            if (!auto_exposure1) {
                ImGui::VSliderFloat("Exposure Time", ImVec2(100.0f, 250.0f), &exposure1, 0.0f, 1.0f);
            }
            ImGui::End();



            //ImGui::SetNextWindowPos(ImVec2(0.0f, 400.0f));
            static bool open2 = true;
            if (!ImGui::Begin("Camera 2", &open2, window_flags)) {
                ImGui::End();
                return;
            }

            static bool auto_exposure2 = false;
            ImGui::Checkbox("Auto Exposure", &auto_exposure2);
            if (!auto_exposure2) {
                ImGui::VSliderFloat("Exposure Time", ImVec2(100.0f, 250.0f), &exposure2, 0.0f, 1.0f);
            }
            ImGui::End();
        }
    }
}