cmake_minimum_required(VERSION 2.8)
project( NVCapSimple ) 
set(CMAKE_BUILD_TYPE Release)
set(_FILE_OFFSET_BITS 64)
set(serial "2.0.0")
set(soserial "2")
#set(CMAKE_C_FLAGS_RELEASE "-Wall -O2")

find_package( OpenCV REQUIRED )
find_package( X11 REQUIRED )
#find_package( Threads REQUIRED )

include_directories("/usr/local/include")
link_directories("/usr/local/lib")

add_executable( ${PROJECT_NAME}_YUV main.cpp Capture.cpp UVCManager_Min.cpp V4LCapture_Min.cpp)
target_link_libraries( ${PROJECT_NAME}_YUV ${OpenCV_LIBS} ${X11_LIBRARIES})
target_compile_definitions( ${PROJECT_NAME}_YUV PUBLIC "-DFORMAT_YUV")

add_executable( ${PROJECT_NAME}_RAW10 main.cpp Capture.cpp UVCManager_Min.cpp V4LCapture_Min.cpp)
target_link_libraries( ${PROJECT_NAME}_RAW10 ${OpenCV_LIBS} ${X11_LIBRARIES})
target_compile_definitions( ${PROJECT_NAME}_RAW10 PUBLIC "-DFORMAT_RAW10")

add_executable( ${PROJECT_NAME}_RAW12 main.cpp Capture.cpp UVCManager_Min.cpp V4LCapture_Min.cpp)
target_link_libraries( ${PROJECT_NAME}_RAW12 ${OpenCV_LIBS} ${X11_LIBRARIES})
target_compile_definitions( ${PROJECT_NAME}_RAW12 PUBLIC "-DFORMAT_RAW12")

#install( TARGETS NVCapSimple_YUV DESTINATION bin)

