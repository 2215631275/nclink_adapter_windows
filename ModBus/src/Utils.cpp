#include <Utils.h>
#include <iostream>
#include <map>
#include <cmath>
namespace tools {

    
    void PathParse::rootParse(nlohmann::json rootObject) {
        nlohmann::json::json_pointer ptr, tempPtr;
        if (rootObject.contains("configs") && rootObject["configs"].size()) {
            ptr.push_back("configs");
            for (int i = 0;i < rootObject["configs"].size();i++) {
                if (rootObject["configs"][i]["type"] != "METHOD") {
                    tempPtr = ptr / i;
                    idPath.insert(make_pair(rootObject["configs"][i]["id"], tempPtr.to_string()));
                }
            }
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
                    sampleChannel.push_back(deviceObject["configs"][i]);
                    flowcollectionChannel.push_back(deviceObject["configs"][i]);
                }
            }
        }
        if (deviceObject.contains("dataItems") && deviceObject["dataItems"].size()) {
            if (ptr.back() == "configs") ptr.pop_back();
            ptr.push_back("dataItems");
            for (int i = 0;i < deviceObject["dataItems"].size();i++) {
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
    std::map<std::string, int> TypeConversionCode{
        std::make_pair("BIT", 0),
        std::make_pair("UINT16", 1),
        std::make_pair("INT16", 2),
        std::make_pair("UINT32", 3),
        std::make_pair("INT32", 4),
        std::make_pair("UINT64", 5),
        std::make_pair("INT64", 6),
        std::make_pair("FLOAT", 7),
    };
    std::uint8_t* TypeConversion::UINT8(std::string endianness, int rc, std::uint8_t* dest) {
        return dest;
    }

    std::uint16_t* TypeConversion::UINT16(std::string endianness, int rc, std::uint8_t* dest) {
        std::uint16_t* conversion = new std::uint16_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 2)
                conversion[j++] = (dest[i + 1] << 8) | dest[i];
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 2)
                conversion[j++] = (dest[i] << 8) | dest[i + 1];
        }
        return conversion;
    }

    std::int16_t* TypeConversion::INT16(std::string endianness, int rc, std::uint8_t* dest) {
        std::int16_t* conversion = new std::int16_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 2)
                conversion[j++] = (dest[i + 1] << 8) | dest[i];
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 2)
                conversion[j++] = (dest[i] << 8) | dest[i + 1];
        }
        return conversion;
    }

    std::uint32_t* TypeConversion::UINT32(std::string endianness, int rc, std::uint8_t* dest) {
        std::uint32_t* conversion = new std::uint32_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 4)
                conversion[j++] = (dest[i + 3] << 24) | (dest[i + 2] << 16) | (dest[i + 1] << 8) | dest[i];
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 4)
                conversion[j++] = (dest[i] << 24) | (dest[i + 1] << 16) | (dest[i + 2] << 8) | dest[i + 3];
        }
        return conversion;
    }

    std::int32_t* TypeConversion::INT32(std::string endianness, int rc, std::uint8_t* dest) {
        std::int32_t* conversion = new std::int32_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 4)
                conversion[j++] = (dest[i + 3] << 24) | (dest[i + 2] << 16) | (dest[i + 1] << 8) | dest[i];
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 4)
                conversion[j++] = (dest[i] << 24) | (dest[i + 1] << 16) | (dest[i + 2] << 8) | dest[i + 3];
        }
        return conversion;
    }

    std::uint64_t* TypeConversion::UINT64(std::string endianness, int rc, std::uint8_t* dest) {
        std::uint64_t* conversion = new std::uint64_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 8, j++) {
                for (auto k = i + 8 - 1; k >= i; k--) {
                    conversion[j] <<= 8;
                    conversion[j] |= dest[k];
                }
            }
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 8, j++) {
                for (auto k = i; k < i + 8; k++) {
                    conversion[j] <<= 8;
                    conversion[j] |= dest[k];
                }
            }
        }
        return conversion;
    }
    std::int64_t* TypeConversion::INT64(std::string endianness, int rc, std::uint8_t* dest) {
        std::int64_t* conversion = new std::int64_t[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 8, j++) {
                for (auto k = i + 8 - 1; k >= i; k--) {
                    conversion[j] <<= 8;
                    conversion[j] |= dest[k];
                }
            }
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 8, j++) {
                for (auto k = i; k < i + 8; k++) {
                    conversion[j] <<= 8;
                    conversion[j] |= dest[k];
                }
            }
        }
        return conversion;
    }
    float* TypeConversion::FLOAT(std::string endianness, int rc, std::uint8_t* dest) {
        float* conversion = new float[100];
        if (endianness == "little_endian") {
            for (auto i = 0, j = 0; i < rc; i += 4) {
                std::uint32_t word = (dest[i + 3] << 24) | (dest[i + 2] << 16) | (dest[i + 1] << 8) | dest[i];
                conversion[j++] = *(float*)&word;
            }
        }
        else {
            for (auto i = 0, j = 0; i < rc; i += 4) {
                std::uint32_t word = (dest[i] << 24) | (dest[i + 1] << 16) | (dest[i + 2] << 8) | dest[i + 3];
                conversion[j++] = *(float*)&word;
            }
        }
        return conversion;
    }
}