#include "ProbeSetRequest.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <plog/Log.h>
ProbeSetRequest::ProbeSetRequest() {}
//requestdata "{\"@id\":\"123\",\"probe\":\"{...}\"}"

std::string ProbeSetRequest::process(std::string requestData)
{
	nlohmann::ordered_json requestJsonData;
	nlohmann::ordered_json responseJsonData;

	try {
		requestJsonData = nlohmann::json::parse(requestData);
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		PLOG_DEBUG << e.what() << std::endl;

	}
	
	responseJsonData["@id"] = requestJsonData["@id"];
	std::fstream  modelFile;
#ifdef _WIN32
	modelFile.open("../../../conf/ModbusModel.json", std::ios::out);
#else
	modelFile.open("./conf/ModbusModel.json", std::ios::out);
#endif
	//ordered_json
	modelFile << requestJsonData["probe"];
	responseJsonData["code"] = "OK";

	modelFile.close();
	return responseJsonData.dump();

}