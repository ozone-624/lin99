file(GLOB_RECURSE HOST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(lin99 SHARED ${HOST_SOURCES})

target_include_directories(lin99 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)