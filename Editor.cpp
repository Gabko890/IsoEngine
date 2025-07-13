#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "Window.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Utils.hpp"
#include "Renderer.hpp"
#include "PhysicsSystem.hpp"

#include "Gui.hpp"
#include "TerminalHelper.hpp"

int editor() {
    Window window("ISO Engine Gui", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);

    Gui gui(&window);
    gui.ApplyStyle();

    SDL_GL_SetSwapInterval(-1);
    
    float dtime = 0;
    Uint64 ltime = SDL_GetTicks();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Renderer renderer;
    if (!renderer.Initialize()) {
        SDL_Log("Failed to initialize renderer");
        return -1;
    }

    Scene scene;
    PhysicsSystem physicsSystem;
    if (!physicsSystem.Initialize()) {
        SDL_Log("Failed to initialize physics system");
        return -1;
    }

    TerminalHelper::scene = &scene;

    //scene.LoadFromFile("scenes/ph_test.scene");

    FPSCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
    renderer.SetProjectionMatrix(projection);

    glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor(1.0f);

    TerminalHelper terminal_helper;
    ImTerm::terminal<TerminalHelper> term;

    bool running = true;
    SDL_Event event;

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        dtime = (currentTime - ltime) / 1000.0f;
        if (dtime <= 0.0f) dtime = 0.001f;
        ltime = currentTime;

        static bool simulate = false;

        const Uint8* keystate = (Uint8*)SDL_GetKeyboardState(nullptr);
        if (keystate[SDL_SCANCODE_W]) camera.MoveForward(dtime, 5.0f);
        if (keystate[SDL_SCANCODE_S]) camera.MoveForward(dtime, -5.0f);
        if (keystate[SDL_SCANCODE_D]) camera.MoveRight(dtime, 5.0f);
        if (keystate[SDL_SCANCODE_A]) camera.MoveRight(dtime, -5.0f);
        if (keystate[SDL_SCANCODE_P]) {
            for (const auto& [id, obj] : scene.GetObjects()) {
                if (obj.physics.hasCollision) {
                    physicsSystem.CreateRigidBody(
                        id,
                        obj.position,
                        obj.rotation,
                        obj.physics.collisionShapeSize,
                        obj.physics.mass,
                        obj.physics.isStatic
                    );
                }
            }

            simulate = true;
        };

        if (simulate) {
            physicsSystem.Update(dtime);
            physicsSystem.SyncPhysicsToScene(scene);
        }

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            static bool move_toggle = false;
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
                move_toggle = true;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT)
                move_toggle = false;
            else if (event.type == SDL_EVENT_MOUSE_MOTION && move_toggle)
                camera.Rotate(-event.motion.yrel * 0.1f, event.motion.xrel * 0.1f);
        }

        gui.Add_GUI_Frame([&]() {
            ImGui::Begin("Scene Objects");
            for (const auto& [id, obj] : scene.GetObjects()) {
                if (ImGui::TreeNode(id.c_str())) {
                    ImGui::Text("Model: %s", obj.modelPath.c_str());

                    glm::vec3 pos = obj.position;
                    if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.0f, 10.0f)) {
                        scene.SetObjectPosition(id, pos);
                    }

                    glm::vec3 rot = obj.rotation;
                    if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -3.14159f, 3.14159f)) {
                        scene.SetObjectRotation(id, rot);
                    }

                    glm::vec3 scale = obj.scale;
                    if (ImGui::SliderFloat3("Scale", glm::value_ptr(scale), 0.01f, 10.0f)) {
                        scene.SetObjectScale(id, scale);
                    }

                    if (ImGui::TreeNode("Physics")) {
                        bool hasCollision = obj.physics.hasCollision;
                        if (ImGui::Checkbox("Has Collision", &hasCollision)) {
                            scene.SetObjectCollisionEnabled(id, hasCollision);
                        }

                        bool isAffectedByPhysics = obj.physics.isAffectedByPhysics;
                        if (ImGui::Checkbox("Affected by Physics", &isAffectedByPhysics)) {
                            scene.SetObjectPhysicsEnabled(id, isAffectedByPhysics);
                        }

                        bool isStatic = obj.physics.isStatic;
                        if (ImGui::Checkbox("Static", &isStatic)) {
                            scene.SetObjectStatic(id, isStatic);
                        }

                        float mass = obj.physics.mass;
                        if (ImGui::SliderFloat("Mass", &mass, 0.1f, 10.0f)) {
                            scene.SetObjectMass(id, mass);
                        }

                        glm::vec3 shapeSize = obj.physics.collisionShapeSize;
                        if (ImGui::SliderFloat3("Collision Shape", glm::value_ptr(shapeSize), 0.1f, 10.0f)) {
                            scene.SetObjectCollisionShape(id, shapeSize);
                        }

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::End();

            ImGui::Begin("Light");
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
            ImGui::ColorEdit3("Color", glm::value_ptr(lightColor));
            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(0, 1080 - 250), ImGuiCond_Always);
            ImGui::SetNextWindowSizeConstraints(ImVec2(1920, 100), ImVec2(1920, 600));
            ImGui::SetNextWindowSize(ImVec2(1920, 200), ImGuiCond_Always);
            term.show();

            ImGui::SetNextWindowPos(ImVec2(0, 1080 - 50), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(1920, 50), ImGuiCond_Always);
            ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            ImGui::Text(" ");
            ImGui::Dummy(ImGui::GetContentRegionAvail());
            ImGui::End();

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New")) {
                        
                    }
                    if (ImGui::MenuItem("Open", "Ctrl+O")) {
                        
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        
                    }

                    ImGui::Separator();

                    if (ImGui::BeginMenu("Recent Files"))
                    {
                        if (ImGui::MenuItem("file1.txt")) {
                            
                        }
                        if (ImGui::MenuItem("file2.txt")) {
                            
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Exit")) {
                        running = false;
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                    if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
                    if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
                    if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help"))
                {
                    if (ImGui::MenuItem("About")) {
                        
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            });

        renderer.SetLightProperties(lightPos, lightColor);
        scene.RenderScene(renderer, camera);
        gui.Render();
        window.Update();
    }

    return 0;
}