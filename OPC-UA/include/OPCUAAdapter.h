#ifndef OPCUA_ADAPTER
#define OPCUA_ADAPTER

#include "OPCUAServer.h"
#include "Config.h"

#include <httplib/httplib.h>
#include <mqtt/async_client.h>

#define MAX_SAMPLE_NODE        (32)

#define NODEID_TYPE_STRING "String"
#define NODEID_TYPE_INT32 "Int32"
class OPCUAAdapter {
public:
	OPCUAAdapter();
	int init();


};
#endif