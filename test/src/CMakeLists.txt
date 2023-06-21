add_subdirectory(gtest/src)
find_package(Threads)
file(GLOB SRCS "*.cpp")
add_executable(zpp-test ${SRCS})
target_compile_definitions(zpp-test PRIVATE ZPP_BITS_INLINE_MODE=0)
# target_compile_definitions(zpp-test PRIVATE ZPP_BITS_AUTODETECT_MEMBERS_MODE=1)
target_compile_options(zpp-test PRIVATE -ftemplate-backtrace-limit=0 -Werror -Wall -Wsign-compare)
target_link_libraries(zpp-test PRIVATE gtest Threads::Threads)
target_include_directories(zpp-test PRIVATE ${CMAKE_SOURCE_DIR})

add_test(zpp-test COMMAND ./zpp-test 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)