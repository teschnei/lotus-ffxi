module;

#include <algorithm>
#include <coroutine>
#include <lotus/renderer/sdl_inc.h>
#include <memory>

module ffxi:entity.component.camera.third_person;

import :entity.component.actor;
import glm;
import lotus;

namespace FFXI
{
class CameraThirdPersonComponent
    : public lotus::Component::Component<CameraThirdPersonComponent, lotus::Component::Before<lotus::Component::CameraComponent, ActorComponent>>
{
public:
    explicit CameraThirdPersonComponent(lotus::Entity*, lotus::Engine* engine, lotus::Component::CameraComponent& camera, ActorComponent& target,
                                        bool right_click_face);

    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);
    bool handleInput(lotus::Input*, const SDL_Event&);

    void setDistance(float _distance);
    float getDistance() const { return distance; }
    void swivel(float x_offset, float y_offset);

protected:
    lotus::Component::CameraComponent& camera;
    ActorComponent& target;
    // camera
    float rot_x{};
    float rot_y{};
    glm::quat rot{1.f, 0.f, 0.f, 0.f};
    float distance{7.f};

    // input
    enum class Look
    {
        NoLook,
        LookCamera,
        LookBoth
    };
    Look look{Look::NoLook};
    Look right_click{Look::LookBoth};
    glm::ivec2 look_pos{};
};
} // namespace FFXI
