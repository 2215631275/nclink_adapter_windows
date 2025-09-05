#include <iostream>
#include <pugixml/pugixml.hpp>
#include <iostream>
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
#include "MTConnectAdapter.h"

int adapter_init(void)
{
	MTConnectAdapter mtconnectAdapter;
	mtconnectAdapter.init();
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

#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif // _WIN32
	/*
	try
	{
		///httplib::Client client("http://127.0.0.1:8080");
		//auto res = client.Get("/sample");
		httplib::Client client("http://www.baidu.com");
		auto res = client.Get("/");
		std::cout << res->body << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Request failed, error: " << e.what() << '\n';
	}
	*/

	std::thread	thread_mtc(adapter_init);
	std::thread thread_pcap(pcap_to_flows);

	thread_pcap.join();
	thread_mtc.join();
	
	return 0;
}
