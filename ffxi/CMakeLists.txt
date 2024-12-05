add_executable(ffxi
    main.cpp
)
target_sources(ffxi PUBLIC
    FILE_SET CXX_MODULES
    FILES
    config.cppm
    ffxi.cppm
    game.cppm
    particle_tester.cppm
    scheduler_resources.cppm
    system_dat.cppm
    test_loader.cppm
    vana_time.cppm
    PRIVATE
    game.cpp
    particle_tester.cpp
    scheduler_resources.cpp
    stb.cpp
    system_dat.cpp
)

target_include_directories(ffxi PRIVATE "." )
target_include_directories(ffxi PRIVATE "../external/stb" )
target_compile_features(ffxi PUBLIC cxx_std_23)
target_link_libraries(ffxi PUBLIC lotus-engine)

if (MSVC)
    #these files never change, so they can be copied at generation time rather than build time
    file( COPY ${SDL2_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

file(COPY textures DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(audio)
add_subdirectory(dat)
add_subdirectory(entity)
add_subdirectory(shaders)
