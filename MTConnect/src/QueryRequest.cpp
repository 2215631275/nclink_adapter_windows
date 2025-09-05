#include "QueryRequest.h"
#include "Config.h"
#include <nlohmann/json.hpp>
#include <plog/Log.h>

extern Config config;
extern httplib::Client httpClient;
QueryRequest::QueryRequest() {}

std::string QueryRequest::process(std::string requestData){
	nlohmann::json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];
	responseJsonData["values"] = nlohmann::json::array({});
    
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
            jsonData = utils::parseStreams(res->body);
        }
    }
    catch (const std::exception& e)
    {
        PLOG_DEBUG << "Request failed, error: ";
        std::cerr << "Request failed, error: " << e.what() << '\n';
    }
    
    for (nlohmann::json reqID : requestJsonData["ids"])
    {
        std::string s_id = reqID["id"];
        responseJsonData["values"].push_back(jsonData[s_id]);
        //responseJsonData["values"].push_back(jsonData[reqID["id"]]);
    }
	return responseJsonData.dump();
}
