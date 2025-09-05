# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("OPC-UA")

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
endif()

link_directories(${PROJECT_SOURCE_DIR}/thirdparty/open62541)
set(LINK_LIBS_OPCUA
    open62541.a)
	message("UNIX")
if (WIN32)
	set(LINK_LIBS_OPCUA
	open62541)
	message("WIN32")
endif()

aux_source_directory(src DIR_SRCS)
aux_source_directory(thirdparty/pugixml THIRD_PARTY)

aux_source_directory(thirdparty/open62541 open62541)
aux_source_directory(OPCUAServer SERVER)

ADD_EXECUTABLE(OPC-UA ${DIR_SRCS} ${THIRD_PARTY} ${open62541})
ADD_EXECUTABLE(OPC-UA-Server ${open62541} ${SERVER} ${PROJECT_SOURCE_DIR}/src/Config.cpp)

target_link_libraries(OPC-UA ${LINK_LIBS} ${LINK_LIBS_OPCUA})
target_link_libraries(OPC-UA-Server ${LINK_LIBS} ${LINK_LIBS_OPCUA})

#[[
add_library(opc-ua SHARED ${DIR_SRCS} ${THIRD_PARTY})
target_link_libraries(opc-ua ${LINK_LIBS})
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build)
install(TARGETS opc-ua LIBRARY DESTINATION opc-ua)
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/thirdparty/httplib
    ${PROJECT_SOURCE_DIR}/thirdparty/mqtt
    ${PROJECT_SOURCE_DIR}/thirdparty/nlohmann
    ${PROJECT_SOURCE_DIR}/thirdparty/plog
    ${PROJECT_SOURCE_DIR}/thirdparty/pugixml
    DESTINATION opc-ua
    )
install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/
    DESTINATION opc-ua
    FILES_MATCHING PATTERN *.h)
	]]
