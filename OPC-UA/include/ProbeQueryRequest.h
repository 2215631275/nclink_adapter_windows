#ifndef PROBE_QUERY_REQUEST
#define PROBE_QUERY_REQUEST
#include "ProcessJsonData.h"
#include "Config.h"
#include <iostream>


class ProbeQueryRequest : public ProcessJsonData
{
public:
	ProbeQueryRequest();
	virtual std::string process(std::string requestData);
};

#endif // !PROBE_QUERY_REQUEST
