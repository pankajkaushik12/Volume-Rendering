cmake_minimum_required(VERSION 3.5)

project(VolumeRendering)
set(TARGET ${CMAKE_PROJECT_NAME})
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

find_package(OpenGL REQUIRED)
set(glew $ENV{GLEW_DIR})
set(glfw $ENV{GLFW_DIR})

set(SOURCES
	"src/main.cpp"
	"src/utils.cpp"
	"depends/imgui/imgui_impl_glfw.cpp"
	"depends/imgui/imgui_impl_opengl3.cpp"
	"depends/imgui/imgui.cpp"
	"depends/imgui/imgui_demo.cpp"
	"depends/imgui/imgui_draw.cpp"
	"depends/imgui/imgui_widgets.cpp"
	)

include_directories(
	${glew}/include
	${glfw}/include
	$ENV{GLM_DIR}
)

add_executable(${TARGET} ${SOURCES})

set(Optional_Library
    glfw3
    glew32s
    ${OPENGL_gl_LIBRARY}
)
cmake_path(SET glfw_lib_dir ${glfw}/lib-vc2022)
cmake_path(SET glew_lib_dir ${glew}/lib/Release/x64)

target_link_directories("${TARGET}" PUBLIC "${glfw_lib_dir}" "${glew_lib_dir}")

target_include_directories(${TARGET} PRIVATE
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/depends/imgui
	)
target_link_libraries(${TARGET} PRIVATE ${Optional_Library})

