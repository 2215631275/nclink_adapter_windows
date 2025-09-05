#include "ProbeQueryRequest.h"
#include <nlohmann/json.hpp>

extern Config config;
extern DeviceModel deviceModel;
ProbeQueryRequest::ProbeQueryRequest() {}

std::string ProbeQueryRequest::process(std::string requestData)
{
	nlohmann::json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["code"] = "OK";
	responseJsonData["probe"] = deviceModel.jsonModel;
	return responseJsonData.dump();
}
