cmake_minimum_required(VERSION 3.10)

project(l-system)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES 
    src/main.cpp
	src/lParser.cpp
	src/Renderer.cpp
	src/Camera.cpp)



# Find and build libraries

add_executable(${PROJECT_NAME}
	${SOURCES}
)

add_subdirectory(libs)

target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw glad ImGui glm ${CMAKE_DL_LIBS}
)
