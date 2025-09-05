#ifndef DEVICE_MODEL
#define DEVICE_MODEL
#include <pugixml/pugixml.hpp>
#include <nlohmann/json.hpp>

#include "ModbusAdapter.h"


class DeviceModel
{
public:
	pugi::xml_document doc;
	pugi::xml_parse_result xmlModel;
	nlohmann::json jsonModel;

	DeviceModel();
	void parse(std::string data);
};

#endif