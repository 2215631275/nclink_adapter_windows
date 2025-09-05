#include "SetRequest.h"

SetRequest::SetRequest() {}

std::string SetRequest::process(std::string requestData)
{
	std::cout << requestData << std::endl;

	return "test";
}