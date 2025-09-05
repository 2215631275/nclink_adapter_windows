#ifndef PROCESS_JSON_DATA
#define PROCESS_JSON_DATA
#include <iostream>

class ProcessJsonData
{
public:
	ProcessJsonData();
	virtual std::string process(std::string requestData);
};

#endif // !PROCESS_JSON_DATA
