target_sources(ffxi PUBLIC
    FILE_SET CXX_MODULES
    FILES
    actor.cppm
    actor_data.cppm
    actor_skeleton_static.cppm
    camera_third_person_classic.cppm
    camera_third_person_modern.cppm
    landscape_entity.cppm
    particle.cppm
    PRIVATE
    actor.cpp
    actor_skeleton_static.cpp
    landscape_entity.cpp
    particle.cpp
)

add_subdirectory(component)
add_subdirectory(loader)
