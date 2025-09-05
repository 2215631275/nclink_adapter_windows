#include "MTConnectAdapter.h"
#include "CallBack.h"
#include "utils.h"
#include "FlowCollection.h"

#ifdef _WIN32
#include <Windows.h>
#define SLEEP_TIME 1000
#else
#define SLEEP_TIME 1
#include <unistd.h>
#endif // _WIN32
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#ifdef _WIN32
#include <direct.h>
#endif // _WIN32

Config config;
DeviceModel deviceModel;
httplib::Client httpClient;
nlohmann::json requestResponseTopicMap;
std::vector<nlohmann::json> flowcollectionChannel; 
std::map<std::string, FlowCollection*> flowMap;
utils::tools tool;

MTConnectAdapter::MTConnectAdapter(){}

void MTConnectAdapter::init()
{	

	#ifdef _WIN32
	char buffer[256]; 
	char* val = _getcwd(buffer, sizeof(buffer));
	if (val) {
		std::cout << buffer << std::endl;
		//printf("%s\n", buffer);
	}
    #endif // _WIN32

	#ifdef _WIN32
	config.setPath("../../../conf/MTConnect.json");
	#else
	config.setPath("./conf/MTConnect.json");
	#endif

	std::cout << config.config["dev_uuid"] << std::endl;
	requestResponseTopicMap = config.config["subscript_publish_map"];
	std::cout << "Log Initing" << std::endl;

	std::string logPath = config.config["log_path"];
	plog::init(plog::debug, logPath.c_str());

	std::cout << "Log Inited" << std::endl;
	//for(int i = 0;i < config.config["configs"].size();i++)
	//tool.flowcollectionChannel=utils::parseStreams(config.config["dev_uuid"]);

	try
	{
        PLOG_DEBUG << "Request failed, error: ";
		httpClient.set_address(config.config["mtconnect_agent"]["server_ip"]);
		auto res = httpClient.Get("/probe");
        PLOG_DEBUG << "Request failed, error: ";
		if (res->status == 200)
		{
			deviceModel.parse(res->body);
			PLOG_DEBUG << res->body;
			std::cout << res->body << std::endl;
		}
		else
		{
			exit(-1);
		}
	}
	catch (const std::exception& e)
	{
		
		PLOG_DEBUG << "Request failed, error: ";
		std::cerr << "Request failed, error: " << e.what() << '\n';
	}
	
	mqtt::connect_options connOpts(config.config["mqtt_config"]["user_name"], config.config["mqtt_config"]["password"]);
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	mqtt::async_client client(config.config["mqtt_config"]["server_ip"], "1");
	CallBack cb(client, connOpts);
	client.set_callback(cb);

	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		PLOG_DEBUG << "Connecting to the MQTT server...";
		client.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception&) {
		PLOG_DEBUG << "\nERROR: Unable to connect to MQTT";
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< config.config["mqtt_config"]["server_ip"] << "'" << std::endl;
		exit(-1);
	}
	//for (std::vector<nlohmann::json>::iterator it = tool.flowcollectionChannel.begin(); it != tool.flowcollectionChannel.end(); it++)
	//{
		 //std::vector<nlohmann::json>::iterator it = tool.flowcollectionChannel.begin();
		 FlowCollection* FlowObj = new FlowCollection(config.config["dev_uuid"], config.config["flowcollection_sample"], config.config["flowcollection_upload"], client);
		 FlowObj->createThread();
		 flowMap.insert(make_pair(FlowObj->sampleChannelId, FlowObj));
	//}
	while (true)
	{
#ifdef _WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME);
#endif
        nlohmann::json jsonData;
        try
        {
            httpClient.set_address(config.config["mtconnect_agent"]["server_ip"]);
            auto res = httpClient.Get("/current");
            if (res->status != 200)
            {
                PLOG_DEBUG << "Request failed, error: response status = " << res->status; 
            }
            else
            {
                std::string timeStamp = utils::getTimeStamp();
                jsonData = utils::parseStreams(res->body);
                std::string devID = deviceModel.jsonModel["id"];
                for (nlohmann::json device : deviceModel.jsonModel["devices"])
                {
                    for (nlohmann::json channel : device["configs"])
                    {
                        std::string channelID = channel["id"];
                        std::string uri = "Sample/" + devID + "/" + channelID;
                        nlohmann::json responseData;
                        responseData["@id"] = "channel_sample_"+timeStamp;
                        responseData["dev_uuid"] = deviceModel.jsonModel["id"];
                        responseData["id"] = channel["id"];
                        responseData["data"] = nlohmann::json::array({});
                        responseData["beginTime"] = timeStamp;

                        for (nlohmann::json idData : channel["ids"])
                        {
							std::string s_id = idData["id"];
                            nlohmann::json tempData;
                            tempData["data"] = nlohmann::json::array({});
                            //tempData["data"].push_back(jsonData[idData["id"]]["value"]);
							tempData["data"].push_back(jsonData[s_id]["value"]);
                            responseData["data"].push_back(tempData);
                        }
                        mqtt::message_ptr pubmsg = mqtt::make_message(uri, responseData.dump());
                        //std::cout << responseData.dump() << std::endl;
                        //std::cout << uri << std::endl;

                        pubmsg->set_qos(1);
                        client.publish(pubmsg);

                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            PLOG_DEBUG << "Request failed, error: ";
            std::cerr << "Request failed, error: " << e.what() << '\n';
        }
		//mqtt::message_ptr pubmsg = mqtt::make_message("Probe/Query/Request/adapter1", "hello world");
		//pubmsg->set_qos(1);
		//client.publish(pubmsg);
		//std::cout << "  ...OK" << std::endl;
		
	}
}



