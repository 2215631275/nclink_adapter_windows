#ifndef QUERY_REQUEST
#define QUERY_REQUEST
#pragma once
#include "ProcessJsonData.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>



class QueryRequest : public ProcessJsonData
{
public:
	QueryRequest();
	virtual std::string process(std::string requestData);
	std::string get(std::string requestData);
	std::int32_t QueryValue(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item);
	std::int32_t QueryDict(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item);
	std::int32_t QueryList(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item);
	static nlohmann::json::array_t QuerySample(nlohmann::json::array_t ids);
};

#endif