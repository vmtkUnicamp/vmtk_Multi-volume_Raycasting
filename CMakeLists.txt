cmake_minimum_required(VERSION 2.6)

project("VMTK-MVR")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Libraries required
find_package(GLEW)
include_directories(${GLEW_INCLUDE_DIR})
set(LIBS ${GLEW_LIBRARIES})

find_package(GLUT)
include_directories(${GLUT_INCLUDE_DIR})			  
set(LIBS ${LIBS} ${GLUT_LIBRARIES})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

find_package(GDCM)
include_directories(${GDCM_INCLUDE_DIRS})

set(GDCM_LIBRARIES
    gdcmMSFF gdcmCommon gdcmDICT gdcmDSED gdcmIOD gdcmMEXD gdcmexpat gdcmjpeg8 gdcmjpeg12 gdcmjpeg16 gdcmopenjpeg gdcmzlib gdcmcharls socketxx)

set(LIBS ${LIBS} ${GDCM_LIBRARIES})
	
#Source code
include_directories(include)
file(GLOB SOURCES "src/*.cpp")

set(LIBS ${LIBS} ${CMAKE_CXX_STANDARD_LIBRARIES})
set(LIBS ${LIBS} winmm)

add_executable(vmtk-mvr ${SOURCES})
target_link_libraries(vmtk-mvr ${LIBS})

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_C_FLAGS "-O0 -g3 -Wall -fmessage-length=0"  CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS "-O0 -g3 -Wall -fmessage-length=0 -static -static-libgcc -static-libstdc++"  CACHE STRING "" FORCE)
endif (CMAKE_COMPILER_IS_GNUCXX)

