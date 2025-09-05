#ifndef ACTION_LISTENER
#define ACTION_LISTENER
#include <mqtt/iaction_listener.h>
#include <mqtt/async_client.h>
#include <iostream>


class ActionListener : public virtual mqtt::iaction_listener
{
    std::string _name;

    void on_failure(const mqtt::token& tok) override;

    void on_success(const mqtt::token& tok) override;

public:
    ActionListener(const std::string& name) : _name(name) {}
};
#endif // !ACTION_LISTENER
