#ifndef PROBE_VERSION_RESPONSE
#define PROBE_VERSION_RESPONSE
#include "ProcessJsonData.h"
#include <iostream>


class ProbeVersionResponse : public ProcessJsonData
{
public:
	ProbeVersionResponse();
	virtual std::string process(std::string requestData);
};

#endif