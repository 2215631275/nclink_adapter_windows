#include "ProbeSetRequest.h"
#include <nlohmann/json.hpp>

ProbeSetRequest::ProbeSetRequest() {}
//requestdata "{\"@id\":\"123\",\"probe\":\"{...}\"}"

std::string ProbeSetRequest::process(std::string requestData)
{
	nlohmann::json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["code"] = "OK";
	return responseJsonData.dump();

}