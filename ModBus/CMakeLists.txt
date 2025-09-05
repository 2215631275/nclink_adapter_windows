# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("ModBus")

# 包含子项目。
include_directories(${PROJECT_SOURCE_DIR}/include  ${PROJECT_SOURCE_DIR}/thirdparty
 ${PROJECT_SOURCE_DIR}/thirdparty/ndpi/include)
#include_directories(${PROJECT_SOURCE_DIR}/thirdparty/ndpi/include/pcap)

link_directories(${PROJECT_SOURCE_DIR}/thirdparty/mqtt)
#link_directories(${PROJECT_SOURCE_DIR}/thirdparty/ndpi/lib)

if (WIN32)
	set(LINK_LIBS
	paho-mqttpp3-static
	paho-mqtt3a
	paho-mqtt3c
	#Packet
	#wpcap
	#libndpi
	)
	#set(CMAKE_CXX_FLAGS -lws2_32)
	message("WIN32")
else()
set(LINK_LIBS
	libpaho-mqtt3a.so
	libpaho-mqtt3as.so
	libpaho-mqtt3c.so
	libpaho-mqtt3cs.so
	libpaho-mqttpp3.so)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
    set(CMAKE_CXX_FLAGS -pthread)
    find_package(Threads REQUIRED)
	message("UNIX")
endif()

aux_source_directory(src DIR_SRCS)
aux_source_directory(thirdparty/pugixml THIRD_PARTY)
aux_source_directory(thirdparty/modbuslib MODBUS_LIB)
#aux_source_directory(thirdparty/ndpi/lib NDPI_LIB)
ADD_EXECUTABLE(ModBus ${DIR_SRCS} ${THIRD_PARTY} ${MODBUS_LIB})
target_link_libraries(ModBus ${LINK_LIBS})
#target_link_libraries(Modbus "${PROJECT_SOURCE_DIR}/thirdparty/ndpi/lib/libndpi.dll")

#WIN32
#file(COPY "..\\ModBus\\thirdparty\\mqtt\\paho-mqtt3a.dll" DESTINATION "..\\ModBus\\out\\build\\x64-Debug")
#[[
add_library(modbus SHARED ${DIR_SRCS} ${THIRD_PARTY})
target_link_libraries(modbus ${LINK_LIBS})
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build)
install(TARGETS mtconnect LIBRARY DESTINATION modbus)
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/thirdparty/httplib
    ${PROJECT_SOURCE_DIR}/thirdparty/mqtt
    ${PROJECT_SOURCE_DIR}/thirdparty/nlohmann
    ${PROJECT_SOURCE_DIR}/thirdparty/plog
    ${PROJECT_SOURCE_DIR}/thirdparty/pugixml
    DESTINATION modbus
    )
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/
    DESTINATION modbus
    FILES_MATCHING PATTERN *.h)

	]]