
cmake_minimum_required(VERSION 3.5)
project(gt_gui)

add_library(gt_gui_driver ${CMAKE_CURRENT_SOURCE_DIR}/driver/gt_gui_driver.c)

file(GLOB_RECURSE SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(gt_gui ${SRC_LIST})

# cmake_minimum_required(VERSION 3.10)

# project(gt_gui VERSION 1.0)

# # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,dynamic_lookup")
# # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined")

# message(STATUS "project name: ${PROJECT_NAME}")

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# # add sub directories
# add_subdirectory(src)
# add_subdirectory(driver)

# !building lib by shell cmd
# cmake --build e:/0_project/23_gui/GT-HMI-Engine/build --config Debug --target gt_gui_driver -- -j1 2>&1
