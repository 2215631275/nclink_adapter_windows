#include <iostream>
#include <pugixml/pugixml.hpp>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <mqtt/async_client.h>
#include <httplib/httplib.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include "main.h"
#include "Config.h"
#include <OPCUAServer.h>
#include "OPCUAAdapter.h"
//#include <Utils.h>
//#include <iostream>
//#include <windows.h>
//#include <map>
//#include <cmath>
//tools::PathParse pathparse;


int adapter_init(void)
{
	OPCUAAdapter opcuaAdapter;
	opcuaAdapter.init();
	return 0;
}

int pcap_to_flows(void)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
#ifdef _WIN32
	const char* applicationName = "..\\..\\..\\..\\flow_collection\\ndpi\\pcap.exe";
#else
	const char* applicationName = "..\\flow_collection\\ndpi\\pcap";
#endif
	char currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	char fullPath[MAX_PATH];
	snprintf(fullPath, sizeof(fullPath), "%s\\%s", currentDir, applicationName);
	printf("full path: %s\n", fullPath);

	if (!CreateProcess(
		fullPath,   // ��ִ���ļ�·��
		NULL,              // �����в���������У�
		NULL,              // ���̰�ȫ����
		NULL,              // �̰߳�ȫ����
		FALSE,             // �Ƿ�̳о��
		CREATE_NEW_CONSOLE,// ������־
		NULL,              // �»����飨����У�
		NULL,              // ��ǰĿ¼
		&si,               // ָ�� STARTUPINFO �ṹ��ָ��
		&pi))              // ָ�� PROCESS_INFORMATION �ṹ��ָ��
	{
		// ��� CreateProcess ʧ�ܣ����������Ϣ
		printf("CreateProcess falied (%d).\n", GetLastError());
		return 1;
	}
}

int main(int argc, char* argv[])
{

//#ifdef _WIN32
//	SetConsoleOutputCP(65001);
//	CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)thread_opcuaServer,NULL,0,NULL);
//#elif __linux__
//	pthread_t tid;
//	int err = pthread_create(&tid, NULL, thr_fn, &cont);
//#endif // _WIN32
	//nlohmann::json config;
	//try {
	//	std::ifstream file("../../../conf/testmodel.json");
	//	config = nlohmann::json::parse(file);
	//}
	//catch (nlohmann::json::parse_error& e) {
	//	// output exception information
	//	std::cout << "message: " << e.what() << '\n'
	//		<< "exception id: " << e.id << '\n'
	//		<< "byte position of error: " << e.byte << std::endl;
	//}
	//catch (nlohmann::json::type_error& e)
	//{
	//	// output exception information
	//	std::cout << "message: " << e.what() << '\n'
	//		<< "exception id: " << e.id << std::endl;
	//}

	//std::string s= config["type"];
	//std::cout << s<<std::endl;
	//pathparse.rootParse(config);
	//std::cout << "protocol:" << pathparse.methodParams["protocol"] << std::endl;
	//std::cout << "address:" << pathparse.methodParams["address"] << std::endl;
	//std::cout << "port:" << pathparse.methodParams["port"] << std::endl;
	//std::cout << pathparse.methodName.first << std::endl;
	//std::cout << pathparse.methodName.second << std::endl;
	//for (std::map<std::string, std::string>::iterator it = pathparse.idPath.begin(); it != pathparse.idPath.end(); it++) {
	//	std::cout << it->first << std::endl;
	//	std::cout << it->second << std::endl;
	//}
		
	std::thread	thread_mtc(adapter_init);
	std::thread thread_pcap(pcap_to_flows);

	thread_pcap.join();
	thread_mtc.join();
	
	//MTConnectAdapter mtconnectAdapter;
	//mtconnectAdapter.init();

	//while (1) {

	//}
	
	return 0;
}
