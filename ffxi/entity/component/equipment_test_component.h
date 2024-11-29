#pragma once
#include "actor_skeleton_component.h"
#include <lotus/renderer/sdl_inc.h>
#include <optional>

import lotus;

namespace FFXI
{
class EquipmentTestComponent : public lotus::Component::Component<EquipmentTestComponent, lotus::Component::Before<ActorSkeletonComponent>>
{
public:
    explicit EquipmentTestComponent(lotus::Entity*, lotus::Engine* engine, ActorSkeletonComponent& actor);

    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);
    bool handleInput(lotus::Input*, const SDL_Event&);

protected:
    ActorSkeletonComponent& actor;
    std::optional<uint16_t> new_modelid;
};
} // namespace FFXI
