#ifndef PROBE_SET_REQUEST
#define PROBE_SET_REQUEST
#include "ProcessJsonData.h"
#include <iostream>


class ProbeSetRequest : public ProcessJsonData
{
public:
	ProbeSetRequest();
	virtual std::string process(std::string requestData);
};

#endif

