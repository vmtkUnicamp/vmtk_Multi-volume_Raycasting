cmake_minimum_required(VERSION 2.6)

project("VMTK-MVR")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Libraries required
find_package(GLEW)
include_directories(${GLEW_INCLUDE_DIR})
set(LIBS ${GLEW_LIBRARIES})
set(INCL ${GLEW_INCLUDE_DIR})

find_package(GLUT)
include_directories(${GLUT_INCLUDE_DIR})
#link_directories(${GLUT_LIBRARY})#Specify directories in which the linker will look for libraries.
set(LIBS ${LIBS} ${GLUT_LIBRARIES})
set(INCL ${INCL} ${GLUT_INCLUDE_DIR})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

find_package(GDCM)
include_directories(${GDCM_INCLUDE_DIRS})
link_directories(${GDCM_LIBRARY_DIRS})#procura as libs nesse local
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GDCM_CMAKE_DIR}")

set(GDCM_LIBRARIES
    gdcmMSFF gdcmCommon gdcmDICT gdcmDSED gdcmIOD gdcmMEXD gdcmexpat gdcmjpeg8 gdcmjpeg12 gdcmjpeg16 gdcmopenjpeg gdcmzlib gdcmcharls gdcmgetopt socketxx ws2_32)

set(LIBS ${LIBS} ${GDCM_LIBRARIES})
set(INCL ${INCL} ${GDCM_INCLUDE_DIRS})

#Source code
include_directories(include)#adiciona .h da pasta include no projeto
file(GLOB SOURCES "src/*.cpp")#pega todos os arquivos .cpp da pasta src e coloca em SOURCES
file(GLOB HEADERS "*.h" "include/*.h")
file(GLOB FILES "shaders/*.*")


set(LIBS ${LIBS} ${CMAKE_CXX_STANDARD_LIBRARIES})
set(LIBS ${LIBS} winmm)

MESSAGE("INCLUDES")
FOREACH(LETTER ${INCL})
  MESSAGE("<<${LETTER}>>")
ENDFOREACH()

message("LIBS")
FOREACH(LETTER ${LIBS})
  MESSAGE("<<${LETTER}>>")
ENDFOREACH()

add_executable(vmtk-drmn ${SOURCES} ${HEADERS} ${FILES})
target_link_libraries(vmtk-drmn ${LIBS})#link das libs ao fonte


if (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_C_FLAGS "-O0 -g3 -Wall -fmessage-length=0"  CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS "-O0 -g3 -Wall -fmessage-length=0 -static -static-libgcc -static-libstdc++"  CACHE STRING "" FORCE)
endif (CMAKE_COMPILER_IS_GNUCXX)
