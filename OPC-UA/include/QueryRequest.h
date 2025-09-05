#ifndef QUERY_REQUEST
#define QUERY_REQUEST
#include "ProcessJsonData.h"
#include <iostream>


class QueryRequest : public ProcessJsonData
{
public:
	QueryRequest();
	virtual std::string process(std::string requestData);
};

#endif