cmake_minimum_required(VERSION 3.0.0)
project(diu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()

add_executable(diu launcher/main.cpp)

file(GLOB headers diu/*.h diu/Engine/*.cpp diu/compiler/*.cpp diu/Engine/Object/*.cpp diu/*.hpp )
add_executable(diuexec diu/main.cpp ${headers})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
