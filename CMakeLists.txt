cmake_minimum_required(VERSION 2.8)

project(true_bitrate)

include_directories("include")

set(SOURCES "src/main.cpp")

## DEPENDENCIES
# manually add kfr
add_subdirectory("3rd/kfr")

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

get_target_property(IMGUI_INCLUDE_DIRS  imgui::imgui    INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(IMPLOT_INCLUDE_DIRS implot::implot  INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(GLAD_INCLUDE_DIRS   glad::glad      INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(GLFW3_INCLUDE_DIRS  glfw            INTERFACE_INCLUDE_DIRECTORIES)

include_directories(${IMGUI_INCLUDE_DIRS})
include_directories(${IMPLOT_INCLUDE_DIRS})
include_directories(${GLAD_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIRS})

message(STATUS ${IMGUI_INCLUDE_DIRS})
message(STATUS ${IMPLOT_INCLUDE_DIRS})
message(STATUS ${GLFW3_INCLUDE_DIRS})
message(STATUS ${GLAD_INCLUDE_DIRS})

add_executable(true_bitrate ${SOURCES})
target_link_libraries(true_bitrate PRIVATE kfr kfr_dft kfr_io glfw glad::glad imgui::imgui implot::implot)
