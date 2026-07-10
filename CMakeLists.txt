set(CMAKE_BUILD_TYPE "Debug")

cmake_minimum_required(VERSION 3.21)
project(line-drawer-2d-3-in-1-opengl-sdl3-c)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (EMSCRIPTEN)
    set(SDL3_DIR "C:/libs/SDL3-devel-3.4.12-wasm/lib/cmake/SDL3")
elseif (WIN32 AND NOT ANDROID)
    set(SDL3_DIR "C:/libs/SDL3-devel-3.4.12-mingw/lib/cmake/SDL3")
endif()

if(ANDROID)
    function(add_sdl_executable TARGET)
        add_library(${TARGET} SHARED ${ARGN})
    endfunction()
else()
    function(add_sdl_executable TARGET)
        add_executable(${TARGET} ${ARGN})
    endfunction()
endif()

find_package(SDL3 REQUIRED CONFIG)

# Use a conditional for the WIN32 flag
if(WIN32 AND NOT ANDROID)
    add_sdl_executable(app WIN32 ${APP_SOURCES})
else()
    add_sdl_executable(app ${APP_SOURCES})
endif()

target_include_directories(app PRIVATE C:/libs/cglm-0.9.6/include)

if (EMSCRIPTEN)
    target_link_options(app PRIVATE 
        "-Oz"                   # Maximize code size optimization
        "-sMALLOC=emmalloc"     # Use minimal memory allocator
        "-sUSE_SDL=0"           # Disable built-in SDL
        "-sMAX_WEBGL_VERSION=2" # Force WebGL 2.0 support
        "-sMIN_WEBGL_VERSION=2" # Do not fallback to WebGL 1.0
        "-sFULL_ES3=1"          # Enable GLES3 -> WebGL 2 mapping
        "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
    )
endif()

if (WIN32)
    target_include_directories(app PRIVATE C:/libs/glad-0.1.36/opengl-3.3/include)

    target_sources(app
        PRIVATE
        C:/libs/glad-0.1.36/opengl-3.3/src/glad.c
    )

    # Copy the assets folder to the dist folder
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        add_custom_command(TARGET app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_SOURCE_DIR}/assets"
                    "$<TARGET_FILE_DIR:app>/assets"
            COMMENT "Copying assets directory"
        )
    endif()
endif()

if(ANDROID)
    target_link_libraries(app PRIVATE EGL GLESv3)

    # Define the target Android assets directory
    set(ANDROID_ASSETS_DIR "${CMAKE_SOURCE_DIR}/android-project/app/src/main/assets")

    # Copy the assets folder to the Android project structure
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        add_custom_command(TARGET app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_SOURCE_DIR}/assets"
                    "${ANDROID_ASSETS_DIR}"
            COMMENT "Copying assets to Android project structure"
        )
    endif()
endif()

target_sources(app
PRIVATE
    src/file_utils.h
    src/file_utils.c
    src/line_drawer.h
    src/line_drawer.c
    src/main.c
    src/math_helper.h
    src/math_helper.c
    src/shader_program.h
    src/shader_program.c
)

target_link_libraries(app PRIVATE SDL3::SDL3)
