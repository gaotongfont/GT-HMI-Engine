
cmake_minimum_required(VERSION 3.0.0)
project(gt_gui)

file(GLOB_RECURSE SRC_LIST "driver" "gt_gui_driver.c")
add_library(gt_gui_driver ${SRC_LIST})

file(GLOB_RECURSE SRC_LIST "src" "*.c") #递归搜索
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
