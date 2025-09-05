#pragma once
#include "ProcessJsonData.h"
#include <iostream>


class ProbeSetRequest : public ProcessJsonData
{
public:
	ProbeSetRequest();
	virtual std::string process(std::string requestData);
};
