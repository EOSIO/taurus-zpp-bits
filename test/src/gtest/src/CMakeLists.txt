
file(GLOB SRCS "*.cc")
add_library(gtest ${SRCS})
target_include_directories(gtest PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/../../../include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)