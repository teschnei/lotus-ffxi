#pragma once
#include "actor_component.h"
#include <lotus/entity/component/component.h>
#include <lotus/entity/component/particle_component.h>
#include <lotus/entity/component/render_base_component.h>
#include <lotus/light_manager.h>

class SchedulerResources;

namespace FFXI
{
class Generator;
class Keyframe;
class GeneratorLightComponent : public lotus::Component::Component<GeneratorLightComponent>
{
public:
    explicit GeneratorLightComponent(lotus::Entity*, lotus::Engine* engine, FFXI::Generator* generator);
    ~GeneratorLightComponent();

    lotus::Task<> init();
    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);

protected:
    glm::vec3 pos{};
    glm::vec3 dpos{};
    glm::vec4 colour{};
    lotus::LightID light{};

    FFXI::Generator* generator{};
};
} // namespace FFXI
