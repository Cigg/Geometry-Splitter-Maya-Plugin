cmake_minimum_required(VERSION 2.8)
project(geometrysplitter)

if(CMAKE_CONFIGURATION_TYPES)
    message("Multi-configuration generator")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "My multi config types" FORCE)
else()
    message("Single-configuration generator")
endif()

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(SOURCE_FILES "src/SplitFromImageCmd.h" "src/SplitFromImageCmd.cpp")

find_package(Maya REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${MAYA_INCLUDE_DIR})
link_directories(${MAYA_LIBRARY_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES} ${OpenCV_LIBS})

MAYA_PLUGIN(${PROJECT_NAME})
# install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)