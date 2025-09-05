#include "utils.h"
#include <plog/Log.h>
#ifdef _WIN32
#include <Windows.h>
#define SLEEP_TIME 1000
#else
#define SLEEP_TIME 1
#include <unistd.h>
#include <sys/time.h>
#endif // _WIN32

nlohmann::json utils::parseStreams(std::string data)
{
    pugi::xml_document doc;
    nlohmann::json jsonData;
    // PLOG_DEBUG << "xml stream data:" << data;
    
    doc.load_string(data.c_str());
    pugi::xml_node streams = doc.child("MTConnectStreams").child("Streams");
    for (pugi::xml_node deviceStream : streams)
    {
        // PLOG_DEBUG << "xml stream data:" << deviceStream.attribute("name").value();
        for (pugi::xml_node componentStream : deviceStream)
        {
            for (pugi::xml_node SECData : componentStream)
            {
                for (pugi::xml_node metaData : SECData)
                {
                    nlohmann::json tempJsonData;
                    tempJsonData["name"] = metaData.attribute("name").value();
                    tempJsonData["subType"] = metaData.attribute("subType").value();
                    tempJsonData["value"] = metaData.text().as_string();
                    jsonData[metaData.attribute("dataItemId").value()] = tempJsonData;
                    
                }
            }
        }
    }

    return jsonData;
}




std::string utils::getTimeStamp()
{
    std::string timeStamp;
    #ifdef _WIN32
    #define S 1
    #else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timeStamp = std::to_string(tv.tv_sec);
    #endif
    return timeStamp;
}

