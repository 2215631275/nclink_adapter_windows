#include "SetRequest.h"
#include <nlohmann/json.hpp>
#include <ModbusInterface.h>
#include "Utils.h"
#include "ErrorReason.h"
#include <plog/Log.h>
#include <string>

#include<fstream>
extern std::map<std::string, std::string> idPathMap;
extern nlohmann::ordered_json model;

extern modbus::modbus_tcp modbustcp;

SetRequest::SetRequest() {}
std::string SetRequest::process(std::string requestData) {
	return  SetRequest::set(requestData);
}

std::string SetRequest::set(std::string requestData) {
	
	nlohmann::ordered_json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::ordered_json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];
	responseJsonData["results"] = nlohmann::json::array();
	std::int32_t retCode = 0;
	nlohmann::ordered_json valueJson;
	nlohmann::ordered_json resultJson;
	std::map<std::int32_t, std::string>::iterator errorIt;
	if (requestJsonData.contains("values") && requestJsonData["values"].is_array()) {
		
		for (int i = 0;i < requestJsonData["values"].size();i++) {
			valueJson = requestJsonData["values"][i];
			if (valueJson["values"][i].contains("id")) {
				retCode = 401;
				errorIt = errorCode.find(retCode);
				resultJson["code"] = "NG";
				resultJson["reason"] = errorIt->second;
				resultJson["error"] = retCode;
				resultJson["params"] = valueJson["params"];
				responseJsonData["results"].push_back(valueJson);
				continue;
			}
			resultJson["id"] = valueJson["id"];//�˴���idд����Ӧ����
			if (valueJson["params"].contains("operation") && (valueJson["params"]["operation"] == "add" || valueJson["params"]["operation"] == "delete") && !valueJson["params"].contains("key")) {
				retCode = 403;//û����key
				
			}
			if (valueJson["params"].contains("key")) {
				//��������Ϊjson��ͨ��key�ж�
				retCode = SetRequest::setDict(valueJson);
			}
			if (valueJson["params"].contains("index")) {
				retCode = SetRequest::setList(resultJson);
			}
			else if(valueJson["params"].contains("offset")|| valueJson["params"].contains("length")) {
				retCode = 402;//û������ֵ
			}
			if ((valueJson["params"].contains("operation") && valueJson["params"]["operation"] == "set_value") || !valueJson["params"].contains("operation")) {
				if (valueJson["params"].contains("value") && valueJson["params"]["value"].is_number())
					retCode = SetRequest::setValue(valueJson);
				else 
					retCode = 102;////���ز�������
				
			}
			if (retCode == 0) {
				resultJson["code"] = "OK";
			}
			else {
				errorIt = errorCode.find(retCode);
				resultJson["code"] = "NG";
				resultJson["reason"] = errorIt->second;
				resultJson["error"] = retCode;
				resultJson["params"] = valueJson["params"];
				//����code��ȡʧ��ԭ��
			}
            responseJsonData["results"].push_back(resultJson);
		}
	}
	else {
		//���ز�������
		retCode = 102;
		errorIt = errorCode.find(retCode);
		resultJson["code"] = "NG";
		resultJson["reason"] = errorIt->second;
		resultJson["error"] = retCode;
		resultJson["params"] = valueJson["params"];
        responseJsonData["results"].push_back(resultJson);
	}
	return responseJsonData.dump(1);
}

