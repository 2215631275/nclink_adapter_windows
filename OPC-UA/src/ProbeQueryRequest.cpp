#include "ProbeQueryRequest.h"
#include <nlohmann/json.hpp>

extern Config config;
ProbeQueryRequest::ProbeQueryRequest() {}

std::string ProbeQueryRequest::process(std::string requestData)
{
	nlohmann::json requestJsonData = nlohmann::json::parse(requestData);
	std::string guid_string,config_string;
	guid_string = requestJsonData["guid"];
	config_string = config.config["dev_uuid"];
	if (strcmp(guid_string.data(), config_string.data()) == 0) {
		return config.opcconfig.dump();
	}
	nlohmann::json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["code"] = "OK";
	responseJsonData["probe"] = "haha";
	return config.config.dump();
}