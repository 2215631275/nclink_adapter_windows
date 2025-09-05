#pragma once
#include <iostream>

class ProcessJsonData
{
public:
	ProcessJsonData();
	virtual std::string process(std::string requestData);
};
