#include "RegisterResponse.h"


RegisterResponse::RegisterResponse() {}

std::string RegisterResponse::process(std::string requestData)
{
	std::cout << requestData << std::endl;

	return "test";
}