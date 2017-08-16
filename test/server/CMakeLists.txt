find_package( Boost 1.64 REQUIRED COMPONENTS
    system date_time thread)
#also consider adding: openssl, regex, system

add_executable(cppchat_server main.cpp)

target_link_libraries(cppchat_server ${Boost_LIBRARIES})
