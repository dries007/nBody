cmake_minimum_required(VERSION 3.10)
project(nBody C)

set(CMAKE_C_STANDARD 99)

FIND_LIBRARY(OPENCL_LIBRARIES OpenCL
        PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH
        )
GET_FILENAME_COMPONENT(OPENCL_LIB_DIR ${OPENCL_LIBRARIES} PATH)
GET_FILENAME_COMPONENT(_OPENCL_INC_CAND ${OPENCL_LIB_DIR}/../../include ABSOLUTE)

add_definitions(-DFOEFELMAGIC)

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
include_directories(${OPENCL_INCLUDE_DIR})

add_library(ocl_utils src/lib/ocl_utils.h src/lib/ocl_utils.c)
add_library(renderer src/lib/renderer.h src/lib/renderer.c)

include_directories(src)

add_executable(nBody src/main.c)
target_link_libraries(nBody renderer ocl_utils m rt ${OPENCL_LIBRARIES} ${OPENGL_LIBRARY} ${GLUT_LIBRARY})

add_executable(nBodyAuto src/auto_main.c)
target_link_libraries(nBodyAuto ocl_utils m rt ${OPENCL_LIBRARIES})
