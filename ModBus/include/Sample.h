
#ifndef SAMPLE
#define SAMPLE
#include <iostream>
#include<nlohmann/json.hpp>
#include<time.h>
#include <mqtt/async_client.h>
#ifndef _WIN32
#include <fcntl.h>
#include <atomic>
#include <semaphore.h>
#endif

#include "Config.h"
#include "ModbusInterface.h"
/**
 * @brief ���ģ���ļ��еĲ���ͨ�������ܽ�������Ӧ��ϵͳ�Ĳ�������
*/
#ifndef _WIN32


class CEvent {
private:
    CEvent(long lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName) {
        _lpEventAttributes = lpEventAttributes;
        _b_manual_reset = bManualReset;
        _b_initial_state = bInitialState;
        if (lpName != nullptr) _sem_name = lpName;
    }
    ~CEvent()
    {
        if (_p_named_sem != SEM_FAILED)
        {
            if (_sem_name.empty())
            {
                sem_destroy(&_sem);
                _p_named_sem = SEM_FAILED;
            }
            else
            {
                sem_close(_p_named_sem);
                sem_unlink(_sem_name.c_str());
                _sem_name.clear();
            }
        }

    }
    bool Init()
    {
        if (_b_initial_state) _sem_count = 1;
        else _sem_count = 0;
        if (_sem_name.empty())
        {
            if (0 != sem_init(&_sem, 0, _sem_count))
            {
                _p_named_sem = SEM_FAILED;
                return false;
            }
            _p_named_sem = &_sem;
        }
        else {
            _p_named_sem = sem_open(_sem_name.c_str(), O_CREAT, _lpEventAttributes, _sem_count);
            if (_p_named_sem == SEM_FAILED)
            {
                sem_unlink(_sem_name.c_str());
                _sem_name.clear();
                return false;
            }
        }
        return true;
    }
    timespec sem_get_time_millsecs(long msecs)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long secs = msecs / 1000;
        msecs = msecs % 1000;

        long add = 0;
        msecs = msecs * 1000 * 1000 + ts.tv_nsec;
        add = msecs / (1000 * 1000 * 1000);
        ts.tv_sec += (add + secs);
        ts.tv_nsec = msecs % (1000 * 1000 * 1000);
        return ts;
    }
public:
    enum {
        WAIT_OBJECT_0 = 0,
        WAIT_TIMEOUT = 0x00000102L,
        WAIT_FAILED = 0xFFFFFFFF
    };
    /// <summary>
    /// 创建事件
    /// </summary>
    /// <param name="lpEventAttributes">安全属性，0777</param>
    /// <param name="bManualReset">复位方式</param>
    /// <param name="bInitialState">初始状态</param>
    /// <param name="lpName">事件名称</param>
    /// <returns>CEvent*</returns>
    static CEvent* CreateEvent(long lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName) {
        CEvent* p_event = new CEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
        if (!p_event->Init())
        {
            delete p_event;
            return nullptr;
        }
        return p_event;
    }
    /// <summary>
    /// 关闭事件
    /// </summary>
    /// <param name="p_event">CEvent*</param>
    static void CloseHandle(CEvent* &p_event) {
        if (p_event != nullptr)
            delete p_event;
        p_event = nullptr;
    }
    /// <summary>
    /// 等待事件
    /// </summary>
    /// <param name="p_event">事件</param>
    /// <param name="ms">超时时间，0 永不超时</param>
    /// <returns>WAIT_OBJECT_0/WAIT_TIMEOUT/WAIT_FAILED</returns>
    static unsigned long WaitForSingleObject(CEvent* p_event, long ms)
    {
        if (p_event == nullptr)
            return WAIT_FAILED;
        if (p_event->_b_initial_state) {
            if (!p_event->_b_manual_reset) {
                p_event->_b_initial_state = false;
                while (sem_getvalue(p_event->_p_named_sem, &p_event->_sem_count) == 0 && p_event->_sem_count > 0)
                    sem_wait(p_event->_p_named_sem);
            }
            return WAIT_OBJECT_0;
        }
        if (ms == 0) {
            int n_ret = sem_wait(p_event->_p_named_sem);
            if (n_ret != 0)
                return WAIT_FAILED;
        }
        else {
            int n_ret = 0;
            timespec ts = p_event->sem_get_time_millsecs(ms);
            while ((n_ret = sem_timedwait(p_event->_p_named_sem, &ts)) == -1 && errno == EINTR)
                continue;       /* Restart if interrupted by handler */
            if (n_ret == -1) {
                if (errno == ETIMEDOUT)
                    return WAIT_TIMEOUT;
                else
                    return WAIT_FAILED;
            }
        }
        if (p_event->_b_manual_reset)
            p_event->_b_initial_state = true;
        return WAIT_OBJECT_0;
    }
    /// <summary>
    /// 触发事件
    /// </summary>
    /// <param name="p_event">事件</param>
    /// <returns>true/false</returns>
    static bool SetEvent(CEvent* p_event)
    {
        if (p_event == nullptr) return false;
        int n_ret = sem_post(p_event->_p_named_sem);
        if (n_ret != 0)
            return false;
        return true;
    }
    /// <summary>
    /// 复位事件
    /// </summary>
    /// <param name="p_event">事件</param>
    /// <returns>true/false</returns>
    static bool ResetEvent(CEvent* p_event)
    {
        if (p_event == nullptr) return false;
        p_event->_b_initial_state = false;
        while (sem_getvalue(p_event->_p_named_sem, &p_event->_sem_count) == 0 && p_event->_sem_count > 0)
            sem_wait(p_event->_p_named_sem);
        return true;
    }
private:
    bool _b_manual_reset;// 复位方式
    std::atomic_bool _b_initial_state;// 初始状态

    sem_t* _p_named_sem = SEM_FAILED;
    sem_t  _sem;
    std::string	_sem_name;
    long _lpEventAttributes;
    int _sem_count = 0;
};
#endif

class Sample
{
public:
    Sample(nlohmann::json sampleItem, std::string dev_uuid, mqtt::async_client& cli);
    ~Sample();
    std::string getCurrentTime();
    static void SampleTask(void* arg);
    static void UploadTask(void* arg);
    void createThread();

    std::string responseTopic;
    mqtt::async_client& client;
#ifndef WIN32
    typedef void *HANDLE;
    CEvent *p_event;
#endif
    HANDLE terminateEvent;
    std::string sampleChannelId;

private:
    //�����������
    nlohmann::json::array_t ids;
    std::int32_t sampleInterval;
    std::int32_t uploadInterval;
    std::string beginTime;

    //���������ռ�
    nlohmann::json::array_t datas;
    nlohmann::ordered_json responseJson;
    int idNum;
    int isClear;
};


#endif






