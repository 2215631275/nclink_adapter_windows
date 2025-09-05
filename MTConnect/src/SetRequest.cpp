#include "SetRequest.h"
#include <nlohmann/json.hpp>


SetRequest::SetRequest() {}

std::string SetRequest::process(std::string requestData) {
	return  SetRequest::set(requestData);
}

std::string SetRequest::set(std::string requestData) {
	nlohmann::ordered_json requestJsonData = nlohmann::json::parse(requestData);
	nlohmann::ordered_json responseJsonData;
	std::string single_value = "0"; std::string dictionary = "1"; std::string list = "2";//�������洢����������

	responseJsonData["@id"] = requestJsonData["@id"];
	responseJsonData["guid"] = requestJsonData["guid"];

	std::string type = requestJsonData["privateInfo"].get<std::string>();//������������
	auto size = requestJsonData["values"].size();//�������ݸ���
	std::vector<std::string> operation;

	for (auto i = 0; i < size; i++) {
		if (requestJsonData["values"][i]["params"].contains("operation")) {//��ȡ�������ݵ�operation
			operation.push_back(requestJsonData["values"][i]["params"]["operation"].get<std::string>());
		}
		else operation.push_back("");
	}

	if (type == single_value) {//��ֵ��
		for (auto i = 0; i < size; i++) {//Ӧ���Ǽ�������Ƿ������������,�������޸�
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {//��ȡvalueֵ�����д����赼������
				responseJsonData["results"].push_back({ {"id",requestJsonData["values"][0]["id"]}, {"code", "OK"} }); //�������ݵô���,�������޸�
				responseJsonData["results"].push_back({ {"id",requestJsonData["values"][1]["id"]}, {"code", "NG"}, {"reason", "Permission Denied"}, {"error",301}, { "params", {"operation","set_value"} } } );//�������ݵô���,�������޸�
				break;
			}
		}
	}

	else if (type == dictionary) {//�ֵ���
		for (auto i = 0; i < size; i++) {//Ӧ���Ǽ�������Ƿ������������,�������޸�
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {
				responseJsonData["results"].push_back({ {"id", requestJsonData["values"][i]["id"]}, {"code", "OK"}, { "params", {"opretion", "set_value"}, {"keys", {"k1", "k2"} } } }); //�������ݵô��� 
			}
			else if (operation[i] == "add") {//��ȡ����ֵ���������ݣ���ʱδ֪���ݸ�ʽ
			}
			else if (operation[i] == "delete") {//��ȡkeyֵ�����д����赼������
			}
		}
	}

	else if (type == list) {//�б���
		for (auto i = 0; i < size; i++) {//Ӧ���Ǽ�������Ƿ������������,�������޸�
			if (!requestJsonData["values"][i]["params"].contains("operation") || operation[i] == "set_value") {//��ȡlengthֵ�����д����赼������
				responseJsonData["results"].push_back({ {"id", requestJsonData["values"][i]["id"]}, {"code", "OK"}, { "params", {"opretion", "set_value"} , {"index", {"0"}}, {"offset",5}, {"length",10}, {"values", 16} } }); //�������ݵô��� 
			}
		}
	}
	else std::cout << "fail" << std::endl;

	return responseJsonData.dump();
}