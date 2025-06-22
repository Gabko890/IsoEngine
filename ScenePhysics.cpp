#include "Scene.hpp"

void Scene::SetObjectPhysicsEnabled(const std::string& id, bool enabled) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->physics.isAffectedByPhysics = enabled;
    }
}

void Scene::SetObjectCollisionEnabled(const std::string& id, bool enabled) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->physics.hasCollision = enabled;
    }
}

void Scene::SetObjectStatic(const std::string& id, bool isStatic) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->physics.isStatic = isStatic;
    }
}

void Scene::SetObjectMass(const std::string& id, float mass) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->physics.mass = mass;
    }
}

void Scene::SetObjectCollisionShape(const std::string& id, const glm::vec3& shapeSize) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->physics.collisionShapeSize = shapeSize;
    }
}

bool Scene::GetObjectPhysicsEnabled(const std::string& id) const {
    const SceneObject* obj = GetObject(id);
    return obj ? obj->physics.isAffectedByPhysics : false;
}

bool Scene::GetObjectCollisionEnabled(const std::string& id) const {
    const SceneObject* obj = GetObject(id);
    return obj ? obj->physics.hasCollision : false;
}

bool Scene::GetObjectStatic(const std::string& id) const {
    const SceneObject* obj = GetObject(id);
    return obj ? obj->physics.isStatic : false;
}

float Scene::GetObjectMass(const std::string& id) const {
    const SceneObject* obj = GetObject(id);
    return obj ? obj->physics.mass : 1.0f;
}

glm::vec3 Scene::GetObjectCollisionShape(const std::string& id) const {
    const SceneObject* obj = GetObject(id);
    return obj ? obj->physics.collisionShapeSize : glm::vec3(1.0f);
}