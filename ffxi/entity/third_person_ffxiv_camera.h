#pragma once

#include "component/actor_component.h"
#include <lotus/entity/component/animation_component.h>
#include <lotus/entity/entity.h>
#include <lotus/renderer/vulkan/renderer.h>

namespace lotus
{
class Scene;
}

class ThirdPersonFFXIVCamera
{
public:
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, std::tuple<lotus::Component::CameraComponent*>>>
    Init(lotus::Engine* engine, lotus::Scene* scene, FFXI::ActorComponent* actor_component,
         lotus::Component::AnimationComponent* animation_component);
};
