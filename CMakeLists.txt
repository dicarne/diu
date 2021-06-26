cmake_minimum_required(VERSION 3.0.0)
project(diu VERSION 0.1.0)

# 在此修改版本号，会影响编译器内部版本号
set(DIU_VERSION 0.1)

include_directories("${PROJECT_BINARY_DIR}")
configure_file("${PROJECT_SOURCE_DIR}/diu/conf.h.in" "${PROJECT_SOURCE_DIR}/diu/conf.h" @ONLY)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -pthread")

include(CTest)
enable_testing()


#find_package(Libevent CONFIG REQUIRED)


add_executable(diu launcher/main.cpp)

file(GLOB headers diu/*.h diu/Engine/*.cpp diu/compiler/*.cpp diu/Engine/Object/*.cpp diu/*.hpp )
add_executable(diuexec diu/main.cpp ${headers})
#target_link_libraries(diuexec ${CMAKE_DL_LIBS} )
#target_link_libraries(diuexec PRIVATE libevent::core libevent::extra libevent::pthreads)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/lib/${DIU_VERSION}") 
add_executable(libdiuexec diu/main.cpp ${headers}) 
#target_link_libraries(libdiuexec PRIVATE libevent::core libevent::extra libevent::pthreads)

#target_link_libraries(libdiuexec ${CMAKE_DL_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