std::int32_t SetRequest::setDict(nlohmann::ordered_json valueJson) {
	std::map<std::string, std::string>::iterator pos = idPathMap.find(valueJson["id"]);
	if (pos != idPathMap.end()) {
		nlohmann::ordered_json dataItem = model[(nlohmann::json::json_pointer)(*pos).second];
		if (dataItem.contains("datatype") && dataItem["datatype"] == "dict") {
			if (valueJson.contains("operation") && valueJson["operation"] == "add") {
				if (valueJson.contains("value"))
					dataItem[(std::string)valueJson["key"]] = valueJson["value"];
				else return 102;//���õ�value������
			}
			if (valueJson.contains("operation") && valueJson["operation"] == "delete" && dataItem["value"].contains(valueJson["key"])) {
				dataItem.erase((std::string)valueJson["key"]);
			}
			else return 410;//��ֵ����
			if ((valueJson.contains("operation") && valueJson["operation"] == "set_value")|| !valueJson.contains("operation")) {
				if (!valueJson.contains("value")) return 102;//���õ�value������
				if (dataItem["value"].contains(valueJson["key"]))
					dataItem[(std::string)valueJson["key"]] = valueJson["value"];
				else return 410; //��ֵ����
			}
		}
		else {
			return 405;//ֵ���ʹ���
		}
		return 0;//���óɹ�

	}
    return 401;//�ڵ㲻���
}
//listûд��
std::int32_t SetRequest::setList(nlohmann::ordered_json valueJson) {
	std::map<std::string, std::string>::iterator pos = idPathMap.find(valueJson["id"]);
	if (pos != idPathMap.end()) {
		nlohmann::ordered_json dataItem = model[(nlohmann::json::json_pointer)(*pos).second];
		if (dataItem.contains("datatype") && dataItem["datatype"] == "list") {
			if (valueJson["index"].is_number()) {
				if (valueJson["index"] > dataItem["value"].size()) {
					return 101;
				}
				else {
					
				}
			}
			else {
				return 405;
			}
		}
		else {
			return 405;
		}
	}
    return 401;
}
/**
 * @brief ͨ��valueJson�е�id���ö�Ӧģ���ļ��е���������޸�modbus������
 * @param valueJson :�����а���id����Сjson�ṹ
 * @return 
*/
std::int32_t SetRequest::setValue(nlohmann::ordered_json valueJson) {
	//��·�����в���id����ȡ������
	std::map<std::string, std::string>::iterator pos = idPathMap.find(valueJson["id"]);
	if (pos != idPathMap.end()) {
		nlohmann::json::json_pointer ptr = (nlohmann::json::json_pointer)(pos->second);
		auto dataItem = model[ptr];
		dataItem["value"] = valueJson["params"]["value"];
		
		std::uint16_t* data16;
		if (dataItem.contains("setable") && dataItem["setable"] == true) {
			auto methods = dataItem["methods"];
			for (int i=0;i < methods.size();i++) {
				if (methods[i]["method"] == "write") {
					std::int16_t regAddress = methods[i]["address"];
					std::int16_t regNum = methods[i]["number"];
					std::int16_t funcCode = methods[i]["functioncode"];
					switch (funcCode) {
					case 5:
						modbustcp.write_single_coil(regAddress, dataItem["value"]);
						break;
					case 15:
						//д�����ȦʱҪ�����һ��0/1����
						if (dataItem["value"].is_array()) {
							std::uint8_t* data = new std::uint8_t[dataItem["value"].size() + 1];
							for (int i = 0; i < dataItem["value"].size(); i++) {
								data[i] = dataItem["value"][i];
							}
							modbustcp.write_multiple_coil(regAddress, regNum, data);
						}
						break;
					case 6:
						modbustcp.write_single_register(regAddress, dataItem["value"]);
						break;
					case 16: {
						int rc = 0; uint16_t dest[100] = { 0 };
						
						//��Ҫ���������͸��ݴ�С�˽���ת������͵Ķ�Ĵ���Ϊ�����Ĵ�����Ĭ�ϵ������Ǵ��
						if (methods[i]["datatype"] == "UINT16" || methods[i]["datatype"] == "INT16" || methods[i]["datatype"] == "BIT") {
							PLOG_DEBUG << "Function code is wrong.Please choose a suitable function code." << std::endl;
							
							return 202;//���ݻ���񲻿���
							break;
						}
						//UINT32��INT32����һ����
						if (methods[i]["datatype"] == "UINT32"|| methods[i]["datatype"] == "INT32") {
							uint32_t number = dataItem["value"].get<uint32_t>();
							uint32_t target = 0; rc = 2;
							memcpy(&target, &number, sizeof(number));
							if (methods[i]["endianness"] == "big_endian") {
								target = (number & 0x0000FFFFU) << 16 | (number & 0xFFFF0000U) >> 16;
								dest[0] = *(uint16_t*)&target;
								dest[1] = *((uint16_t*)&target + 1);
							}
							else {
								//���������ֽ����������С�ˣ�modbus�������ֽ�����С�ˣ������Ǵ�ˡ�0xABCD->DCBA ==>0xCDAB
								
								dest[0] = *((uint16_t*)&number + 1);
								dest[0] = (dest[0] & 0xFFU) << 8 | (dest[0] & 0xFF00U) >> 8;//��������Ĭ���Ǵ�ˣ�������Ҫ�������е��ֽ�
								dest[1] = *(uint16_t*)&number;
								dest[1] = (dest[1] & 0xFFU) << 8 | (dest[1] & 0xFF00U) >> 8;

							}
						}
						//UINT64��INT64����һ����
						if (methods[i]["datatype"] == "UINT64"|| methods[i]["datatype"] == "INT64") {
							uint64_t number = dataItem["value"].get<uint64_t>();
							uint64_t target = 0; rc = 4;
							if (methods[i]["endianness"] == "big_endian") {
								target |= ((number & 0x000000000000FFFFull) << 48);
								target |= ((number & 0x00000000FFFF0000ull) << 16);
								target |= ((number & 0x0000FFFF00000000ull) >> 16);
								target |= ((number & 0xFFFF000000000000ull) >> 48);
								for (int i = 0; i < rc; i++) {
									dest[i] = *((uint16_t*)&target + i);
								}
							}
							else {
								for (int i = 0; i <rc; i++) {
									dest[3-i] = *((uint16_t*)&number + i);
									dest[3-i] = (dest[3 - i] & 0xFFU) << 8 | (dest[3 - i] & 0xFF00U) >> 8;

								}

							}
						}
						if (methods[i]["datatype"] == "FLOAT") {
							float number = dataItem["value"].get<float>();
							uint32_t target = 0; rc = 2;
							memcpy(&target, &number, sizeof(number));
							if (methods[i]["endianness"] == "big_endian") {
								target = (target & 0x0000FFFFU) << 16 | (target & 0xFFFF0000U) >> 16;
								dest[0] = *(uint16_t*)&target;
								dest[1] = *((uint16_t*)&target + 1);
							}
							else {
								dest[0] = *((uint16_t*)&target + 1);
								dest[0] = (dest[0] & 0xFFU) << 8 | (dest[0] & 0xFF00U) >> 8;//��������Ĭ���Ǵ�ˣ�������Ҫ�������е��ֽ�
								dest[1] = *(uint16_t*)&target;
								dest[1] = (dest[1] & 0xFFU) << 8 | (dest[1] & 0xFF00U) >> 8;

							}
						}

						modbustcp.write_multiple_registers(regAddress, rc, dest);
					}
					default:
						break;
					}
					



				}
			}
		}
		return 0;
		

	}
	else return 401;
}
