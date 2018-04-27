cmake_minimum_required(VERSION 3.10)
project(nBody C)

set(CMAKE_C_STANDARD 99)

FIND_LIBRARY(OPENCL_LIBRARIES OpenCL
        PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH
        )
GET_FILENAME_COMPONENT(OPENCL_LIB_DIR ${OPENCL_LIBRARIES} PATH)
GET_FILENAME_COMPONENT(_OPENCL_INC_CAND ${OPENCL_LIB_DIR}/../../include ABSOLUTE)

find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIR})
message("OpenCL: Incl: ${OPENCL_INCLUDE_DIR} Libs: ${OPENCL_LIBRARIES}")

add_library(common src/lib/ocl_utils.h src/lib/ocl_utils.c src/lib/time_utils.c src/lib/time_utils.h)

include_directories(src)

add_executable(nBody src/main.c)
target_link_libraries(nBody common m ${OPENCL_LIBRARIES})
