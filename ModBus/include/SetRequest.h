#ifndef SET_REQUEST
#define SET_REQUEST
#pragma once
#include "ProcessJsonData.h"
#include <iostream>
#include<nlohmann/json.hpp>


class SetRequest : public ProcessJsonData
{
public:
	SetRequest();
	virtual std::string process(std::string requestData);
	std::string set(std::string requestData);
	std::int32_t setDict(nlohmann::ordered_json valueJson);
	std::int32_t setList(nlohmann::ordered_json valueObject);
	std::int32_t setValue(nlohmann::ordered_json valueObject);
	
};

#endif
