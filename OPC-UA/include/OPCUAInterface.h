#ifndef OPCUAINTERFACE
#define OPCUAINTERFACE

#include <httplib/httplib.h>
#include <mqtt/async_client.h>
#define MAX_SAMPLE_NODE        (32)

#ifdef _WIN32
#include <open62541/open62541.h>
#elif __linux__
#include <open62541_linux/open62541.h>
#endif

class OPCUAInterface {

public :
	typedef enum {
		opc_string=1,
	    opc_int,
		opc_boolean
	}opcvalue_type;
	typedef struct {
		void* value;
		opcvalue_type type;
	}opcreturn,opcvalue;
	OPCUAInterface();

	std::string SampleTopic;
	typedef struct _SampleDriver
	{
		int sampleNodeNum;
		UA_NodeId sampleNode;

		bool operator < (const _SampleDriver& c) const {

			return sampleNodeNum <c.sampleNodeNum;
		}
	}SampleDriver;
	
	UA_Client* opcclient;
	static void handler_DataChanged(UA_Client* client, UA_UInt32 subId,
		void* subContext, UA_UInt32 monId,
		void* monContext, UA_DataValue* value);
	void addMonitoredItemToVariable(UA_Client* client, UA_NodeId* target);
	opcreturn readFunction(UA_Client* client, UA_NodeId nodeId);
	opcreturn readVariableAttribute(UA_Client* client, int opcnamespace, char* t);
	opcreturn readVariableAttribute(UA_Client* client, int opcnamespace, std::string t);
	opcreturn readVariableAttribute(UA_Client* client, int opcnamespace, int t);

	void SamplePublish(void* cli);
	int SampleManagerStart(mqtt::async_client& cli);
};

#endif // !OPCUAINTERFACE