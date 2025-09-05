#ifndef REGISTER_REQUEST
#define REGISTER_REQUEST
#include "ProcessJsonData.h"
#include <iostream>


class RegisterResponse : public ProcessJsonData
{
public:
	RegisterResponse();
	virtual std::string process(std::string requestData);
};

#endif

