#include <iostream>
#include <stdio.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "lParser.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void loadExampleAlgae(lParser::LParserInfo* info) {
    info->axiom = "F";
    info->constants = {};
    info->rules = { {"F", "FF>-[F&+F+F]+[+F^-F-F]"} };
    info->maxRecursionLevel = 5;
    info->defaultAngle = 20.0f;
    info->defaultThickness = 0.3f;
    info->thicknessReductionFactor = 0.98f;
}

void showParserInfo(lParser::LParserInfo* info) {
    ImGui::PushID("showParseInfo");
    int32_t step = 1;
    if (ImGui::TreeNode("Constants")) {
        uint32_t size = (uint32_t)info->constants.size();
        ImGui::InputScalar("##nConstants", ImGuiDataType_U32, (void*)&size, &step, nullptr, "%d");
        if (size != (uint32_t)info->constants.size()) {
            info->constants.resize(size);
        }
        const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | 
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV |
            ImGuiTableFlags_ContextMenuInBody;
        if (ImGui::BeginTable("TableConstants", 2, flags)) {
            ImGui::TableSetupColumn("Constant Id");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            for (uint32_t i = 0; i < size; ++i) {
                ImGui::TableNextRow();
                if (i == 0) {
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-FLT_MIN);
                }
                ImGui::PushID(i);
                ImGui::TableSetColumnIndex(0);
                ImGui::InputText("##constId", &info->constants[i].first);
                ImGui::TableNextColumn();
                ImGui::InputFloat("##constDouble", &info->constants[i].second, 1.0);
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // recursion level
    ImGui::InputScalar("Max Recursion", ImGuiDataType_U32, (void*)&info->maxRecursionLevel, &step, nullptr, "%d");
    // Default Angle
    ImGui::InputFloat("Default Angle", &info->defaultAngle);
    ImGui::InputFloat("Default Thickness", &info->defaultThickness);
    ImGui::InputFloat("Thickness reduction factor", &info->thicknessReductionFactor);
    ImGui::InputText("Axiom", &info->axiom);

    if (ImGui::TreeNode("Rules")) {
        uint32_t size = (uint32_t)info->rules.size();
        ImGui::InputScalar("##nConstants", ImGuiDataType_U32, (void*)&size, &step, nullptr, "%d");
        if (size != (uint32_t)info->rules.size()) {
            info->rules.resize(size);
        }
        const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV |
            ImGuiTableFlags_ContextMenuInBody;
        if (ImGui::BeginTable("TableRules", 2, flags)) {
            ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 2.f);
            ImGui::TableSetupColumn("Mapping");
            ImGui::TableHeadersRow();
            for (uint32_t i = 0; i < size; ++i) {
                ImGui::TableNextRow();
                if (i == 0) {
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-FLT_MIN);
                }
                ImGui::PushID(i);
                ImGui::TableSetColumnIndex(0);
                ImGui::InputText("##idRule", &info->rules[i].id);
                ImGui::TableNextColumn();
                ImGui::InputText("##mapRule", &info->rules[i].mapping);
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void mainLoop(GLFWwindow* window) {
    glm::vec3 clear_color = glm::vec3(0.45f, 0.55f, 0.60f);
    bool show_demo_window = true;
    lParser::LParserInfo parserInfo;
    lParser::LParserOut parserOut;
    std::string errorString;
    Renderer renderer;
    Camera camera;
    float scale = 1.0f;
    float cylinderWidthMultiplier = 1.0f;;
    uint32_t renderMode = 0;

    glEnable(GL_MULTISAMPLE);

    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        if (ImGui::Begin("L-System")) {

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            showParserInfo(&parserInfo);
            ImGui::Separator();
            bool parse = false;
            if (ImGui::Button("Parse")) {
                parse = true;
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Examples")) {
                if (ImGui::Button("Algae")) {
                    loadExampleAlgae(&parserInfo);
                    parse = true;
                }
                ImGui::TreePop();
            }

            if (parse) {
                bool ret = lParser::parse(parserInfo, &parserOut, &errorString);
                if (!ret) {
                    std::cerr << "Error when parsing: " << errorString << std::endl;
                    errorString.clear();
                }

                renderer.setupPrimitivesToRender(parserOut.cylinders);
            }

            ImGui::Separator();
            ImGui::Text("Render Configuration");
            const char* modes[] = { "Lines", "Cylinders" };
            if (ImGui::BeginCombo("Render Mode", modes[renderMode])) {
                for (uint32_t i = 0; i < 2; ++i) {
                    const bool is_selected = (renderMode == i);
                    if (ImGui::Selectable(modes[i], is_selected)) {
                        renderMode = i;
                    }
                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            ImGui::InputFloat("Model Scale", &scale, 0.01f,0.0f, "%.5f");
            if (ImGui::InputFloat("Cylinder width scale", &cylinderWidthMultiplier, 0.01f, .0f, "%.3f", 0)) {
                renderer.setCylinderScale(cylinderWidthMultiplier);
            }
            bool v;
            if(ImGui::Checkbox("Antialiasing", &v)) {
                if (v) {
                    glEnable(GL_MULTISAMPLE);
                }
                else {
                    glDisable(GL_MULTISAMPLE);
                }
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            if (ImGui::TreeNode("Camera Info")) {
                camera.renderImGui();
                ImGui::TreePop();
            }

        }
        ImGui::End();

        // Camera update
        camera.update();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        renderer.render(glm::scale(camera.getProjView(), glm::vec3(scale)), renderMode);
        glDisable(GL_DEPTH_TEST);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    } // while
} // main Loop

int main() {
    // Setup window
    GLFWwindow* window;
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;

        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(1280, 720, "L-systems project", NULL, NULL);
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
        bool err = gladLoadGL() == 0;
        if (err)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            return 1;
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        // disable .ini file
        io.IniFilename = nullptr;
        io.FontAllowUserScaling = true; // zoom wiht ctrl + mouse wheel 

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
         // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    mainLoop(window);

    // Cleanup
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    return 0;
}

