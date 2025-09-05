#pragma once
#include "ProcessJsonData.h"
#include "utils.h"
#include <httplib/httplib.h>
#include <iostream>


class QueryRequest : public ProcessJsonData
{
public:
	QueryRequest();
	virtual std::string process(std::string requestData);
};
