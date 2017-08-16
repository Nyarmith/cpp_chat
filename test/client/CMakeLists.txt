find_package( Boost 1.64 REQUIRED COMPONENTS
    system date_time thread)
#also consider adding: openssl, regex, system

add_executable(cppchat_client main.cpp)

target_link_libraries(daytime1 ${Boost_LIBRARIES})
