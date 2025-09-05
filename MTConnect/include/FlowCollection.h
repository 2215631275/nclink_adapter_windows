#pragma once
#include "ProcessJsonData.h"
#include <iostream>
#include <iostream>
#include<nlohmann/json.hpp>
#include<time.h>
#include <mqtt/async_client.h>
#include "Config.h"

class FlowCollection
{
public:
    FlowCollection(std::string dev_uuid, std::int32_t sampleinterval, std::int32_t uploadinterval, mqtt::async_client& cli);
    ~FlowCollection();
    std::string getCurrentTime();
    static void SampleTask(void* arg);
    static void UploadTask(void* arg);
    void createThread();

    std::string responseTopic;
    mqtt::async_client& client;
    typedef void *HANDLE;
#ifndef _WIN32
    CEvent *p_event;
#endif 
    HANDLE terminateEvent;
    std::string sampleChannelId;

private:

    nlohmann::json::array_t ids;
    std::int32_t sampleInterval;
    std::int32_t uploadInterval;
    std::string beginTime;

    nlohmann::ordered_json probe;
    nlohmann::ordered_json responseJson;
    int idNum;
    int isClear;
    int isJson;
};


