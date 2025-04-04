module;

#include <chrono>
#include <coroutine>
#include <lotus/renderer/sdl_inc.h>
#include <memory>

module ffxi:entity.component.input.third_person_modern;

import :entity.component.actor;
import glm;
import lotus;

namespace FFXI
{
class ModernThirdPersonInputComponent
    : public lotus::Component::Component<ModernThirdPersonInputComponent, lotus::Component::Before<ActorComponent, lotus::Component::AnimationComponent>>
{
public:
    explicit ModernThirdPersonInputComponent(lotus::Entity*, lotus::Engine* engine, ActorComponent& actor, lotus::Component::AnimationComponent& animation);

    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);
    bool handleInput(lotus::Input*, const SDL_Event&);

private:
    ActorComponent& actor;
    lotus::Component::AnimationComponent& animation_component;
    glm::vec3 moving{};
    glm::vec3 moving_prev{};
    glm::vec3 move_to_rot{};
    constexpr static glm::vec3 step_height{0.f, -0.3f, 0.f};
};
} // namespace FFXI
