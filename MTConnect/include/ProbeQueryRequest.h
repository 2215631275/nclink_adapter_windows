#pragma once
#include "ProcessJsonData.h"
#include "Config.h"
#include "DeviceModel.h"
#include <iostream>


class ProbeQueryRequest : public ProcessJsonData
{
public:
	ProbeQueryRequest();
	virtual std::string process(std::string requestData);
};

