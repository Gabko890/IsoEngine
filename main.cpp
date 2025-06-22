#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <reactphysics3d/reactphysics3d.h>

#include "Window.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Utils.hpp"
#include "Renderer.hpp"

#include "Editor.hpp"
#include "TerminalHelper.hpp"

int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    Editor editor(&window);
    editor.ApplyStyle();

    SDL_GL_SetSwapInterval(-1);
    float dtime = 0;
    Uint64 ltime = SDL_GetTicks();

    SDL_HideCursor();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Renderer renderer;
    if (!renderer.Initialize()) {
        SDL_Log("Failed to initialize renderer");
        return -1;
    }

    Scene scene;
    TerminalHelper::scene = &scene;

    scene.AddObject("cube1", "@assets/example_objects/test_cube_color.glb");
    scene.AddObject("cube2", "@assets/example_objects/test_cube_color.glb");
    scene.AddObject("cube3", "@assets/example_objects/test_cube_color.glb");

    scene.MoveObject("cube2", glm::vec3(1.2f, 3.4f, 0.4f));
    scene.MoveObject("cube3", glm::vec3(-0.8f, 9.0f, 0.2f));

    scene.AddObject("ground", "@assets/example_objects/plane_color.glb");
    scene.MoveObject("ground", glm::vec3(0.0f, -3.0f, 0.0f));

    FPSCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
    renderer.SetProjectionMatrix(projection);

    glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor(1.0f);

    TerminalHelper terminal_helper;
    ImTerm::terminal<TerminalHelper> term;

    bool running = true;
    SDL_Event event;

    // === ReactPhysics3D ===
    reactphysics3d::PhysicsCommon physicsCommon;
    reactphysics3d::PhysicsWorld* physicsWorld = physicsCommon.createPhysicsWorld();
    physicsWorld->setGravity(reactphysics3d::Vector3(0.0f, -9.81f, 0.0f));

    // Ground
    SceneObject* ground = scene.GetObject("ground");
    reactphysics3d::Transform groundTransform(
        reactphysics3d::Vector3(ground->position.x, ground->position.y, ground->position.z),
        reactphysics3d::Quaternion::identity());

    reactphysics3d::RigidBody* groundBody = physicsWorld->createRigidBody(groundTransform);
    groundBody->setType(reactphysics3d::BodyType::STATIC);
    auto groundShape = physicsCommon.createBoxShape(reactphysics3d::Vector3(5.0f, 0.1f, 5.0f));
    groundBody->addCollider(groundShape, reactphysics3d::Transform::identity());

    // Cube1
    SceneObject* cube1 = scene.GetObject("cube1");
    reactphysics3d::Transform cubeTransform1(
        reactphysics3d::Vector3(cube1->position.x, cube1->position.y, cube1->position.z),
        reactphysics3d::Quaternion::identity());
    reactphysics3d::RigidBody* cubeBody1 = physicsWorld->createRigidBody(cubeTransform1);
    cubeBody1->setMass(1.0f);
    auto cubeShape1 = physicsCommon.createBoxShape(reactphysics3d::Vector3(1.0f, 1.0f, 1.0f));
    cubeBody1->addCollider(cubeShape1, reactphysics3d::Transform::identity());

    // Cube2
    SceneObject* cube2 = scene.GetObject("cube2");
    reactphysics3d::Transform cubeTransform2(
        reactphysics3d::Vector3(cube2->position.x, cube2->position.y, cube2->position.z),
        reactphysics3d::Quaternion::identity());
    reactphysics3d::RigidBody* cubeBody2 = physicsWorld->createRigidBody(cubeTransform2);
    cubeBody2->setMass(1.0f);
    auto cubeShape2 = physicsCommon.createBoxShape(reactphysics3d::Vector3(1.0f, 1.0f, 1.0f));
    cubeBody2->addCollider(cubeShape2, reactphysics3d::Transform::identity());

    // Cube3
    SceneObject* cube3 = scene.GetObject("cube3");
    reactphysics3d::Transform cubeTransform3(
        reactphysics3d::Vector3(cube3->position.x, cube3->position.y, cube3->position.z),
        reactphysics3d::Quaternion::identity());
    reactphysics3d::RigidBody* cubeBody3 = physicsWorld->createRigidBody(cubeTransform3);
    cubeBody3->setMass(1.0f);
    auto cubeShape3 = physicsCommon.createBoxShape(reactphysics3d::Vector3(1.0f, 1.0f, 1.0f));
    cubeBody3->addCollider(cubeShape3, reactphysics3d::Transform::identity());

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
        if (keystate[SDL_SCANCODE_P]) simulate ^= 1;

        if (simulate) {
            physicsWorld->update(dtime);
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

        // === Sync Physics to Scene ===
        auto syncBody = [](SceneObject* obj, reactphysics3d::RigidBody* body) {
            auto t = body->getTransform();
            obj->position = glm::vec3(t.getPosition().x, t.getPosition().y, t.getPosition().z);

            reactphysics3d::Quaternion q = t.getOrientation();
            glm::quat glmQ(q.w, q.x, q.y, q.z);
            obj->rotation = glm::eulerAngles(glm::normalize(glmQ));
            };

        syncBody(cube1, cubeBody1);
        syncBody(cube2, cubeBody2);
        syncBody(cube3, cubeBody3);

        editor.Add_GUI_Frame([&]() {
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
            });

        renderer.SetLightProperties(lightPos, lightColor);
        scene.RenderScene(renderer, camera);
        editor.Render();
        window.Update();
    }

    physicsCommon.destroyPhysicsWorld(physicsWorld);
    return 0;
}