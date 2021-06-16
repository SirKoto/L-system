cmake_minimum_required(VERSION 3.10)

project(l-system)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES 
    src/main.cpp)



# Find and build libraries

add_executable(${PROJECT_NAME}
	${SOURCES}
)

find_package(glfw3 3.3 REQUIRED)

add_subdirectory(libs)

target_link_libraries(${PROJECT_NAME}
	glfw glad ImGui glm
)