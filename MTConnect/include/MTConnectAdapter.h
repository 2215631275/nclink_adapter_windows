#pragma once
#include "Config.h"
#include "DeviceModel.h"

#include <httplib/httplib.h>
#include <mqtt/async_client.h>


class MTConnectAdapter {
public:

	MTConnectAdapter();
	void init();
};
