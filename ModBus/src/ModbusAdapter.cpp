#include "ModbusAdapter.h"
#include "CallBack.h"
#ifdef _WIN32
#include <Windows.h>
#define SLEEP_TIME 1000
#else
#define SLEEP_TIME 1
#include <unistd.h>
#endif // _WIN32
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <ModbusInterface.h>
#include <Utils.h>
#include <cstring>
#include "Sample.h"
#include "FlowCollection.h"
#ifdef _WIN32
#include <direct.h>

#endif // _WIN32

Config config;
DeviceModel deviceModel;
httplib::Client httpClient;
nlohmann::json requestResponseTopicMap;
nlohmann::ordered_json model;//ģ���ļ���������
std::map<std::string, std::string> idPathMap;//<dataItem id,json����ָ��>
std::vector<nlohmann::json> sampleChannel; //ģ���ļ��еĲ���ͨ������
std::vector<nlohmann::json> flowcollectionChannel; //ģ���ļ��еĲ���ͨ������
std::map<std::string, Sample*> sampleMap;//<sample channel,��������ָ��>
std::map<std::string, FlowCollection*> flowMap;
modbus::modbus_tcp modbustcp;//modbus���ӹ������?

ModbusAdapter::ModbusAdapter(){}
//路径需要修�?
void ModbusAdapter::init()
{	
	// ��ȡ���� ��ǰ����·����../ModBus/out/build/x64-Debug
	//config.setPath("D:\\Microsoft Visual Studio\\PROJECT\\ModBus\\conf\\Modbus.json");//路径需要修�?
#ifdef _WIN32
	config.setPath("../../../conf/Modbus.json");
#else
	config.setPath("./conf/Modbus.json");
#endif

	// ��ʼ����־
	std::string logPath = config.config["log_path"];
	plog::init(plog::debug, logPath.c_str());

	PLOG_DEBUG << "Log Inited";
	PLOG_DEBUG << "dev_uuid:" << config.config["dev_uuid"];
	//config.setDevuuid(config.config["dev_uuid"]);
	requestResponseTopicMap = config.config["subscript_publish_map"];

	
	//���� MODBUS�ͻ���
	std::string server_ip_string = config.config["modbus_server"]["server_ip"];
	std::string server_port_string = config.config["modbus_server"]["server_port"];
	const short server_port = atoi((char*)server_port_string.c_str());
	try {
		modbustcp.modbustcp_connect(server_ip_string, server_port);  //����һ��modbus_tcp����
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		return ;
	}

	//����model�ļ�
	//std::string modelPath = config.config["model_path"];
	PLOG_DEBUG << "driver module initing..";
#ifdef _WIN32
	std::fstream file("../../../conf/ModbusModel.json");
#else
	std::fstream file("./conf/ModbusModel.json");
#endif
	model = nlohmann::ordered_json::parse(file);
	file.close();
	tools::PathParse dataitemMap;
	dataitemMap.rootParse(model);
	idPathMap = dataitemMap.idPath;
	sampleChannel = dataitemMap.sampleChannel;
	flowcollectionChannel = dataitemMap.flowcollectionChannel;
	PLOG_DEBUG << "driver module inited !";
	// MQTT�����˺����롢�����ͻ��ˡ��ص�����

	PLOG_DEBUG << "MQTT module initing..";
	mqtt::connect_options connOpts(config.config["mqtt_config"]["user_name"], config.config["mqtt_config"]["password"]);
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	mqtt::async_client client(config.config["mqtt_config"]["server_ip"], "1");
	CallBack cb(client, connOpts);
	client.set_callback(cb);

	// ����MQTT��������ɺ��Զ�����topic
	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		PLOG_DEBUG << "Connecting to the MQTT server...";
		client.connect(connOpts, nullptr, cb);
		PLOG_DEBUG << "MQTT server connected..";
	}
	catch (const mqtt::exception&) {
		PLOG_DEBUG << "\nERROR: Unable to connect to MQTT";
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< config.config["mqtt_config"]["server_ip"] << "'" << std::endl;
		exit(-1);
	}
	PLOG_DEBUG << "MQTT module inited !";
	//���ݲ������󴴽�ʵ��
	//int num = samplechannel.size();
	//
	for (std::vector<nlohmann::json>::iterator it = sampleChannel.begin(); it != sampleChannel.end(); it++)
	{
		Sample* sampleObj = new Sample((*it), config.config["dev_uuid"], client);
	 	sampleObj->createThread();
	 	sampleMap.insert(make_pair(sampleObj->sampleChannelId, sampleObj));
	}
	for (std::vector<nlohmann::json>::iterator it = flowcollectionChannel.begin(); it != flowcollectionChannel.end(); it++)
	{
		FlowCollection* FlowObj = new FlowCollection((*it), config.config["dev_uuid"], config.config["flowcollection_sample"], config.config["flowcollection_upload"], client);
		FlowObj->createThread();
		flowMap.insert(make_pair(FlowObj->sampleChannelId, FlowObj));
	}
	//���̶߳�ʱhttp����sample���ݣ���װ��json���͵�topic
	while (true)
	{
#ifdef _WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME);
#endif

		//mqtt::message_ptr pubmsg = mqtt::make_message("Probe/Query/Request/adapter1", "hello world");
		//pubmsg->set_qos(1);
		//client.publish(pubmsg);
		//std::cout << "  ...OK" << std::endl;
		
	}
	
}



