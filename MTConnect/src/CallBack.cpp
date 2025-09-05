#include "CallBack.h"
#include "ProcessJsonData.h"
#include "RegisterResponse.h"
#include "MTConnectAdapter.h"
#include "ProbeQueryRequest.h"
#include "ProbeSetRequest.h"
#include "QueryRequest.h"
#include "SetRequest.h"
#include "ProbeVersionResponse.h"
#include "FlowCollection.h"
#include <plog/Log.h>
#include <nlohmann/json.hpp>

extern Config config;
extern DeviceModel deviceModel;
extern httplib::Client client;
extern nlohmann::json requestResponseTopicMap;

RegisterResponse registerResponse;
ProbeQueryRequest probeQueryRequest;
ProbeSetRequest probeSetRequest;
QueryRequest queryRequest;
SetRequest setRequest;
ProbeVersionResponse probeVersionResponse;
std::map<std::string, ProcessJsonData*> processJsonMap
{
	{"Register/Response/", &registerResponse},
	{"Probe/Query/Request/", &probeQueryRequest},
	{"Probe/Set/Request/", &probeSetRequest},
	{"Query/Request/", &queryRequest},
	{"Set/Request/", &setRequest},
	{"Probe/Version/Response/", &probeVersionResponse}
};
std::map<std::string, std::string> topicMap;


void CallBack::reconnect()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	try {
		cli.connect(_connOpts, nullptr, *this);
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "Error: " << exc.what() << std::endl;
		exit(1);
	}
}

void CallBack::on_failure(const mqtt::token & tok)
{
	std::cout << "Connection attempt failed" << std::endl;
	if (++_nretry > 5)
		exit(1);
	reconnect();
}
/*
void CallBack::connected(const std::string& cause)
{
	std::cout << "\nConnection success" << std::endl;
	std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
		<< "\tfor client " << CLIENT_ID
		<< " using QoS" << QOS << "\n"
		<< "\nPress Q<Enter> to quit\n" << std::endl;

	cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
}
*/

void CallBack::connection_lost(const std::string& cause)
{
	std::cout << "\nConnection lost" << std::endl;
	if (!cause.empty())
		std::cout << "\tcause: " << cause << std::endl;

	std::cout << "Reconnecting..." << std::endl;
	_nretry = 0;
	reconnect();
}

// Callback for when a message arrives.
void CallBack::message_arrived(mqtt::const_message_ptr msg)
{
	/*
	RegisterResponse r;
	ProcessJsonData* j = &r;
	j->process();
	*/
	std::string topic = msg->get_topic();
	std::string topicPrefix = topicMap[topic];
	ProcessJsonData* processData = processJsonMap[topicPrefix];
	std::string responseData = processData->process(msg->to_string());
	if (requestResponseTopicMap.contains(topicPrefix))
	{
		
		std::string devId = config.config["dev_uuid"];
		//std::cout << topicPrefix << std::endl;
		std::string responsePrefix = requestResponseTopicMap[topicPrefix];
		//std::cout << responsePrefix << std::endl;
		std::string responseTopic = responsePrefix + devId;
		mqtt::message_ptr pubmsg = mqtt::make_message(responseTopic, responseData);
		int qos = config.config["publish_method_qos"][responsePrefix];
		pubmsg->set_qos(qos);
		cli.publish(pubmsg);
		std::cout << "  send response" << std::endl;

	}
	// std::cout << "topic: " << topic << ", topicPrefix:" << topicPrefix << '\n';
	PLOG_DEBUG << "Message arrived" << "\ttopic: '" << topic << "\tpayload: '" << msg->to_string();
	std::cout << "Message arrived" << std::endl;
	std::cout << "\ttopic: '" << topic << "'" << std::endl;
	std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
	
}

void CallBack::on_success(const mqtt::token& tok) {}

void CallBack::connected(const std::string& cause)
{
	// ���������е�topic
	std::string devId = config.config["dev_uuid"];
	nlohmann::json subscribeMethods = config.config["subscribe_method_qos"];
	for (auto& data : subscribeMethods.items())
	{
		std::string topicPrefix = data.key();
		int qos = data.value();
		std::string topic = topicPrefix + devId;
		cli.subscribe(topic, qos, nullptr, subListener);
		std::cout << "key: " << topic << ", value:" << qos << '\n';
		PLOG_DEBUG << "key: " << topic << ", value:" << qos << '\n';
		topicMap.insert(std::pair<std::string, std::string>(topic, topicPrefix));
	}
	// �����������ע����Ϣ
	nlohmann::json registerData;
	registerData["@id"] = config.config["dev_uuid"];
	registerData["cli_uuid"] = config.config["dev_uuid"];
	mqtt::message_ptr pubmsg = mqtt::make_message("Register/Request", registerData.dump());
	int qos = config.config["publish_method_qos"]["Register/Request"];
	pubmsg->set_qos(qos);
	cli.publish(pubmsg);

	//����������Ͱ汾������Ϣ 
	nlohmann::json versionData;
	versionData["@id"] = config.config["dev_uuid"];
	versionData["cli_uuid"] = config.config["dev_uuid"];
	versionData["version"] = config.config["version"];
	versionData["privateInfo"] = NULL;
  std::string topic ="Probe/Version/" + devId; 
	pubmsg = mqtt::make_message(topic, versionData.dump());
	qos = config.config["publish_method_qos"]["Probe/Version/"];
	pubmsg->set_qos(qos);
	cli.publish(pubmsg);



}

void CallBack::delivery_complete(mqtt::delivery_token_ptr token) {}
