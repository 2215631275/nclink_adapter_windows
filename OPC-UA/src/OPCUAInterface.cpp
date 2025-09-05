#include <OPCUAInterface.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <cstdarg>
#include <nlohmann/json.hpp>
//#define BOOL_TO_STR(bool_expr) (bool_expr) ? "true" :"false"
const char* const  BOOL_TO_STR[2] = { "false","true" };
static OPCUAInterface::SampleDriver sampleDriver[MAX_SAMPLE_NODE];
int sampleDriverNums = 0;

std::map<OPCUAInterface::SampleDriver, OPCUAInterface::opcvalue> SampleTestmap;

OPCUAInterface::OPCUAInterface() {};

UA_NodeId idArr[2] = { UA_NODEID_STRING(1, (char*)"the.answer"), UA_NODEID_STRING(1, (char*)"the.answer2") };

void OPCUAInterface::handler_DataChanged(UA_Client* client, UA_UInt32 subId,
    void* subContext, UA_UInt32 monId,
    void* monContext, UA_DataValue* value)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Received Notification");

    UA_NodeId* ptr = (UA_NodeId*)monContext;
    std::map<OPCUAInterface::SampleDriver, OPCUAInterface::opcvalue>::iterator strmap_iter = SampleTestmap.begin();
    for (; strmap_iter != SampleTestmap.end(); strmap_iter++) {
        if (UA_NodeId_equal(ptr, &strmap_iter->first.sampleNode)) {
            if (value->value.type == &UA_TYPES[UA_TYPES_INT32]) {
                std::cout << "type is int32" << std::endl;
                UA_Int32 currentValue = *(UA_Int32*)(value->value.data);
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "SubId:%u, MonId:%u, Current Value: %d",
                    subId, monId, currentValue);
                strmap_iter->second.value = (void*)currentValue;
                std::cout << strmap_iter->first.sampleNodeNum << ' ' << std::endl;
                std::cout << "type:" << strmap_iter->second.type << std::endl;
                std::cout << "value:" << (size_t)strmap_iter->second.value << std::endl;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_STRING]) {
                UA_String variableValue = *(UA_String*)(value->value.data);
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %.*s", variableValue.length, variableValue.data);

                //将变量节点的unsigned char*类型的值转化为char*类型,变量节点指针未设置终止符
                char* cr = new char[variableValue.length + 1];
                memcpy(cr, variableValue.data, variableValue.length);
                cr[variableValue.length] = 0;

                strmap_iter->second.value = (void*)cr;
                std::cout << strmap_iter->first.sampleNodeNum << ' ' << std::endl;
                std::cout << "type:" << strmap_iter->second.type << std::endl;
                std::cout << "value:" << (char*)strmap_iter->second.value << std::endl;
            }
            else if (value->value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                UA_Boolean variableValue = *(UA_Boolean*)value->value.data;
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "SubId:%u, MonId:%u, Current Value: %s",
                    subId, monId, BOOL_TO_STR[variableValue]);

                strmap_iter->second.value = (void*)BOOL_TO_STR[variableValue];
                std::cout << strmap_iter->first.sampleNodeNum << ' ' << std::endl;
                std::cout << "type:" << strmap_iter->second.type << std::endl;
                std::cout << "value:" << (char*)strmap_iter->second.value << std::endl;
            }
            else {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "not done");
            }

        }
    
    }
}


void OPCUAInterface::addMonitoredItemToVariable(UA_Client* client, UA_NodeId* target)
{
    /* Create a subscription */
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
        NULL, NULL, NULL);

    UA_UInt32 subId = response.subscriptionId;
    if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
    {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Create subscription succeeded, id %u\n", subId);
    }

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(*target);
    UA_MonitoredItemCreateResult monResponse =
        UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
            UA_TIMESTAMPSTORETURN_BOTH,
            monRequest, (void*)target,
            handler_DataChanged, NULL);
    if (monResponse.statusCode == UA_STATUSCODE_GOOD)
    {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Monitoring 'the.answer', id %u\n",
            monResponse.monitoredItemId);
        sampleDriver[sampleDriverNums].sampleNode = *target;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "sampleDriverNums %d\n",
            sampleDriverNums);
        OPCUAInterface::SampleDriver driver1 = { sampleDriverNums,*target };
        SampleTestmap.insert(std::pair<OPCUAInterface::SampleDriver, OPCUAInterface::opcvalue>(driver1, readFunction(client, *target)));
        sampleDriverNums++;
        PLOG_DEBUG << "MonitorId adding completed!\n";
    }
    else {
        PLOG_DEBUG << "MonitorId adding error!";
    }

}

