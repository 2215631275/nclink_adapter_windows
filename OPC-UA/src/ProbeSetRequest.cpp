#include "ProbeSetRequest.h"


ProbeSetRequest::ProbeSetRequest() {}

std::string ProbeSetRequest::process(std::string requestData)
{
	std::cout << requestData << std::endl;

	return "test";
}