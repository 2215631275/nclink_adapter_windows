#ifndef OPCUA_SERVER
#define OPCUA_SERVER
#include "Config.h"

class OPCUAServer {
public:

	OPCUAServer();
	int init(int port);
};

#endif