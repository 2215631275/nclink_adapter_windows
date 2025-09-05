#include "SetRequest.h"
#include <nlohmann/json.hpp>


SetRequest::SetRequest() {}

std::string SetRequest::process(std::string requestData) {
	return  SetRequest::set(requestData);
}

std::string SetRequest::set(std::string requestData) {
	nlohmann::ordered_json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::ordered_json responseJsonData;
	std::string single_value = "0"; std::string dictionary = "1"; std::string list = "2";//适配器存储的数据类型

	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];

	std::string type = requestJsonData["privateInfo"].get<std::string>();//请求数据类型
	auto size = requestJsonData["values"].size();//请求数据个数
	std::vector<std::string> operation;

	for (auto i = 0; i < size; i++) {
		if (requestJsonData["values"][i]["params"].contains("operation")) {//获取请求数据的operation
			operation.push_back(requestJsonData["values"][i]["params"]["operation"].get<std::string>());
		}
		else operation.push_back("");
	}

	if (type == single_value) {//单值型
		for (auto i = 0; i < size; i++) {//应当是检查数据是否存在于适配器,后面需修改
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {//获取value值，下列代码需导入数据
				responseJsonData["results"].push_back({ {"id",requestJsonData["values"][0]["id"]}, {"code", "OK"} }); //导入数据得代码,后面需修改
				responseJsonData["results"].push_back({ {"id",requestJsonData["values"][1]["id"]}, {"code", "NG"}, {"reason", "Permission Denied"}, {"error",301}, { "params", {"operation","set_value"} } } );//导入数据得代码,后面需修改
				break;
			}
		}
	}

	else if (type == dictionary) {//字典型
		for (auto i = 0; i < size; i++) {//应当是检查数据是否存在于适配器,后面需修改
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {
				responseJsonData["results"].push_back({ {"id", requestJsonData["values"][i]["id"]}, {"code", "OK"}, { "params", {"opretion", "set_value"}, {"keys", {"k1", "k2"} } } }); //导入数据得代码 
			}
			else if (operation[i] == "add") {//获取属性值，导入数据，暂时未知数据格式
			}
			else if (operation[i] == "delete") {//获取key值，下列代码需导入数据
			}
		}
	}

	else if (type == list) {//列表型
		for (auto i = 0; i < size; i++) {//应当是检查数据是否存在于适配器,后面需修改
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {//获取length值，下列代码需导入数据
				responseJsonData["results"].push_back({ {"id", requestJsonData["values"][i]["id"]}, {"code", "OK"}, { "params", {"opretion", "set_value"} , {"index", {"0"}}, {"offset",5}, {"length",10}, {"values", 16} } }); //导入数据得代码 
			}
		}
	}
	else std::cout << "fail" << std::endl;

	return responseJsonData.dump();
}