void OPCUAInterface::SamplePublish(void* cli) {


}
void* sample(void* arg) {
    while (true) {
        mqtt::async_client* cli = (mqtt::async_client*)arg;
        nlohmann::json j1;
        std::map<OPCUAInterface::SampleDriver, OPCUAInterface::opcvalue>::iterator strmap_iter = SampleTestmap.begin();
        
        for (int i = 0; strmap_iter != SampleTestmap.end(); strmap_iter++,i++)
        {
                
                switch (strmap_iter->first.sampleNode.identifierType) {
                case UA_NODEIDTYPE_NUMERIC: {
                    int cr = strmap_iter->first.sampleNode.identifier.numeric;
                    j1["sample"][i]["node"] = cr;
                    break;
                }
                case UA_NODEIDTYPE_STRING:{
                    char* cr = new char[strmap_iter->first.sampleNode.identifier.string.length + 1];
                    memcpy(cr, strmap_iter->first.sampleNode.identifier.string.data, strmap_iter->first.sampleNode.identifier.string.length);
                    cr[strmap_iter->first.sampleNode.identifier.string.length] = 0;
                    j1["sample"][i]["node"] = cr;
                    break;
                }
                //GUID类型好像暂时用不到
                case UA_NODEIDTYPE_GUID: {
                    break;
                }
                case UA_NODEIDTYPE_BYTESTRING: {
                    char* cr = new char[strmap_iter->first.sampleNode.identifier.string.length + 1];
                    memcpy(cr, strmap_iter->first.sampleNode.identifier.string.data, strmap_iter->first.sampleNode.identifier.string.length);
                    cr[strmap_iter->first.sampleNode.identifier.string.length] = 0;
                    j1["sample"][i]["node"] = cr;
                    break;
                }
                default:break;
                } 

                j1["sample"][i]["type"] = strmap_iter->second.type;
                switch (strmap_iter->second.type)
                {
                case OPCUAInterface::opc_string:{
                    j1["sample"][i]["value"] = (char*)strmap_iter->second.value;
                    break;
                }
                case OPCUAInterface::opc_int: {
                    j1["sample"][i]["value"] = (size_t)strmap_iter->second.value;
                    break;
                }
                case OPCUAInterface::opc_boolean: {
                    j1["sample"][i]["value"] = (char*)strmap_iter->second.value;
                    break;

                }
                default:
                    j1["sample"][i]["value"] = "This type has not been completed";
                    break;
                }

        }
        mqtt::message_ptr pubmsg = mqtt::make_message("Sample/ID_HNC81/sample_channel", j1.dump());
        int qos = 0;
        pubmsg->set_qos(qos);
        (*cli).publish(pubmsg);
        #ifdef _WIN32
        Sleep(3000);
        #elif __linux__
        sleep(3);
        #endif
    }
}
int OPCUAInterface::SampleManagerStart(mqtt::async_client& cli)
{
    int flag = 1;
    #ifdef _WIN32
    SetConsoleOutputCP(65001);

    HANDLE hHandle1 =CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sample, (void*)&cli, 0, NULL);
    #elif __linux__
    pthread_t tid;
    int err = pthread_create(&tid, NULL, sample, (void*)&cli);
    #endif // _WIN32

   
    return 0;
}

OPCUAInterface::opcreturn OPCUAInterface::readFunction(UA_Client* client, UA_NodeId nodeId) {
    OPCUAInterface::opcreturn opcreturn = {};
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Variant_init(&value);
    int retval = UA_Client_readValueAttribute(client, nodeId, &value);
    UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32]);
    if (retval == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Type is: %d\n", value.type->typeId.identifierType);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Identifier is: %d\n", value.type->typeId.identifier);
    }
    else {
        PLOG_ERROR << "readVariableAttribute Error! return:" << UA_StatusCode_name(retval);
        return opcreturn;
    }
    if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        UA_Int32 variableValue = *(UA_Int32*)value.data;
        int returnvalue = variableValue;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %d\n", variableValue);
        std::cout <<"arrayDimensionsSize:"<< value.arrayDimensionsSize << std::endl;
        std::cout << "arrayLength:" << value.arrayLength << std::endl;
        OPCUAInterface::opcvalue_type type = opc_int;
        opcreturn.type = type;
        opcreturn.value = (void*)returnvalue;
        /*  int tmp;
          memcpy(&tmp, (void*)&opcreturn.value, 4);
          std::std::cout<<tmp<<std::std::endl;*/
        return opcreturn;
    }
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING])) {
        UA_String variableValue = *(UA_String*)value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %.*s\n", variableValue.length, variableValue.data);

        //将变量节点的unsigned char*类型的值转化为char*类型,变量节点指针未设置终止符
        char* cr = new char[variableValue.length + 1];
        memcpy(cr, variableValue.data, variableValue.length);
        cr[variableValue.length] = 0;

        //另一种方法
        //char* str = new char[variableValue.length];
        //for (int i = 0; i < variableValue.length; i++) {
        //    str[i] = *(variableValue.data + i);
        //}
        //str[variableValue.length] = '\0';

        OPCUAInterface::opcvalue_type type = opc_string;

        opcreturn.type = type;
        opcreturn.value = (void*)cr;

        return opcreturn;
    }
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])){
        UA_Boolean variableValue= *(UA_Boolean*)value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %s\n", BOOL_TO_STR[variableValue]);
        OPCUAInterface::opcvalue_type type = opc_boolean;

        opcreturn.type = type;
        opcreturn.value = (void*)BOOL_TO_STR[variableValue];
        return opcreturn;

    }
    else {
        PLOG_DEBUG << "this type has not been completed";
        return opcreturn;
    }
}

OPCUAInterface::opcreturn OPCUAInterface::readVariableAttribute(UA_Client* client, int opcnamespace, char* t) {
    const UA_NodeId nodeId = UA_NODEID_STRING(opcnamespace, t);
    return OPCUAInterface::readFunction(client,nodeId); 
}
OPCUAInterface::opcreturn OPCUAInterface::readVariableAttribute(UA_Client* client, int opcnamespace, int t) {
    const UA_NodeId nodeId = UA_NODEID_NUMERIC(opcnamespace, t);
    return OPCUAInterface::readFunction(client, nodeId);
}
OPCUAInterface::opcreturn OPCUAInterface::readVariableAttribute(UA_Client* client, int opcnamespace, std::string t) {
    char* opcnodeId = new char[strlen(t.c_str()) + 1];
    strcpy(opcnodeId, t.c_str());
    return OPCUAInterface::readVariableAttribute(client,opcnamespace,opcnodeId);
}
