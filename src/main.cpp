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

void loadExampleSimpleRng(lParser::LParserInfo* info) {
    info->axiom = "F";
    info->constants = {};
    info->rules = { {"F", 0.333f, "F[+F]F[-F]F"},
                    {"F", 0.333f, "F[+F]F"},
                    {"F", 0.333f, "F[-F]F"},
    };
    info->maxRecursionLevel = 6;
    info->defaultAngle = 20.0f;
    info->defaultThickness = 0.15f;
    info->thicknessReductionFactor = 0.98f;
    info->rngSeed = 15312;
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

void loadExampleTree(lParser::LParserInfo* info) {
    info->axiom = "F(1.5)A";
    info->constants = { {"div", 137.5f}, {"tru", 45.5f}, {"lat", 50.0f} };
    info->rules = { {"A", "F[&(tru)[>B]]/(div)[>A]"},
                    {"B", "F[-(lat)[>C]]/(div)[>A]"},
                    {"C", "F[+(lat)[>B]]/(div)[>A]"} 
    };
    info->maxRecursionLevel = 8;
    info->defaultAngle = 20.0f;
    info->defaultThickness = 0.3f;
    info->thicknessReductionFactor = 0.707f;
}

void loadExampleTreeSmooth(lParser::LParserInfo* info) {
    info->axiom = "F(200)/(45)A";
    info->constants = { {"d1", 94.74f}, {"d2", 132.63f}, {"a", 18.95f} };
    info->rules = { {"A", ">F(50)[&(a)F(50)A]/(d1)[&(a)F(50)A]/(d2)[&(a)F(50)A]"}
    };
    info->maxRecursionLevel = 7;
    info->defaultAngle = 20.0f;
    info->defaultThickness =15.f;
    info->thicknessReductionFactor = 0.707f;
}

void loadExamplePlantNoLeaves(lParser::LParserInfo* info) {
info->axiom = "A";
info->constants = { };
info->rules = { {"A", "[&FL>A]/////[&FL>A]///////[&FL>A]"},
                {"F", "S/////F"},
                {"S", "FL"},
                {"L", "[^^<[-f+f+f-|-f+f+f]]"}
};
info->maxRecursionLevel = 7;
info->defaultAngle = 22.5f;
info->defaultThickness = 0.3f;
info->thicknessReductionFactor = 0.707f;
}

void loadExampleFanTree(lParser::LParserInfo* info) {
    info->axiom = "X";
    info->constants = { };
    info->rules = { {"F", "F>"},
                    {"X", 0.25f, "F-[\\(35)[X]+X]+F[+FX]-X"},
                    {"X", 0.25f, "F-[[X]+X]+F[\\(15)+FX]-X"},
                    {"X", 0.25f, "F-[/(15)[X]+X]+F[+FX]-X"},
                    {"X", 0.25f, "F-[[X]+X]+F[/(30)+FX]-X"}
    };
    info->maxRecursionLevel = 5;
    info->defaultAngle = 25.7f;
    info->defaultThickness = 0.5f;
    info->thicknessReductionFactor = 0.9f;
    info->rngSeed = 15315;
}

void showHelpText() {
    ImGui::TextWrapped("Look at the examples to see how are this concepts applied");
    ImGui::Separator();
    ImGui::Text("List of available operators:\n"
        "\tF\t\tdraw line forward\n"
        "\t+ / -\trotate Yaw by angle\n"
        "\t& / ^\trotate Pitch by angle\n"
        "\t/ / \\\trotate Roll by angle\n"
        "\t> / <\tMultiply/Divide width\n"
        "\t|\t\tGo Rotate by 180º\n"
        "\t[ / ]\tPush/Pop turtle state"
    );
    ImGui::TextWrapped("The first 5 of these operations accept a floating point parameter, in order to "
        "set a custom (non default) value to the operation. This can be done "
        "by setting the value im between parenthesis.\n"
        "For example: F(2)+(45.2)FF, advances 2 units, and rotates 45.22º, Then advances 1 unit two times.");
    ImGui::Separator();
    ImGui::TextWrapped("Instead of using numbers on this parameters, you can use pre-defined constants");
    ImGui::TextWrapped("Constants must start with a letter");
    ImGui::Separator();
    ImGui::TextWrapped("Rules need to be stablished with a single letter identifier, and a mapping");
    ImGui::TextWrapped("Also, all different rules with the same identifier need to have a probability "
        " that adds up to 1. This probability will be sampled from a uniform real distribution");
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
    ImGui::InputInt("RNG Seed", &info->rngSeed);
    ImGui::InputText("Axiom", &info->axiom);

    if (ImGui::TreeNode("Rules")) {
        uint32_t size = (uint32_t)info->rules.size();
        ImGui::InputScalar("##nRules", ImGuiDataType_U32, (void*)&size, &step, nullptr, "%d");
        if (size != (uint32_t)info->rules.size()) {
            info->rules.resize(size);
        }
        const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_BordersOuter |
            ImGuiTableFlags_BordersV |
            ImGuiTableFlags_ContextMenuInBody;
        if (ImGui::BeginTable("TableRules", 3, flags)) {
            ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 2.f);
            ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3.f);
            ImGui::TableSetupColumn("Mapping");
            ImGui::TableHeadersRow();
            for (uint32_t i = 0; i < size; ++i) {
                ImGui::TableNextRow();
                if (i == 0) {
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushItemWidth(-FLT_MIN);
                }
                ImGui::PushID(i);
                ImGui::TableSetColumnIndex(0);
                ImGui::InputText("##idRule", &info->rules[i].id);
                ImGui::TableNextColumn();
                ImGui::InputFloat("##prob", &info->rules[i].probability);
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
    glm::vec3 plant_color = glm::vec3(0.1f, 0.9f, 0.2f);
    // bool show_demo_window = true;
    bool show_help_window = false;
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

       // if (show_demo_window)
       //     ImGui::ShowDemoWindow(&show_demo_window);

        if (ImGui::Begin("L-System")) {
            
            // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            if (ImGui::Button("Help")) {
                show_help_window = true;
            }
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
                if (ImGui::Button("Simple Stochastic")) {
                    loadExampleSimpleRng(&parserInfo);
                    parse = true;
                }
                if (ImGui::Button("Tree")) {
                    loadExampleTree(&parserInfo);
                    parse = true;
                }
                if (ImGui::Button("Tree Smooth")) {
                    loadExampleTreeSmooth(&parserInfo);
                    parse = true;
                }
                if (ImGui::Button("Plant that should have leaves")) {
                    loadExamplePlantNoLeaves(&parserInfo);
                    parse = true;
                }
                if (ImGui::Button("Flater plant")) {
                    loadExampleFanTree(&parserInfo);
                    parse = true;
                }
                ImGui::TreePop();
            }

            if (parse) {
                bool ret = lParser::parse(parserInfo, &parserOut, &errorString);
                if (!ret) {
                    ImGui::OpenPopup("Error PopUp");
                    //std::cerr << "Error when parsing: " << errorString << std::endl;
                    //errorString.clear();
                }

                renderer.setupPrimitivesToRender(parserOut.cylinders);
            }

            if (ImGui::BeginPopup("Error PopUp")) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f,0.1f,0.1f,1.0f });
                ImGui::Text("Parsing Error:");
                ImGui::Separator();
                ImGui::Text(errorString.c_str());
                ImGui::PopStyleColor();
                ImGui::EndPopup();
            }

            ImGui::Separator();
            ImGui::Text("Render Configuration");
            const char* modes[] = { "Lines", "Cylinders", "Cylinders Normal"};
            if (ImGui::BeginCombo("Render Mode", modes[renderMode])) {
                for (uint32_t i = 0; i < 3; ++i) {
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
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::ColorEdit3("Plant Color", (float*)&plant_color)) {
                renderer.setPlantColor(plant_color);
            }
            ImGui::InputFloat("Model Scale", &scale, 0.005f,0.01f, "%.5f");
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

        if (show_help_window) {
            if (ImGui::Begin("Help", &show_help_window)) {
                showHelpText();
            }
            ImGui::End();
        }

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

