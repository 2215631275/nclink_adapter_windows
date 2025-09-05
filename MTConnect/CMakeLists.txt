# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("MTConnect")

# 包含子项目。
include_directories("include" "thirdparty")

link_directories(${PROJECT_SOURCE_DIR}/thirdparty/mqtt)
set(LINK_LIBS
	paho-mqtt3a
	paho-mqtt3as
	paho-mqtt3c
	paho-mqtt3cs
	paho-mqttpp3)
	message("UNIX")
if (WIN32)
	set(LINK_LIBS
	paho-mqttpp3-static
	paho-mqtt3a
	paho-mqtt3c)
	message("WIN32")
	#set(CMAKE_C_COMPILER "clang")
	message(${CMAKE_C_COMPILER})
	#set(CMAKE_CXX_COMPILER "clang++")
	message(${CMAKE_CXX_COMPILER})
endif()

aux_source_directory(src DIR_SRCS)
aux_source_directory(thirdparty/pugixml THIRD_PARTY)
ADD_EXECUTABLE(MTConnect ${DIR_SRCS} ${THIRD_PARTY})


target_link_directories(MTConnect PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty/mqtt)
target_link_libraries(MTConnect ${LINK_LIBS})

#[[
add_library(mtconnect SHARED ${DIR_SRCS} ${THIRD_PARTY})
target_link_libraries(mtconnect ${LINK_LIBS})
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build)
install(TARGETS mtconnect LIBRARY DESTINATION mtconnect)
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/thirdparty/httplib
    ${PROJECT_SOURCE_DIR}/thirdparty/mqtt
    ${PROJECT_SOURCE_DIR}/thirdparty/nlohmann
    ${PROJECT_SOURCE_DIR}/thirdparty/plog
    ${PROJECT_SOURCE_DIR}/thirdparty/pugixml
    DESTINATION mtconnect
    )
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/
    DESTINATION mtconnect
    FILES_MATCHING PATTERN *.h)
	]]
