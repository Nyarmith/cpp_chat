cmake_minimum_required(VERSION 2.8)

project(cpp_chat)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_subdirectory(client)
add_subdirectory(server)

#TODO : In first fully architected version, add testcases and uncomment this
#enable_testing()
#add_subdirectory(test)
