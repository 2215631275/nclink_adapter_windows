#include "ProbeVersionResponse.h"
#include <nlohmann/json.hpp>

ProbeVersionResponse::ProbeVersionResponse() {}
//requestdata {\"@id\": \"some_mid_001\",\"guid\" : \"dev_uuid\",\"version\" : \"v1.0.1\",\"privateInfo\" : null}"
std::string ProbeVersionResponse::process(std::string requestData)
{
	nlohmann::json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];
	responseJsonData["version"] = requestJsonData["version"];
	responseJsonData["privateInfo"] = requestJsonData["privateInfo"];
	return responseJsonData.dump();

}