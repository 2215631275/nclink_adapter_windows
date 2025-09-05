#include <OPCUAAdapter.h>


#include <OPCUAInterface.h> 
#include "CallBack.h"

#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include "ActionListener.h"
#include "FlowCollection.h"

#ifdef _WIN32
#include <Windows.h>
#include <open62541/open62541.h>
#define SLEEP_TIME 1000
#define MONITER_TIMEOUT 1000

#elif __linux__
#include <open62541_linux/open62541.h>
#define SLEEP_TIME 1
#include <unistd.h>
#endif // _WIN32
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
//#include <tools.h>
#include <Utils.h>
#include <iostream>
//#include <windows.h>
#include <map>
#include <cmath>
tools::PathParse pathparse;

Config config;
httplib::Client httpClient;
nlohmann::json requestResponseTopicMap;
nlohmann::json MonitorNodeMap;
OPCUAInterface opcuaInterface;
static UA_NodeId MonitorId[MAX_SAMPLE_NODE];
std::vector<nlohmann::json> flowcollectionChannel;
std::map<std::string, FlowCollection*> flowMap;

OPCUAAdapter::OPCUAAdapter() {};
int OPCUAAdapter::init() {
	

	// 读取配置 windows下当前运行路径是../OPCUA/out/build/x64-Debug
	#ifdef _WIN32
    config.setPath("../../../conf/OPCUA.json");
    #elif __linux__
    config.setPath("./conf/OPCUA.json");
    #endif

	PLOG_DEBUG << "Log Inited";
	PLOG_DEBUG << "dev_uuid:" << config.config["dev_uuid"];
	config.setDevuuid(config.config["dev_uuid"]);
	requestResponseTopicMap = config.config["subscript_publish_map"];

	
	// 初始化日志
	std::string logPath = config.config["log_path"];
	plog::init(plog::debug, logPath.c_str());

	//加载testmodel.json模型文件
	//nlohmann::json opcconfig;
	#ifdef _WIN32
	try {
		std::ifstream file("../../../conf/testmodel.json");
		config.opcconfig = nlohmann::json::parse(file);
	}
	#else
	try {
		std::ifstream file("./conf/testmodel.json");
		config.opcconfig = nlohmann::json::parse(file);
	}
	#endif

	catch (nlohmann::json::parse_error& e) {
		// output exception information
		std::cout << "message: " << e.what() << '\n'
			<< "exception id: " << e.id << '\n'
			<< "byte position of error: " << e.byte << std::endl;
	}
	catch (nlohmann::json::type_error& e)
	{
		// output exception information
		std::cout << "message: " << e.what() << '\n'
			<< "exception id: " << e.id << std::endl;
	}

	std::string s = config.opcconfig["type"];
	std::cout << s << std::endl;
	pathparse.rootParse(config.opcconfig);
	//测试解析结果
	//std::cout << "protocol:" << pathparse.methodParams["protocol"] << std::endl;
	//std::cout << "address:" << pathparse.methodParams["address"] << std::endl;
	//std::cout << "port:" << pathparse.methodParams["port"] << std::endl;
	std::string server_address;
	std::string server_opcip = pathparse.methodParams["address"];
	std::string server_port = pathparse.methodParams["port"];
	server_address = server_opcip + ":" + server_port;
	std::cout << "server_address:" << server_address << std::endl;

	//for (std::map<std::string, std::string>::iterator it = pathparse.idPath.begin(); it != pathparse.idPath.end(); it++) {
	//std::cout << it->first << std::endl;
	//std::cout << it->second << std::endl;
 //   }
	std::cout << "test idnode:" << pathparse.idPath["010302"] << std::endl;
	std::cout << "test identity:" << pathparse.idIdentity["010302"]["namespace"] << std::endl;
	std::cout << "test IdentifierType:" << pathparse.idIdentity["010302"]["IdentifierType"] << std::endl;
	std::cout << "test Identifier:" << pathparse.idIdentity["010302"]["Identifier"] << std::endl;
	if (pathparse.idIdentity["0"].empty()) std::cout << "123456" << std::endl;
	std::cout << "test Identifier:" << pathparse.idIdentity["0"]["Identifier"] << std::endl;


	std::cout << "size:" << pathparse.idIdentity.size() << std::endl;
	std::cout << "sample id:" << pathparse.sampleId[0] << " and " << pathparse.sampleId[1] << std::endl;
	//创建 OPCUA客户端
	UA_Client* opcclient = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(opcclient));
	opcuaInterface.opcclient = opcclient;
	//连接 OPCUA服务端
	//std::string server_ip_string = config.config["opcua_config"]["server_ip"];
	//const char* server_ip = server_ip_string.data();
	const char* server_ip = server_address.data();
	UA_StatusCode retval = UA_Client_connect(opcclient, server_ip);
	if (retval != UA_STATUSCODE_GOOD) {
		PLOG_DEBUG <<"UA_Client_connect error,return code :"<<UA_StatusCode_name(retval);
		UA_Client_delete(opcclient);
		return (int)retval;
	}
	PLOG_DEBUG << "UA_Client_connect completed";

	/*数组测试
	const UA_NodeId nodeId = UA_NODEID_STRING(1, "3DPoint Variable");
	UA_Variant value; 
	UA_Variant_init(&value);
	int ret = UA_Client_readValueAttribute(opcclient, nodeId, &value);
	if (ret == UA_STATUSCODE_GOOD) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Type is: %d\n", value.type->typeId.identifierType);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Identifier is: %d\n", value.type->typeId.identifier);
		std::cout << "value arrayDimensionsSize :" << value.arrayDimensionsSize << std::endl;
		std::cout << "value arrayDimensions[0]:" << value.arrayDimensions[0] << std::endl;
		std::cout << "value arrayDimensions[1]:" << value.arrayDimensions[1] << std::endl;
		std::cout << "value arrayLength:" << value.arrayLength << std::endl;
		double* test = (double*)value.data;
		for (int i = 0; i < value.arrayLength; i++) {
			std::cout << test[i] << std::endl;
		}
				
	}
	*/
	//添加采样监测项
	//测试通过
	//MonitorNodeMap[0] = pathparse.idIdentity["010302"];
	//MonitorNodeMap[1] = pathparse.idIdentity["010303"];
	for (int i = 0; i < pathparse.sampleId.size(); i++) {
		MonitorNodeMap[i] = pathparse.idIdentity[pathparse.sampleId[i]];
	}
	//MonitorNodeMap = config.config["sample_nodes"];
	PLOG_DEBUG << "MonitorNodeMap size :" << MonitorNodeMap.size();
	PLOG_DEBUG << "addMoitoredItemToVariable starting\n";
	for (int i = 0; i < MonitorNodeMap.size(); i++) {
		int namesp= MonitorNodeMap[i]["namespace"];
		std::string str = MonitorNodeMap[i]["IdentifierType"];
		const char* idtype = str.data();
		std::string str1=MonitorNodeMap[i]["Identifier"];
		char* identifier=(char*)malloc((str1.length()+1)*sizeof(char));
		str1.copy(identifier, str1.length(), 0);
		*(identifier + str1.length()) = '\0';

		if (strcmp(idtype, NODEID_TYPE_STRING) == 0) {
			//PLOG_DEBUG <<"type "<< NODEID_TYPE_STRING<< ",Sample MonitorId adding:"<< utf8_s((std::string)str1.c_str());
			MonitorId[i] = UA_NODEID_STRING(namesp, identifier);
			opcuaInterface.addMonitoredItemToVariable(opcclient, &MonitorId[i]);
		}
		else if (strcmp(idtype, NODEID_TYPE_INT32) == 0) {
			PLOG_DEBUG << "type " << NODEID_TYPE_INT32 << ",Sample MonitorId adding:" << identifier;
			int y = atoi(identifier);
			std::cout << "identifier int32:" <<y<< std::endl;
			MonitorId[i] = UA_NODEID_NUMERIC(namesp, y);
			opcuaInterface.addMonitoredItemToVariable(opcclient, &MonitorId[i]);

		}
	}
	PLOG_DEBUG << "addMoitoredItemToVariable Completed";
	//static UA_NodeId targetNodeId = UA_NODEID_STRING(1, "the.answer");
	//opcuaInterface.addMonitoredItemToVariable(opcclient, &targetNodeId);
	//static UA_NodeId targetNodeId1 = UA_NODEID_STRING(1, "the.answer2");
	//opcuaInterface.addMonitoredItemToVariable(opcclient, &targetNodeId1);


	//opcuaInterface.readVariableAttribute(opcclient, 1,"the.answer");

	//UA_Variant value; /* Variants can hold scalar values and arrays of any type */
	//UA_Variant_init(&value);
	//const UA_NodeId nodeId = UA_NODEID_STRING(1, "the.answer");
	//retval = UA_Client_readValueAttribute(opcclient, nodeId, &value);
	//UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32]);
	//if (retval == UA_STATUSCODE_GOOD) {
	//	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Type is: %d\n",value.type->typeId.identifierType);
	//	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Identifier is: %d\n", value.type->typeId.identifier);
	//	UA_Int32 variableValue = *(UA_Int32*)value.data;
	//	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %d\n", variableValue);
	//}

	// MQTT设置账号密码、创建客户端、回调函数
	mqtt::connect_options connOpts(config.config["mqtt_config"]["user_name"], config.config["mqtt_config"]["password"]);
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	mqtt::async_client client(config.config["mqtt_config"]["server_ip"], "1");
	CallBack cb(client, connOpts);
	client.set_callback(cb);
	
	// 连接MQTT，连接完成后自动订阅topic
	try {
		PLOG_DEBUG << "Connecting to the MQTT server...";
		mqtt::token_ptr conntok =client.connect(connOpts, nullptr, cb);
		PLOG_DEBUG << "Waiting connectted to the MQTT server...";
		//等待mqtt连接完毕
		conntok->wait();
		PLOG_DEBUG << "MQTT server connected...";
	}
	catch (const mqtt::exception&) {
		PLOG_DEBUG << "\nERROR: Unable to connect to MQTT";
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< config.config["mqtt_config"]["server_ip"] << "'" << std::endl;
		exit(-1);
	}
	PLOG_DEBUG << "Sample starting";
	opcuaInterface.SampleManagerStart(client);
	PLOG_DEBUG << "SampleChannel:" << pathparse.sampleChannel[0]["id"];

	PLOG_DEBUG << "Sample started";
	for (std::vector<nlohmann::json>::iterator it = pathparse.sampleChannel.begin(); it != pathparse.sampleChannel.end(); it++)
	{
		nlohmann::json test = (*it);
		std::string samplechannel = test["id"];
		PLOG_DEBUG << "samplechannel:" <<samplechannel;
	}
	for (std::vector<nlohmann::json>::iterator it = pathparse.flowcollectionChannel.begin(); it != pathparse.flowcollectionChannel.end(); it++)
	{
		FlowCollection* FlowObj = new FlowCollection((*it), config.config["dev_uuid"], config.config["flowcollection_sample"], config.config["flowcollection_upload"], client);
		FlowObj->createThread();
		flowMap.insert(make_pair(FlowObj->sampleChannelId, FlowObj));
	}
	


	// 主线程定时监测
	while (true)
	{
        #ifdef _WIN32
		Sleep(SLEEP_TIME);
        #else
		sleep(SLEEP_TIME);
        #endif

		//用于设置server监测项回复时间，0表示异步执行
		//需要不断调用，因为一次Publish请求只会触发一次handler_DataChanged()的调用
		UA_Client_run_iterate(opcclient, 0);

		
	}
	UA_Client_delete(opcclient);
	return 0;
}

