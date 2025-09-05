#pragma once
#include <Utils.h>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <map>
#include <cmath>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
namespace tools {

    
    void PathParse::rootParse(nlohmann::json rootObject) {
        nlohmann::json::json_pointer ptr, tempPtr;
        if (rootObject.contains("configs") && rootObject["configs"].size()) {
            ptr.push_back("configs");
            for (int i = 0; i < rootObject["configs"].size(); i++) {
                if (rootObject["configs"][i]["type"] != "METHOD") {
                    tempPtr = ptr / i;
                    idPath.insert(make_pair(rootObject["configs"][i]["id"], tempPtr.to_string()));
                }
                else {
                    tempPtr = ptr / i;
                    idPath.insert(make_pair(rootObject["configs"][i]["id"], tempPtr.to_string()));
                    methodName.first = rootObject["configs"][i]["id"];
                    methodName.second= rootObject["configs"][i]["name"];
                    methodParams.insert(make_pair("protocol", rootObject["configs"][i]["args"]["protocol"]));
                    methodParams.insert(make_pair("address", rootObject["configs"][i]["args"]["address"]));
                    methodParams.insert(make_pair("port", rootObject["configs"][i]["args"]["port"]));
                }
            }
        }
        else {
            printf("root has no configs\n");
        }
        if (rootObject.contains("devices") && rootObject["devices"].size()) {
            if (!ptr.empty() && ptr.back() == "configs") ptr.pop_back();
            ptr.push_back("devices");
            for (int i = 0;i < rootObject["devices"].size();i++) {
                tempPtr = ptr / i;
                devicesParse(rootObject["devices"][i], tempPtr);
            }
        }
    }

    void PathParse::devicesParse(nlohmann::json deviceObject, nlohmann::json::json_pointer ptr) {
        nlohmann::json::json_pointer tempPtr;
        if (deviceObject.contains("configs") && deviceObject["configs"].size()) {
            ptr.push_back("configs");
            for (int i = 0;i < deviceObject["configs"].size();i++) {
                if (deviceObject["configs"][i]["type"] != "SAMPLE_CHANNEL") {
                    tempPtr = ptr / i;
                    idPath.insert(make_pair(deviceObject["configs"][i]["id"], tempPtr.to_string()));
                }
                else {
                    //�洢����ͨ��
                    flowcollectionChannel.push_back(deviceObject["configs"][i]);
                    sampleChannel.push_back(deviceObject["configs"][i]);
                    std::cout << deviceObject["configs"][i] << std::endl;

                    int j = deviceObject["configs"][i]["ids"].size();
                    for (int k = 0; k < j; k++) {
                        sampleId.push_back(deviceObject["configs"][i]["ids"][k]["id"]);
                    }
                }
            }
        }
        if (deviceObject.contains("dataItems") && deviceObject["dataItems"].size()) {
            if (ptr.back() == "configs") ptr.pop_back();
            ptr.push_back("dataItems");
            for (int i = 0;i < deviceObject["dataItems"].size();i++) {
                if (deviceObject["dataItems"][i].contains("identity")) {
                    nlohmann::json temp = deviceObject["dataItems"][i]["identity"];
                    idIdentity.insert(make_pair(deviceObject["dataItems"][i]["id"], temp));
                }
                tempPtr = ptr / i;
                idPath.insert(make_pair(deviceObject["dataItems"][i]["id"], tempPtr.to_string()));

            }
        }

        if (deviceObject.contains("components") && deviceObject["components"].size()) {
            if (ptr.back() == "dataItems") ptr.pop_back();
            ptr.push_back("components");
            for (int i = 0;i < deviceObject["components"].size();i++) {
                tempPtr = ptr / i;
                componentsParse(deviceObject["components"][i], tempPtr);
            }
        }



    }

    void PathParse::componentsParse(nlohmann::json component, nlohmann::json::json_pointer ptr) {
        nlohmann::json::json_pointer tempPtr;
        if (component.contains("configs") && component["configs"].size()) {
            ptr.push_back("configs");
            for (int i = 0;i < component["configs"].size();i++) {
                if (component["configs"][i]["type"] != "SAMPLE_CHANNEL") {
                    tempPtr = ptr / i;
                    idPath.insert(make_pair(component["configs"][i]["id"], tempPtr.to_string()));
                }
            }
        }
        if (component.contains("dataItems") && component["dataItems"].size()) {
            if (ptr.back() == "configs") ptr.pop_back();
            ptr.push_back("dataItems");
            for (int i = 0;i < component["dataItems"].size();i++) {
                tempPtr = ptr / i;
                idPath.insert(make_pair(component["dataItems"][i]["id"], tempPtr.to_string()));
            }
        }
        if (component.contains("components") && component["components"].size()) {
            if (ptr.back() == "dataItems") ptr.pop_back();
            ptr.push_back("components");
            for (int i = 0;i < component["components"].size();i++) {
                tempPtr = ptr / i;
                componentsParse(component["components"][i], tempPtr);
            }
        }
    }
   
}