#include "QueryRequest.h"
#include <nlohmann/json.hpp>
#include <nlohmann/detail/exceptions.hpp>
#include <plog/Log.h>
#include <Config.h>
#include <OPCUAInterface.h>
#include <OPCUAAdapter.h>
#include <tools.h>
#include <Utils.h>

QueryRequest::QueryRequest() {}

extern Config config;
extern OPCUAInterface opcuaInterface;
extern tools::PathParse pathparse;
OPCUAInterface::opcreturn opcreturn;
std::string QueryRequest::process(std::string requestData)
{
	nlohmann::json default_return{
	{ "@id",NULL},
	{ "guid",config.dev_uuid},
	};
	nlohmann::json error_return{
    { "@id","null"},
    { "guid",config.dev_uuid},
    { "code", 404 },
    { "reason", "type error"}
	};
	nlohmann::json requestjson;
	/* requestjson example :
	{
		"@id" : "some_mid_x5",
		"guid" : "dev_uuid",
		"ids" : [{"id" : "the.answer", "type" : "String", "namespace" : 1}, { "id" : "the.answer1","type" : "String", "namespace" : 1 }] ,
		"private" : "1"
	}

	*/

	try {
		requestjson = nlohmann::json::parse(requestData);
		if (requestjson["@id"].is_string() && requestjson["guid"].is_string() && requestjson["ids"].is_array()) {
			default_return["@id"] = requestjson["@id"];
			for (int i=0; i<requestjson["ids"].size();i++)
			{
				nlohmann::json opcid = requestjson["ids"][i];
				std::string nodeType_string;
				const char* nodeType;
				if (opcid.contains("id") && opcid.contains("namespace") && opcid.contains("type")) {
					nodeType_string = opcid["type"];
					nodeType = nodeType_string.data();
					if (strcmp(nodeType, NODEID_TYPE_STRING) == 0) {
						std::string str = requestjson["ids"][i]["id"];
						int opcnamespace = requestjson["ids"][i]["namespace"];
						opcreturn = opcuaInterface.readVariableAttribute(opcuaInterface.opcclient, opcnamespace, str);
					}
					else if (strcmp(nodeType, NODEID_TYPE_INT32) == 0) {
						int opcid = requestjson["ids"][i]["id"];
						int opcnamespace = requestjson["ids"][i]["namespace"];
						opcreturn = opcuaInterface.readVariableAttribute(opcuaInterface.opcclient, opcnamespace, opcid);

					}
				}else if (opcid.contains("id")) {
					std::string nodeid = opcid["id"];
					if (!pathparse.idIdentity[nodeid].empty()) {
						nodeType_string = pathparse.idIdentity[nodeid]["IdentifierType"];
						nodeType = nodeType_string.data();
						if (strcmp(nodeType, NODEID_TYPE_STRING) == 0) {
							std::string str = pathparse.idIdentity[nodeid]["Identifier"];
							int opcnamespace = pathparse.idIdentity[nodeid]["namespace"];
							opcreturn = opcuaInterface.readVariableAttribute(opcuaInterface.opcclient, opcnamespace, str);
						}
						else if (strcmp(nodeType, NODEID_TYPE_INT32) == 0) {
							int opcid = pathparse.idIdentity[nodeid]["Identifier"];
							int opcnamespace = pathparse.idIdentity[nodeid]["namespace"];
							opcreturn = opcuaInterface.readVariableAttribute(opcuaInterface.opcclient, opcnamespace, opcid);

						}
					
					}
					else {
						std::cout << "this node is empty" << std::endl;
					}


				}else {
					throw 2;
				}
				switch (opcreturn.type)
				{
				case OPCUAInterface::opc_string: {
					default_return["results"][i]["id"] = opcid["id"];
					default_return["results"][i]["code"] = "OK";
					default_return["results"][i]["value"] = (char*)opcreturn.value;
					break;
				}
				case OPCUAInterface::opc_int: {
					default_return["results"][i]["id"] = opcid["id"];
					default_return["results"][i]["code"] = "OK";
					default_return["results"][i]["value"] = (size_t)opcreturn.value;
					break;

				}
				case OPCUAInterface::opc_boolean: {
					default_return["results"][i]["id"] = opcid["id"];
					default_return["results"][i]["code"] = "OK";
					default_return["results"][i]["value"] = (char*)opcreturn.value;
					break;
				}

				default: {
					default_return["results"][i]["id"] = opcid["id"];
					default_return["results"][i]["code"] = "NG";
					default_return["results"][i]["value"] = "null";
					PLOG_DEBUG << "opcua readVariableValue error,can't find id " << opcid["id"];
					break;
				}
				}

			}
		}else {
			throw 1;
		}
	
	}catch (nlohmann::json::exception& e) {
		PLOG_ERROR<< e.what();
		return error_return.dump();

	}catch (const int errorcode) {
		switch (errorcode)
		{
		case 1: {
			error_return["reason"] = "query struct error,errorcode=1";
			return error_return.dump();
		}
		case 2: {
			error_return["reason"] = "query struct error,errorcode=2";
			return error_return.dump();
		}

		default:
			return "errorcode not known";
		}
	}

	std::string str = default_return.dump();
	return str;
}