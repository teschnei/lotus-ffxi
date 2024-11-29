#pragma once

#include "component/actor_component.h"
import lotus;

class ThirdPersonFFXIVCamera
{
public:
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, std::tuple<lotus::Component::CameraComponent*>>>
    Init(lotus::Engine* engine, lotus::Scene* scene, FFXI::ActorComponent* actor_component, lotus::Component::AnimationComponent* animation_component);
};
