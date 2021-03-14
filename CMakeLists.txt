cmake_minimum_required(VERSION 3.9)
project(test-cpp-server)

set(CMAKE_CXX_STANDARD 17)

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS
    SSL Crypto
)

add_executable(test-cpp-server test-cpp-server.cpp)
target_link_libraries(test-cpp-server
   	PRIVATE cpprestsdk::cpprest
    PRIVATE OpenSSL::SSL
    PRIVATE OpenSSL::Crypto
)

install(TARGETS test-cpp-server DESTINATION bin)
