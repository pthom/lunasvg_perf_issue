project(lunasvg_perf_issue)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)


# Add lunasvg
add_subdirectory(external/lunasvg)
# Add glfw
add_subdirectory(external/glfw)
# Add freetype
add_subdirectory(external/freetype)

# Add imgui
# - with support for freetype + lunasvg
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
target_link_libraries(imgui PUBLIC glfw freetype lunasvg)
target_compile_definitions(imgui PUBLIC
    IMGUI_ENABLE_FREETYPE IMGUI_ENABLE_FREETYPE_LUNASVG # Enable freetype with lunasvg
    IMGUI_USE_WCHAR32  # enable wchar32 to support emoji
)
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC OpenGL::GL)

# Add our perf issue test
add_executable(demo_perf_issue_load_font demo_perf_issue_load_font.cpp)
target_link_libraries(demo_perf_issue_load_font PUBLIC imgui)
