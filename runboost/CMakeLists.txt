cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(boost_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.70 REQUIRED COMPONENTS program_options system)
find_package(Threads REQUIRED)

add_library(boost_chat_protocol INTERFACE)
target_include_directories(boost_chat_protocol INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(boost_chat_protocol INTERFACE Boost::boost)

add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server PRIVATE
    boost_chat_protocol
    Boost::program_options
    Boost::system
    Threads::Threads
)

add_executable(chat_client chat_client.cpp)
target_link_libraries(chat_client PRIVATE
    boost_chat_protocol
    Boost::program_options
    Boost::system
    Threads::Threads
)
