module;

#include <coroutine>
#include <memory>
#include <utility>

module ffxi:entity.camera.third_person_modern;

import :entity.component.camera.third_person;
import :entity.component.input.third_person_modern;
import lotus;

class ThirdPersonModernCamera
{
public:
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, std::tuple<lotus::Component::CameraComponent*>>>
    Init(lotus::Engine* engine, lotus::Scene* scene, FFXI::ActorComponent* actor_component, lotus::Component::AnimationComponent* animation_component);
};

lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, std::tuple<lotus::Component::CameraComponent*>>>
ThirdPersonModernCamera::Init(lotus::Engine* engine, lotus::Scene* scene, FFXI::ActorComponent* actor_component,
                              lotus::Component::AnimationComponent* animation_component)
{
    auto sp = std::make_shared<lotus::Entity>();
    auto cam_c = co_await lotus::Component::CameraComponent::make_component(sp.get(), engine);
    auto dur = co_await FFXI::CameraThirdPersonComponent::make_component(sp.get(), engine, *cam_c, *actor_component, true);
    auto input = co_await FFXI::ModernThirdPersonInputComponent::make_component(sp.get(), engine, *actor_component, *animation_component);
    auto cc = engine->config->renderer.render_mode == lotus::Config::Renderer::RenderMode::Rasterization
                  ? co_await lotus::Component::CameraCascadesComponent::make_component(sp.get(), engine, *cam_c)
                  : nullptr;
    auto p = scene->AddComponents(std::move(cam_c), std::move(dur), std::move(cc), std::move(input));
    co_return std::make_pair(sp, std::get<lotus::Component::CameraComponent*>(p));
}
