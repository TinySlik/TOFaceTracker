project( TO_FACE )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV 3.1.0 REQUIRED )

add_subdirectory(./src/dlib)
INCLUDE_DIRECTORIES(./src)

add_subdirectory(./src/asyncplusplus)
INCLUDE_DIRECTORIES(./src/asyncplusplus/include)

INCLUDE_DIRECTORIES(../include)

aux_source_directory(./src LIB_SRC_LIST)
link_libraries(Async++)
link_libraries(dlib)

ADD_LIBRARY(TOFace STATIC ${LIB_SRC_LIST} )

SET(CROSS_COMPILE 0)

IF(CROSS_COMPILE) 

SET(CMAKE_SYSTEM_NAME Linux)


SET(TOOLCHAIN_DIR "/home/tiny/develop/7251s/ToolChains/stbgcc-4.8-1.5")


set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-g++)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/bin/arm-linux-gcc)
set(GNU_FLAGS "-mfpu=vfp -fPIC")
set(CMAKE_CXX_FLAGS "${GNU_FLAGS} ")
set(CMAKE_C_FLAGS "${GNU_FLAGS}  ")


SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR}
 ${TOOLCHAIN_DIR}/include
 ${TOOLCHAIN_DIR}/lib )


include_directories(/home/tiny/develop/7251s/app/libs/opencv/include)


link_directories(/home/tiny/develop/7251s/app/libs/opencv3.1_from_mr_wang_arm_gnu_cross_compline_11.20/lib)

ELSE()

aux_source_directory(./test TEST_SRC_LIST)

set(TEST_FILE "TOLibConfig.cfg")
set(ORIG_FILE "../test/TOLibConfig.cfg")
add_custom_command(OUTPUT  ${TEST_FILE}
  COMMAND echo "Generating TOLibConfig.cfg file..."
  COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_FILE}  ${TEST_FILE}
  COMMENT  "config file copy"
 )
add_custom_target(config ALL DEPENDS ${TEST_FILE})


add_custom_command(TARGET config
  POST_BUILD 
  COMMAND echo "executing a config command"
  COMMENT "config"
 )

LINK_LIBRARIES(SOIL)
LINK_LIBRARIES(GLEW)
LINK_LIBRARIES(glfw3)
LINK_LIBRARIES(GL)
LINK_LIBRARIES(dl)
LINK_LIBRARIES(GLU)
LINK_LIBRARIES(glut)
LINK_LIBRARIES(X11)
LINK_LIBRARIES(Xxf86vm)
LINK_LIBRARIES(pthread)

link_libraries(TOFace)
add_executable(TOFaceTest ${TEST_SRC_LIST})
target_link_libraries(TOFaceTest ${OpenCV_LIBS} ) 

ENDIF(CROSS_COMPILE)






