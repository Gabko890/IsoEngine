#pragma once

#include <reactphysics3d/reactphysics3d.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class Scene;
struct SceneObject;

struct PhysicsBody {
    reactphysics3d::RigidBody* body = nullptr;
    reactphysics3d::Collider* collider = nullptr;
    glm::vec3 shapeSize = glm::vec3(1.0f);
};

class PhysicsSystem {
public:
    PhysicsSystem();
    ~PhysicsSystem();

    bool Initialize();
    void Update(float deltaTime);
    void Shutdown();

    bool CreateRigidBody(const std::string& objectId, const glm::vec3& position,
        const glm::vec3& rotation, const glm::vec3& shapeSize,
        float mass, bool isStatic = false);

    bool RemoveRigidBody(const std::string& objectId);

    void SyncPhysicsToScene(Scene& scene);
    void SyncSceneToPhysics(const Scene& scene);

    void SetGravity(const glm::vec3& gravity);
    void SetObjectMass(const std::string& objectId, float mass);
    void SetObjectStatic(const std::string& objectId, bool isStatic);

    PhysicsBody* GetPhysicsBody(const std::string& objectId);

private:
    reactphysics3d::PhysicsCommon physicsCommon;
    reactphysics3d::PhysicsWorld* physicsWorld;
    std::unordered_map<std::string, PhysicsBody> physicsBodies;

    bool isInitialized;
};