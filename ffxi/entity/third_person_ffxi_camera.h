#pragma once

#include <lotus/renderer/vulkan/renderer.h>
#include <lotus/entity/entity.h>
#include <lotus/entity/component/animation_component.h>
#include "component/actor_component.h"

namespace lotus
{
    class Scene;
}

class ThirdPersonFFXICamera
{
public:
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, std::tuple<lotus::Component::CameraComponent*>>> Init(lotus::Engine* engine,
        lotus::Scene* scene, FFXI::ActorComponent* actor_component, lotus::Component::AnimationComponent* animation_component);
};
