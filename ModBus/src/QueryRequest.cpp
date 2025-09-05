#include "QueryRequest.h"
#include<ModbusInterface.h>
#include <Utils.h>
#include "ErrorReason.h"
#include<plog/Log.h>
#include<string>
#include<sstream>

#include<fstream>
extern std::map<std::string, std::string> idPathMap;
extern nlohmann::ordered_json model;
extern modbus::modbus_tcp modbustcp;
QueryRequest::QueryRequest() {}
//ע�͵�Ϊ��ȡ��ǰʱ��
//std::string time() {
//	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
//	time_t tt = std::chrono::system_clock::to_time_t(now);
//	struct tm* tmnow = localtime(&tt);
//	char date[20] = { 0 };
//	sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", (int)tmnow->tm_year + 1900, (int)tmnow->tm_mon + 1,
//		(int)tmnow->tm_mday, (int)tmnow->tm_hour, (int)tmnow->tm_min, (int)tmnow->tm_sec);
//	std::string timenow(date);
//	return timenow;
//}

std::string QueryRequest::process(std::string requestData) {
	return  QueryRequest::get(requestData);
}

std::string QueryRequest::get(std::string requestData) {
	nlohmann::ordered_json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::ordered_json responseJsonData;
	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];
	responseJsonData["values"] = nlohmann::json::array();
	nlohmann::ordered_json valueJson, resultJson;
	std::int32_t retCode = 0;
	std::string operation;
  std::map<std::int32_t, std::string>::iterator errorIt;

	for (auto i = 0; i < requestJsonData["ids"].size(); i++) {
		nlohmann::ordered_json::iterator item;

		std::string id = requestJsonData["ids"][i]["id"];
		if (requestJsonData["ids"][i]["params"].contains("operation")) {//��ȡ�������ݵ�operation
			operation = requestJsonData["ids"][i]["params"]["operation"];
		}
		else operation = "get_value";
		//ģ���ļ�������Ҹ�id
		valueJson = requestJsonData["ids"][i];
		std::map<std::string, std::string>::iterator pos = idPathMap.find(valueJson["id"]);
		if (pos != idPathMap.end()) {
			nlohmann::ordered_json dataItem = model[(nlohmann::json::json_pointer)(*pos).second];

			if (dataItem.contains("methods")) {
				for (auto index = dataItem["methods"].begin(); index != dataItem["methods"].end(); index++) {
					if ((*index)["method"] == "read")
						item = index;
					break;
				}
				responseJsonData["values"][i]["id"] = id;
				QueryRequest::QueryValue(resultJson, operation, item);
				responseJsonData["values"][i]["code"] = "OK";
				responseJsonData["values"][i]["params"]["operation"] = operation;
				responseJsonData["values"][i].insert(resultJson.begin(), resultJson.end());
				PLOG_DEBUG << "Modbus request: No exceptions, succeed !";
			}
		}
		else {
      responseJsonData["values"][i]["id"] = id;
      retCode = 401;
      errorIt = errorCode.find(retCode);
			responseJsonData["values"][i]["code"] = "NG";
			responseJsonData["values"][i]["reason"] = errorIt->second;
			responseJsonData["values"][i]["error"] = retCode;
    }
	}
	return responseJsonData.dump(1);
}

