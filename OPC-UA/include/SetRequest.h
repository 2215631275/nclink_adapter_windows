#ifndef SET_REQUEST
#define SET_REQUEST
#include "ProcessJsonData.h"
#include <iostream>


class SetRequest : public ProcessJsonData
{
public:
	SetRequest();
	virtual std::string process(std::string requestData);
};

#endif
