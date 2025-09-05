#pragma once
#include "ProcessJsonData.h"
#include <iostream>


class RegisterResponse : public ProcessJsonData
{
public:
	RegisterResponse();
	virtual std::string process(std::string requestData);
};

