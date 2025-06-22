#include "PhysicsSystem.hpp"
#include "Scene.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

PhysicsSystem::PhysicsSystem() : physicsWorld(nullptr), isInitialized(false) {
}

PhysicsSystem::~PhysicsSystem() {
    Shutdown();
}

bool PhysicsSystem::Initialize() {
    if (isInitialized) {
        return true;
    }

    physicsWorld = physicsCommon.createPhysicsWorld();
    if (!physicsWorld) {
        return false;
    }

    physicsWorld->setGravity(reactphysics3d::Vector3(0.0f, -9.81f, 0.0f));
    isInitialized = true;
    return true;
}

void PhysicsSystem::Update(float deltaTime) {
    if (!isInitialized || !physicsWorld) {
        return;
    }

    physicsWorld->update(deltaTime);
}

void PhysicsSystem::Shutdown() {
    if (physicsWorld) {
        physicsBodies.clear();
        physicsCommon.destroyPhysicsWorld(physicsWorld);
        physicsWorld = nullptr;
    }
    isInitialized = false;
}

bool PhysicsSystem::CreateRigidBody(const std::string& objectId, const glm::vec3& position,
    const glm::vec3& rotation, const glm::vec3& shapeSize,
    float mass, bool isStatic) {
    if (!isInitialized || !physicsWorld) {
        return false;
    }

    if (physicsBodies.find(objectId) != physicsBodies.end()) {
        RemoveRigidBody(objectId);
    }

    glm::quat quat = glm::quat(rotation);
    reactphysics3d::Transform transform(
        reactphysics3d::Vector3(position.x, position.y, position.z),
        reactphysics3d::Quaternion(quat.x, quat.y, quat.z, quat.w)
    );

    reactphysics3d::RigidBody* body = physicsWorld->createRigidBody(transform);
    if (!body) {
        return false;
    }

    if (isStatic) {
        body->setType(reactphysics3d::BodyType::STATIC);
    }
    else {
        body->setType(reactphysics3d::BodyType::DYNAMIC);
        body->setMass(mass);
    }

    auto shape = physicsCommon.createBoxShape(
        reactphysics3d::Vector3(shapeSize.x, shapeSize.y, shapeSize.z)
    );

    reactphysics3d::Collider* collider = body->addCollider(shape, reactphysics3d::Transform::identity());
    if (!collider) {
        physicsWorld->destroyRigidBody(body);
        return false;
    }

    PhysicsBody physicsBody;
    physicsBody.body = body;
    physicsBody.collider = collider;
    physicsBody.shapeSize = shapeSize;

    physicsBodies[objectId] = physicsBody;
    return true;
}

bool PhysicsSystem::RemoveRigidBody(const std::string& objectId) {
    auto it = physicsBodies.find(objectId);
    if (it == physicsBodies.end()) {
        return false;
    }

    if (it->second.body) {
        physicsWorld->destroyRigidBody(it->second.body);
    }

    physicsBodies.erase(it);
    return true;
}

void PhysicsSystem::SyncPhysicsToScene(Scene& scene) {
    for (const auto& [objectId, physicsBody] : physicsBodies) {
        if (!physicsBody.body) continue;

        SceneObject* sceneObject = scene.GetObject(objectId);
        if (!sceneObject) continue;

        reactphysics3d::Transform transform = physicsBody.body->getTransform();
        reactphysics3d::Vector3 pos = transform.getPosition();
        reactphysics3d::Quaternion rot = transform.getOrientation();

        sceneObject->position = glm::vec3(pos.x, pos.y, pos.z);

        glm::quat glmQuat(rot.w, rot.x, rot.y, rot.z);
        sceneObject->rotation = glm::eulerAngles(glm::normalize(glmQuat));
    }
}

void PhysicsSystem::SyncSceneToPhysics(const Scene& scene) {
    for (const auto& [objectId, physicsBody] : physicsBodies) {
        if (!physicsBody.body) continue;

        const SceneObject* sceneObject = scene.GetObject(objectId);
        if (!sceneObject) continue;

        if (physicsBody.body->getType() == reactphysics3d::BodyType::STATIC) {
            glm::quat quat = glm::quat(sceneObject->rotation);
            reactphysics3d::Transform transform(
                reactphysics3d::Vector3(sceneObject->position.x, sceneObject->position.y, sceneObject->position.z),
                reactphysics3d::Quaternion(quat.x, quat.y, quat.z, quat.w)
            );
            physicsBody.body->setTransform(transform);
        }
    }
}

void PhysicsSystem::SetGravity(const glm::vec3& gravity) {
    if (physicsWorld) {
        physicsWorld->setGravity(reactphysics3d::Vector3(gravity.x, gravity.y, gravity.z));
    }
}

void PhysicsSystem::SetObjectMass(const std::string& objectId, float mass) {
    auto it = physicsBodies.find(objectId);
    if (it != physicsBodies.end() && it->second.body) {
        it->second.body->setMass(mass);
    }
}

void PhysicsSystem::SetObjectStatic(const std::string& objectId, bool isStatic) {
    auto it = physicsBodies.find(objectId);
    if (it != physicsBodies.end() && it->second.body) {
        if (isStatic) {
            it->second.body->setType(reactphysics3d::BodyType::STATIC);
        }
        else {
            it->second.body->setType(reactphysics3d::BodyType::DYNAMIC);
        }
    }
}

PhysicsBody* PhysicsSystem::GetPhysicsBody(const std::string& objectId) {
    auto it = physicsBodies.find(objectId);
    return (it != physicsBodies.end()) ? &it->second : nullptr;
}