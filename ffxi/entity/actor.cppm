module;

#include <cstdint>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

module ffxi:entity.actor;

import :entity.component.actor;
import :entity.component.actor_skeleton;
import :dat;
import lotus;

namespace FFXI
{
class Dat;
class ActorSkeletonStatic;
} // namespace FFXI

// main FFXI entity class
class Actor
{
public:
    using InitComponents = std::tuple<lotus::Component::AnimationComponent*, lotus::Component::RenderBaseComponent*, lotus::Component::DeformedMeshComponent*,
                                      lotus::Component::DeformableRasterComponent*, lotus::Component::DeformableRaytraceComponent*, FFXI::ActorComponent*,
                                      FFXI::ActorSkeletonComponent*>;
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, InitComponents>> Init(lotus::Engine* engine, lotus::Scene* scene, uint16_t modelid);
    using InitPCComponents = std::tuple<lotus::Component::AnimationComponent*, lotus::Component::RenderBaseComponent*, lotus::Component::DeformedMeshComponent*,
                                        lotus::Component::DeformableRasterComponent*, lotus::Component::DeformableRaytraceComponent*, FFXI::ActorComponent*,
                                        FFXI::ActorSkeletonComponent*>;
    static lotus::Task<std::pair<std::shared_ptr<lotus::Entity>, InitPCComponents>> Init(lotus::Engine* engine, lotus::Scene* scene,
                                                                                         FFXI::ActorSkeletonComponent::LookData look);
    static size_t GetPCModelDatID(uint16_t modelid, uint8_t race);

protected:
    static lotus::WorkerTask<InitComponents> Load(std::shared_ptr<lotus::Entity> entity, lotus::Engine* engine, lotus::Scene* scene,
                                                  std::shared_ptr<const FFXI::ActorSkeletonStatic>,
                                                  std::variant<FFXI::ActorSkeletonComponent::LookData, uint16_t>,
                                                  std::vector<std::reference_wrapper<const FFXI::Dat>> dats);
    // static lotus::WorkerTask<std::tuple<>> LoadPC(std::shared_ptr<lotus::Entity> entity, lotus::Engine* engine,
    // lotus::Scene* scene,
    //     InitComponents components, FFXI::ActorPCModelsComponent::LookData look,
    //     std::initializer_list<std::reference_wrapper<const FFXI::Dat>> dats);
};