std::int32_t QueryRequest::QueryValue(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item) {
	resultJson.clear();
	std::uint8_t dest[100] = {};
	int rc = 0;
	if ((*item)["functioncode"].is_null() || (*item)["registertype"].is_null() || (*item)["address"].is_null() || (*item)["number"].is_null())
		return 102;
	else {
		if(operation == "get_value"){
			switch ((*item)["functioncode"].get<int>()) {
				case 1:
					rc = modbustcp.read_coil_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest);
					break;
				case 2:
					rc = modbustcp.read_input_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest);
					break;
				case 3:
					rc = modbustcp.read_holding_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest);
					break;
				case 4:
					rc = modbustcp.read_input_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest);
					break;
				default:
					break;
			}
			std::map<std::string, int>::iterator it = tools::TypeConversionCode.find((*item)["datatype"]);
			tools::TypeConversion typeconvertion;
			switch (it->second) {
				case 0: {
					

					std::uint8_t* dest1 = typeconvertion.UINT8((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>(); j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
					}
				case 1: {
					std::uint16_t* dest1 = typeconvertion.UINT16((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>(); j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 2: {
					std::int16_t* dest1 = typeconvertion.INT16((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>(); j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 3: {
					std::uint32_t* dest1 = typeconvertion.UINT32((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>() / 2; j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 4: {
					std::int32_t* dest1 = typeconvertion.INT32((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>() / 2; j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
						Sleep(50);
#else
						usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 5: {
					std::uint64_t* dest1 = typeconvertion.UINT64((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>() / 4; j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 6: {
					std::int64_t* dest1 = typeconvertion.INT64((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>() / 4; j++)
						resultJson["values"].push_back(dest1[j]);
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				case 7: {
					float* dest1 = typeconvertion.FLOAT((*item)["endianness"], rc, dest);
					for (auto j = 0; j < (*item)["number"].get<int>() / 2; j++)
						resultJson["values"].push_back(std::stod(std::to_string(dest1[j])));
#ifdef WIN32
					Sleep(50);
#else
					usleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
#endif
					break;
				}
				default:
					break;
			}
		}
		if (operation == "get_attributes") {
			resultJson["values"] = (*item)["registertype"];
		}
	}
	return 0;
}


nlohmann::json::array_t QueryRequest::QuerySample(nlohmann::json::array_t ids) {
	nlohmann::json::array_t data ;
	data.clear();
	int rc = 0;
	std::uint8_t dest[100] = {};
	for (auto index = ids.begin();index != ids.end();index++) {
		std::string id = (*index)["id"];
		std::map<std::string, std::string>::iterator pos = idPathMap.find(id);
		if (pos != idPathMap.end()) {
			nlohmann::ordered_json dataItem = model[(nlohmann::json::json_pointer)(*pos).second];
			if (dataItem.contains("methods")) {
				for (auto index = dataItem["methods"].begin(); index != dataItem["methods"].end(); index++) {
					if ((*index)["method"] == "read") {
						switch ((*index)["functioncode"].get<int>()) {
						case 1:
							rc = modbustcp.read_coil_status((*index)["address"].get<int>(), (*index)["number"].get<int>(), dest);
							break;
						case 2:
							rc = modbustcp.read_input_status((*index)["address"].get<int>(), (*index)["number"].get<int>(), dest);
							break;
						case 3:
							rc = modbustcp.read_holding_register((*index)["address"].get<int>(), (*index)["number"].get<int>(), dest);
							break;
						case 4:
							rc = modbustcp.read_input_register((*index)["address"].get<int>(), (*index)["number"].get<int>(), dest);
							break;
						default:
							break;
						}
						std::map<std::string, int>::iterator it = tools::TypeConversionCode.find((*index)["datatype"]);
						tools::TypeConversion typeconvertion;
						
						switch (it->second) {
						case 0: {

							nlohmann::json::array_t bitArray;
							std::uint8_t* dest1 = typeconvertion.UINT8((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>(); j++)
								bitArray.push_back(dest1[j]);
							data.push_back(bitArray);
							//_sleep(50);
							break;
						}
						case 1: {
							std::uint16_t* dest1 = typeconvertion.UINT16((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>(); j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 2: {
							std::int16_t* dest1 = typeconvertion.INT16((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>(); j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 3: {
							std::uint32_t* dest1 = typeconvertion.UINT32((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>() / 2; j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 4: {
							std::int32_t* dest1 = typeconvertion.INT32((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>() / 2; j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 5: {
							std::uint64_t* dest1 = typeconvertion.UINT64((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>() / 4; j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 6: {
							std::int64_t* dest1 = typeconvertion.INT64((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>() / 4; j++)
								data.push_back(dest1[j]);
							//_sleep(50);
							break;
						}
						case 7: {
							float* dest1 = typeconvertion.FLOAT((*index)["endianness"], rc, dest);
							for (auto j = 0; j < (*index)["number"].get<int>() / 2; j++)
								data.push_back(std::stod(std::to_string(dest1[j])));
							//_sleep(50);
							break;
						}
						default:
							break;
						}
					}

						break;
				}

			}

		}
	}
		return data;
}


		//std::int32_t QueryRequest::QueryDict(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item) {
//	resultJson.clear();
//	if (operation == "get_value") {
//		std::uint8_t dest_1[100] = {}; std::uint8_t dest_2[100] = {}; std::uint8_t dest_3[100] = {}; std::uint16_t dest_4[100] = {};
//		switch ((*item)["functioncode"].get<int>()) {
//		case 1:
//			modbustcp.read_coil_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_1);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_1[j]);
//			_sleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
//			break;
//		case 2:
//			modbustcp.read_input_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_2);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_2[j]);
//			_sleep(50);
//			break;
//		case 3:
//			modbustcp.read_holding_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_3);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_3[j]);
//			_sleep(50);
//			break;
//		case 4:
//			modbustcp.read_input_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_4);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_4[j]);
//			_sleep(50);
//			break;
//		default:
//			break;
//		}
//	}
//	if (operation == "get_keys") {
//		
//	}
//	if (operation == "get_length") {
//		resultJson["values"] = (*item)["number"];
//	}
//	return 0;
//}
//
//std::int32_t QueryRequest::QueryList(nlohmann::ordered_json& resultJson, std::string operation, nlohmann::ordered_json::iterator item) {
//	resultJson.clear();
//	if (operation == "get_value") {
//		std::uint8_t dest_1[100] = {}; std::uint8_t dest_2[100] = {}; std::uint8_t dest_3[100] = {}; std::uint16_t dest_4[100] = {};
//		switch ((*item)["functioncode"].get<int>()) {
//		case 1:
//			modbustcp.read_coil_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_1);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_1[j]);
//			_sleep(50);//0.05s�ӳٱ�֤��������������ֹճ��
//			break;
//		case 2:
//			modbustcp.read_input_status((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_2);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_2[j]);
//			_sleep(50);
//			break;
//		case 3:
//			modbustcp.read_holding_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_3);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_3[j]);
//			_sleep(50);
//			break;
//		case 4:
//			modbustcp.read_input_register((*item)["address"].get<int>(), (*item)["number"].get<int>(), dest_4);
//			for (auto j = 0; j < (*item)["number"].get<int>(); j++)
//				resultJson["values"].push_back(dest_4[j]);
//			_sleep(50);
//			break;
//		default:
//			break;
//		}
//	}
//	if (operation == "get_attributes") {
//		resultJson["values"] = (*item)["registertype"];
//	}
//	if (operation == "get_length") {
//		resultJson["values"] = (*item)["number"];
//	}
//	return 0;
//}
//
