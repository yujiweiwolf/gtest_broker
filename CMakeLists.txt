# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(gtest_broker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

message("*****************************************")
find_package(X REQUIRED)
include_directories(${X_INCLUDE_DIRS})
link_directories(${X_LIBRARY_DIRS})

find_package(CORAL REQUIRED)
include_directories(${CORAL_INCLUDE_DIRS})
link_directories(${CORAL_LIBRARY_DIRS})

#find_package(SWORDFISH REQUIRED)
#include_directories(${SWORDFISH_INCLUDE_DIRS})
#link_directories(${SWORDFISH_LIBRARY_DIRS})
message("*****************************************")


aux_source_directory (./src/read_broker/ READ_BROKER)
add_executable(gtest_broker ${READ_BROKER})
target_link_libraries(gtest_broker ${CORAL_LIBRARIES} ${SWORDFISH_LIBRARIES} ${X_LIBRARIES} boost_program_options protobuf protobuf-lite)
