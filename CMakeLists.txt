project(lunasvg_perf_issue)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

option(USE_PLUTO_INSTEAD_OF_LUNA_SVG "Use plutosvg" ON)  # Set this to off to compare with lunasvg

# Add glfw
add_subdirectory(external/glfw)
# Add freetype
add_subdirectory(external/freetype)

if (USE_PLUTO_INSTEAD_OF_LUNA_SVG)
    #
    # Add plutovg and plutosvg
    #
    # plutovg
    file(GLOB plutovg_sources external/plutovg/source/*.c)
    add_library(plutovg STATIC ${plutovg_sources})
    target_include_directories(plutovg PUBLIC external/plutovg/include)
    target_include_directories(plutovg PRIVATE external/plutovg/stb)  # Note: two versions of stb_truetype.h (one in plutovg, one in imgui)
    # plutosvg
    add_library(plutosvg STATIC external/plutosvg/source/plutosvg.c)
    target_include_directories(plutosvg PUBLIC external/plutosvg/source)
    target_compile_definitions(plutosvg PUBLIC PLUTOSVG_HAS_FREETYPE)
    target_link_libraries(plutosvg PUBLIC plutovg freetype)
else()
    # Add lunasvg
    add_subdirectory(external/lunasvg)
endif()

# Add imgui
# - with support for freetype
# - with backend glfw + opengl3
set(imgui_dir external/imgui)
add_library(imgui STATIC
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui_demo.cpp
    ${imgui_dir}/imgui_draw.cpp
    ${imgui_dir}/imgui_widgets.cpp
    ${imgui_dir}/imgui_tables.cpp

    # backend: glfw + opengl3
    ${imgui_dir}/backends/imgui_impl_glfw.cpp
    ${imgui_dir}/backends/imgui_impl_opengl3.cpp

    # support for freetype
    ${imgui_dir}/misc/freetype/imgui_freetype.cpp
    ${imgui_dir}/misc/freetype/imgui_freetype.h
)
target_include_directories(imgui PUBLIC ${imgui_dir} ${imgui_dir}/backends)
target_link_libraries(imgui PUBLIC glfw freetype)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE IMGUI_USE_WCHAR32)  # enable wchar32 to support emoji
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC OpenGL::GL)

if (USE_PLUTO_INSTEAD_OF_LUNA_SVG)
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE_PLUTOSVG)
    target_link_libraries(imgui PUBLIC plutosvg)
else()
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE_LUNASVG)
    target_link_libraries(imgui PUBLIC lunasvg)
endif()

# Add our demo
add_executable(demo demo.cpp)
target_link_libraries(demo PUBLIC imgui)
