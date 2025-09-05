
#ifdef _WIN32
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#include <iostream>
#include <open62541_linux/open62541.h>
#include <OPCUAServer.h>

using namespace std;
UA_Boolean running = true;
Config config;

static void
stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

OPCUAServer::OPCUAServer(){}

int OpenServer(int port) {


    cout << "OPCUA Server Starting ,port is opened on " << port << endl;
    UA_Server* server = UA_Server_new();
    // UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    UA_ServerConfig_setMinimal(UA_Server_getConfig(server), port, NULL);

    static UA_NodeId pointTypeId;
    UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;
    vtAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    vtAttr.valueRank = UA_VALUERANK_TWO_DIMENSIONS;
    //另一种设置vtAttr的数组类型的方法
    // vtAttr.arrayDimensionsSize=2;
    // UA_UInt32 arrayDims[2]={3,3};
    // vtAttr.arrayDimensions = arrayDims;

    vtAttr.arrayDimensions = (UA_UInt32*)UA_Array_new(2, &UA_TYPES[UA_TYPES_UINT32]);
    vtAttr.arrayDimensionsSize = 2;
    vtAttr.arrayDimensions[0] = 3;
    vtAttr.arrayDimensions[1] = 3;

    vtAttr.displayName = UA_LOCALIZEDTEXT("RenFei", "3D Test");
    // UA_Double test[3]={0.0,1.0,2.0};
    // UA_Double test[3][3]={{0.0,1.0,2.0},{3.0,4.0,5.0},{6.0,7.0,8.0}};
    UA_Double test1[9] = { 1.0, 2.0, 3.0,
                      4.0, 5.0, 6.0,
                      7.0, 8.0, 9.0 };

    UA_Variant_setArray(&vtAttr.value, test1, 9, &UA_TYPES[UA_TYPES_DOUBLE]);
    vtAttr.value.arrayDimensions = (UA_UInt32*)UA_Array_new(2, &UA_TYPES[UA_TYPES_UINT32]);
    vtAttr.value.arrayDimensionsSize = 2;
    vtAttr.value.arrayDimensions[0] = 3;
    vtAttr.value.arrayDimensions[1] = 3;
    UA_StatusCode result = UA_Server_addVariableTypeNode(server, UA_NODEID_NULL,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
        UA_QUALIFIEDNAME(1, "3D Test"), UA_NODEID_NULL,
        vtAttr, NULL, &pointTypeId);

    /* Prepare the node attributes */
    UA_VariableAttributes vAttr = UA_VariableAttributes_default;
    vAttr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    vAttr.valueRank = UA_VALUERANK_TWO_DIMENSIONS;
    UA_UInt32 arrayDims[2] = { 3,3 };
    vAttr.arrayDimensions = arrayDims;
    vAttr.arrayDimensionsSize = 2;
    vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "3DPoint Variable");
    vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "3DPoint Variable");
    /* vAttr.value is left empty, the server instantiates with the default value */
    // UA_Double zero[2] = {2.0, 3.0};
    // UA_Variant_setArray(&vAttr.value, zero, 2, &UA_TYPES[UA_TYPES_DOUBLE]);

    /* Add the node */
    UA_Server_addVariableNode(server, currentNodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, "3DPoint Type"), pointTypeId,
        vAttr, NULL, NULL);

    // add a variable node to the adresspace
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 42;
    UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    //UA_String tmp = UA_STRING_ALLOC("Hello");
    //UA_Variant_setScalarCopy(&attr.value, &tmp, &UA_TYPES[UA_TYPES_STRING]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "123456");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
        parentReferenceNodeId, myIntegerName, UA_NODEID_NULL, attr,
        NULL, NULL);

    UA_NodeId myboolNodeId = UA_NODEID_STRING_ALLOC(1, "the.answer1");
    UA_QualifiedName myboolName = UA_QUALIFIEDNAME_ALLOC(1, "the answer1");
    UA_VariableAttributes attrbool = UA_VariableAttributes_default;
    UA_Boolean test = false;
    UA_Variant_setScalarCopy(&attrbool.value, &test, &UA_TYPES[UA_TYPES_BOOLEAN]);
    attrbool.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer1");
    attrbool.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer1");
    attrbool.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Server_addVariableNode(server, myboolNodeId, parentNodeId,
        parentReferenceNodeId, myboolName, UA_NODEID_NULL, attrbool,
        NULL, NULL);

    UA_NodeId myInt32NodeId = UA_NODEID_NUMERIC(1, 123456);
    UA_QualifiedName myInt32Name = UA_QUALIFIEDNAME_ALLOC(1, "the answer2");
    UA_VariableAttributes attrInt32 = UA_VariableAttributes_default;
    UA_Int32 testInt32 = 40;
    UA_Variant_setScalarCopy(&attrInt32.value, &testInt32, &UA_TYPES[UA_TYPES_INT32]);
    attrInt32.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer");
    attrInt32.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer");
    attrInt32.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Server_addVariableNode(server, myInt32NodeId, parentNodeId,
        parentReferenceNodeId, myInt32Name, UA_NODEID_NULL, attrInt32,
        NULL, NULL);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attrInt32);
    UA_NodeId_clear(&myInt32NodeId);
    UA_QualifiedName_clear(&myInt32Name);

    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myIntegerNodeId);
    UA_QualifiedName_clear(&myIntegerName);

    UA_VariableAttributes_clear(&attrbool);
    UA_NodeId_clear(&myboolNodeId);
    UA_QualifiedName_clear(&myboolName);

    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
void* thread_opcuaServer(void* arg)
{
    int port = (size_t)arg;
    OpenServer(port);
    return NULL;
}

int OPCUAServer::init(int port) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);

    HANDLE hHandle1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_opcuaServer, (void*)port, 0, NULL);
#elif __linux__
    pthread_t tid;
    int err = pthread_create(&tid, NULL, thread_opcuaServer, (void*)port);
#endif // _WIN32
    return 0;
}

int main(int argc, char* argv[])
{
    OPCUAServer opcuaServer;

    // ��ȡ���� ��ǰ����·����../OPCUA/out/build/x64-Debug
    #ifdef _WIN32
    config.setPath("../../../conf/OPCUA.json");
    #elif __linux__
    config.setPath("./conf/OPCUA.json");
    #endif
    opcuaServer.init(config.config["opcua_config"]["server_port"]);
    while (1) {

    }
    return 0;
}
