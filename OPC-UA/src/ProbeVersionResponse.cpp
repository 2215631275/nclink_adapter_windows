#include "ProbeVersionResponse.h"


ProbeVersionResponse::ProbeVersionResponse() {}

std::string ProbeVersionResponse::process(std::string requestData)
{
	std::cout << requestData << std::endl;

	return "test";
}