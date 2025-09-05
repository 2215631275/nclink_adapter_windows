#ifndef CALL_BACK
#define CALL_BACK
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include "ActionListener.h"


class CallBack : public virtual mqtt::callback,
	public virtual mqtt::iaction_listener

{
	int _nretry;
	// The MQTT client
	
	// Options to use if we need to reconnect
	mqtt::connect_options& _connOpts;
	// An action listener to display the result of actions.
	

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect();

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override;

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override;

	// (Re)connection success
	void connected(const std::string& cause) override;

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override;

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override;

	void delivery_complete(mqtt::delivery_token_ptr token) override;

public:
	ActionListener subListener;
	mqtt::async_client& cli;

	CallBack(mqtt::async_client & client, mqtt::connect_options& connOpts) : _nretry(0), cli(client), _connOpts(connOpts), subListener("subListener") {}
};
#endif // !CALL_BACK
