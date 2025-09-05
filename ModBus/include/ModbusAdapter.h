#ifndef MTCONNECT_ADAPTER
#define MTCONNECT_ADAPTER
#include "Config.h"
#include "DeviceModel.h"

#include <httplib/httplib.h>
#include <mqtt/async_client.h>


class ModbusAdapter {
public:

	ModbusAdapter();
	void init();
};

#endif
