#pragma once
#include "ProcessJsonData.h"
#include <iostream>


class SetRequest : public ProcessJsonData
{
public:
	SetRequest();
	virtual std::string process(std::string requestData);
	std::string set(std::string requestData);
};
