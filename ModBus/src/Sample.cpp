#include "Sample.h"
#include "QueryRequest.h"
#include <iostream>
#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/time.h>
#include <pthread.h>
#endif


extern std::map<std::string, std::string> idPathMap;
extern modbus::modbus_tcp modbustcp;
//extern mqtt::async_client client;
extern Config config;
/**
 * @brief ��������json������
 * @param sampleItem json��ʽ�������ݶ���
 * @param dev_uuid  �豸id
*/
Sample::Sample(nlohmann::json sampleItem, std::string dev_uuid, mqtt::async_client& cli) :client(cli)
{
    //���ݿ��ܶ�ȡ�����쳣����
    ids = sampleItem["ids"];
    sampleInterval = sampleItem["sampleInterval"];
    uploadInterval = sampleItem["uploadInterval"];
    responseJson["guid"] = dev_uuid;
    responseJson["id"] = sampleItem["id"];
    sampleChannelId = sampleItem["id"];
    datas.clear();
    responseTopic = "Sample/" + dev_uuid + "/" ;
    responseTopic += sampleItem["id"];
    std::cout << "\ntopic:" << responseTopic << std::endl;
    //ͳ��ids������������������������Ӧ������
    idNum = ids.size();
    nlohmann::json dataJson = { { "data",nlohmann::json::array()}};
    for (int i = 0; i < idNum; i++) {
        datas.push_back(dataJson);
    }
    //std::cout << datas[0]<<datas[1] << std::endl;
#ifdef WIN32
    terminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//���������̵߳��¼�
#else
    p_event = CEvent::CreateEvent(0644, TRUE, FALSE, NULL);//���������̵߳��¼�
#endif
    isClear = 1;
}

/**
 * @brief ��ȡ13λ��ʱ���
 * @return
*/
std::string Sample::getCurrentTime()
{
#ifdef _WIN32
    time_t tt;
    struct tm* st = nullptr;
	time(&tt);
	std::string seconds = std::to_string(tt);

	SYSTEMTIME t1;
	GetSystemTime(&t1);

	return seconds + std::to_string(t1.wMilliseconds);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return std::to_string(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}
/**
 * @brief ����idsִ��һ�β�������
 * @return
*/
void Sample::SampleTask(void *arg) {
    Sample* pThread = (Sample*)arg;
    nlohmann::json::array_t data;
    while (1) {
        //������ձ�ʾ��һ�η�����ɣ������µ�ʱ��
        if (pThread->isClear) {
            pThread->beginTime = pThread->getCurrentTime();
            pThread->isClear = 0;
        }
        //����ids����һ�β���������ids��˳�򷵻�
        data = QueryRequest::QuerySample(pThread->ids);
        //����ids��˳�����ζ�datas�е�Ԫ�ظ�ֵ
        for (int i = 0; i < pThread->idNum; i++) {
            (pThread->datas[i])["data"].push_back(data[i]);
        }
        data.clear();
        //Sleep(pThread->sampleInterval);
        //�ȴ�������������Ҫ��������Ը���sleep
#ifdef WIN32
        if (WAIT_OBJECT_0 == WaitForSingleObject(pThread->terminateEvent, pThread->sampleInterval)) { return; }
#else
        if (CEvent::WAIT_OBJECT_0 == CEvent::WaitForSingleObject(pThread->p_event, pThread->sampleInterval)) { return; }
#endif


    }

}
/**
 * @brief ִ��һ�������ϴ�����
 * @return
*/
void Sample::UploadTask(void* arg) {
    Sample* pThread = (Sample*)arg;
    while (1) {
        //Sleep(pThread->uploadInterval);
#ifdef WIN32
        if (WAIT_OBJECT_0 == WaitForSingleObject(pThread->terminateEvent, pThread->uploadInterval)) { return; }
#else
        if (CEvent::WAIT_OBJECT_0 == CEvent::WaitForSingleObject(pThread->p_event, pThread->uploadInterval)) { return; }
#endif

        pThread->responseJson["beginTime"] = pThread->beginTime;
        pThread->responseJson["data"] = pThread->datas;
        //�ϴ������datas�е�����
        for (int i = 0; i < pThread->idNum; i++) {
            pThread->datas[i]["data"].clear();
        }
        pThread->isClear = 1;
        //pThread->datas.clear();
        mqtt::message_ptr pubmsg = mqtt::make_message(pThread->responseTopic, to_string(pThread->responseJson));
        int qos = config.config["publish_method_qos"]["Sample/"];
        pubmsg->set_qos(qos);
        pThread->client.publish(pubmsg);
        //std::cout << "  send sampleResponse" << std::endl;

    }

}
/**
 * @brief �������������̺߳��ϴ������߳�
*/
void Sample::createThread() {
#ifdef WIN32
    HANDLE h_sampleThread, h_uploadThread;   //�߳̾��
    unsigned sampleThreadId, uploadThreadId;  //�߳�ID
    h_sampleThread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))(Sample::SampleTask), this, 0, &sampleThreadId); // �����̣߳�this���Ǳ���Ķ���ָ��
    if(h_sampleThread)
        CloseHandle(h_sampleThread);
    h_uploadThread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))(Sample::UploadTask), this, 0, &uploadThreadId);
    if(h_uploadThread)
        CloseHandle(h_uploadThread);
#else
    CEvent *h_sampleThread, *h_uploadThread;
    pthread_t sampleThreadId, uploadThreadId;  //�߳�ID
    h_sampleThread = (CEvent*)pthread_create(&sampleThreadId, NULL,
                                             reinterpret_cast<void *(*)(void *)>(Sample::SampleTask), this); // �����̣߳�this���Ǳ���Ķ���ָ��
    if(h_sampleThread)
        CEvent::CloseHandle(h_sampleThread);
    h_uploadThread = (CEvent*)pthread_create(&uploadThreadId, NULL,
                                             reinterpret_cast<void *(*)(void *)>(Sample::UploadTask), this); // �����̣߳�this���Ǳ���Ķ���ָ��
    if(h_uploadThread)
        CEvent::CloseHandle(h_uploadThread);
#endif
}



Sample::~Sample()
{
#ifdef WIN32
    if (terminateEvent) {
        SetEvent(terminateEvent);
        CloseHandle(terminateEvent);
    }

#else
    if (p_event) {
        CEvent::SetEvent(p_event);
        CEvent::CloseHandle(p_event);
    }
#endif
}
