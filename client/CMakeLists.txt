find_package( Boost 1.64 REQUIRED COMPONENTS
    system date_time thread)
#also consider adding: openssl, regex, system

add_executable(cpp_chat_client main.cpp)

target_link_libraries(cpp_chat_client ${Boost_LIBRARIES})
target_link_libraries(cpp_chat_client curses)


