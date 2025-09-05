#pragma once
#include "ProcessJsonData.h"
#include <iostream>


class ProbeVersionResponse : public ProcessJsonData
{
public:
	ProbeVersionResponse();
	virtual std::string process(std::string requestData);
};
