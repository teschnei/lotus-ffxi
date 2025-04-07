module;

#include <coroutine>
#include <lotus/renderer/sdl_inc.h>
#include <optional>

module ffxi;

import :entity.component.equipment_test;

import :entity.actor;
import :entity.component.actor_skeleton;
import lotus;

namespace FFXI
{
EquipmentTestComponent::EquipmentTestComponent(lotus::Entity* _entity, lotus::Engine* _engine, ActorSkeletonComponent& _actor)
    : Component(_entity, _engine), actor(_actor)
{
}

lotus::Task<> EquipmentTestComponent::tick(lotus::time_point time, lotus::duration delta)
{
    if (new_modelid)
    {
        actor.updateEquipLook(*new_modelid);
        new_modelid.reset();
    }
    co_return;
}

bool EquipmentTestComponent::handleInput(lotus::Input* input, const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_UP && event.key.repeat == 0)
    {
        if (event.key.scancode == SDL_SCANCODE_G)
        {
            new_modelid = 0x2000 + 64;
            return true;
        }
        if (event.key.scancode == SDL_SCANCODE_H)
        {
            new_modelid = 0x2000;
            return true;
        }
    }
    return false;
}
} // namespace FFXI
