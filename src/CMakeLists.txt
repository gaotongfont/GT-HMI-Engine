project(src)

include_directories(
    ${PROJECT_SOURCE_DIR}/*/*.h
    ${PROJECT_SOURCE_DIR}/*/*/*.h
)

file(GLOB SRC_FILES
    ${PROJECT_SOURCE_DIR}/../driver/*.c
    ${PROJECT_SOURCE_DIR}/*/*.c
    ${PROJECT_SOURCE_DIR}/*/*/*.c
)

message(STATUS "Src code: [${SRC_FILES}]")

add_library(gt_gui_lib_static STATIC ${SRC_FILES})
add_library(gt_gui_lib_dynamic SHARED ${SRC_FILES})

message(STATUS "Install path: [${CMAKE_LIBRARY_OUTPUT_DIRECTORY}]")

set_target_properties(gt_gui_lib_static PROPERTIES LINKER_LANGUAGE C
    OUTPUT_NAME "gt_gui_static"
)
set_target_properties(gt_gui_lib_dynamic PROPERTIES VERSION 1.2 SOVERSION 1)
set_target_properties(gt_gui_lib_dynamic PROPERTIES LINKER_LANGUAGE C
    OUTPUT_NAME "gt_gui_dynamic"
)

set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

