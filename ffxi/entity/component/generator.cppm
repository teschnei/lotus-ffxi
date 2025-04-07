module;

#include <cmath>
#include <coroutine>
#include <memory>
#include <optional>

module ffxi:entity.component.generator;

import glm;
import lotus;

class SchedulerResources;

namespace FFXI
{
class Generator;
class SchedulerComponent;
class GeneratorComponent : public lotus::Component::Component<GeneratorComponent>
{
public:
    explicit GeneratorComponent(lotus::Entity*, lotus::Engine* engine, FFXI::Generator* generator, lotus::duration duration, FFXI::SchedulerComponent* parent);

    lotus::Task<> init();
    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);

    FFXI::SchedulerComponent* getParent() { return parent; }

protected:
    FFXI::Generator* generator{};
    lotus::duration duration{};
    FFXI::SchedulerComponent* parent{};
    lotus::time_point start_time;
    std::shared_ptr<lotus::Model> model;
    std::unique_ptr<lotus::AudioInstance> sound;
    glm::vec3 base_pos{};
    bool light{false};
    uint64_t generations{0};
    uint64_t index{0};

    enum class Type : uint8_t
    {
        None = 0x00,
        ModelD3MMMB = 0x0B,
        ModelD3A = 0x0E,
        ModelRing = 0x24,
        Sound = 0x3D,
        Light = 0x47
    } type{Type::None};
};
} // namespace FFXI
