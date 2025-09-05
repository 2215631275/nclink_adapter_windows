#include "DeviceModel.h"
#include <plog/Log.h>

extern Config config;

DeviceModel::DeviceModel() {};

// 解析数据到jsonModel
void DeviceModel::parse(std::string data)
{
	xmlModel = doc.load_string(data.c_str());
	jsonModel["id"] = config.config["model_config"]["model_id"];
	jsonModel["name"] = config.config["model_config"]["model_name"];
	jsonModel["type"] = config.config["model_config"]["model_type"];
	jsonModel["description"] = config.config["model_config"]["description"];
	jsonModel["version"] = config.config["model_config"]["version"];
	jsonModel["devices"] = nlohmann::json::array({});
	pugi::xml_node devices = doc.child("MTConnectDevices").child("Devices");
	PLOG_DEBUG << devices.child("Device").attribute("id").value();
	for (pugi::xml_node xmldevice = devices.child("Device"); xmldevice; xmldevice = xmldevice.next_sibling())
	{
		nlohmann::json jsonDevice;
		jsonDevice["id"] = xmldevice.attribute("id").value();
		jsonDevice["type"] = "MACHINE";
		jsonDevice["name"] = xmldevice.attribute("name").value();
		jsonDevice["description"] = xmldevice.child_value("Description");
		jsonDevice["number"] = xmldevice.attribute("uuid").value();
		jsonDevice["guid"] = xmldevice.attribute("uuid").value();
		jsonDevice["version"] = config.config["model_config"]["model_id"];
		jsonDevice["configs"] = nlohmann::json::array({});
		jsonDevice["dataItems"] = nlohmann::json::array({});
		jsonDevice["components"] = nlohmann::json::array({});
		
        nlohmann::json sampleObj;
        sampleObj["id"] = "sample01";
        sampleObj["name"] = "sample object";
        sampleObj["type"] = "SAMPLE_CHANNEL";
        sampleObj["description"] = "sample channel";
        sampleObj["ids"] = nlohmann::json::array({});
        sampleObj["sampleInterval"] = 1000;
        sampleObj["uploadInterval"] = 1000;
		
        pugi::xml_node xmlDataItems = xmldevice.child("DataItems");
		for (pugi::xml_node xmlDataItem : xmlDataItems)
		{
			nlohmann::json jsonDataItem;
			jsonDataItem["id"] = xmlDataItem.attribute("id").value();
			jsonDataItem["type"] = xmlDataItem.attribute("type").value();
			jsonDevice["dataItems"].push_back(jsonDataItem);
            
            // sample channel ids
            nlohmann::json jsonID;
            jsonID["id"] = xmlDataItem.attribute("id").value();
            sampleObj["ids"].push_back(jsonID);
		}

		pugi::xml_node xmlComponents = xmldevice.child("Components");
		// Components
		for (pugi::xml_node xmlComponent : xmlComponents)
		{
			nlohmann::json jsonComponent;
			jsonComponent["id"] = xmlComponent.attribute("id").value();
			jsonComponent["type"] = xmlComponent.name();
			jsonComponent["name"] = xmlComponent.attribute("name").value();
			jsonComponent["description"] = "";
			jsonComponent["number"] = "";
			jsonComponent["configs"] = nlohmann::json::array({});
			jsonComponent["dataItems"] = nlohmann::json::array({});
			jsonComponent["components"] = nlohmann::json::array({});
            // dataItem
			for (pugi::xml_node xmlCDataItem : xmlComponent.child("DataItems"))
            {
                nlohmann::json jsonCDataItem;
                jsonCDataItem["id"] = xmlCDataItem.attribute("id").value();
                jsonCDataItem["type"] = xmlCDataItem.attribute("type").value();
                jsonComponent["dataItems"].push_back(jsonCDataItem);

                // sample channel ids
                nlohmann::json jsonID;
                jsonID["id"] = xmlCDataItem.attribute("id").value();
                sampleObj["ids"].push_back(jsonID);
            }

			// component 
			for (pugi::xml_node xmlCComponent : xmlComponent.child("Components"))
			{
				nlohmann::json jsonCComponent;
				jsonCComponent["id"] = xmlCComponent.attribute("id").value();
				jsonCComponent["name"] = xmlCComponent.attribute("name").value();
				jsonCComponent["type"] = xmlCComponent.name();
				jsonCComponent["number"] = "";
				jsonCComponent["description"] = "";
				jsonCComponent["dataItems"] = nlohmann::json::array({});

				// dataItems
				for (pugi::xml_node xmlDataItem : xmlCComponent.child("DataItems"))
				{
					nlohmann::json jsonDataItem;
					jsonDataItem["id"] = xmlDataItem.attribute("id").value();
					jsonDataItem["name"] = xmlDataItem.attribute("name").value();
					jsonDataItem["type"] = xmlDataItem.attribute("type").value();
					jsonDataItem["description"] = xmlDataItem.attribute("name").value();
					jsonDataItem["datatype"] = "string";
					jsonDataItem["value"] = "";
					jsonDataItem["source"] = "";
					if (xmlDataItem.child("Source"))
					{
						jsonDataItem["source"] = xmlDataItem.child("Source").text().as_string();
					}
					jsonDataItem["units"] = xmlDataItem.attribute("units").value();
					jsonDataItem["setable"] = false;
					
					jsonCComponent["dataItems"].push_back(jsonDataItem);
                    
                    // sample channel ids
                    nlohmann::json jsonID;
                    jsonID["id"] = xmlDataItem.attribute("id").value();
                    sampleObj["ids"].push_back(jsonID);
				}

				
				jsonComponent["components"].push_back(jsonCComponent);
				
			}
			jsonDevice["components"].push_back(jsonComponent);

		}
        jsonDevice["configs"].push_back(sampleObj);
		jsonModel["devices"].push_back(jsonDevice);
	}
	PLOG_DEBUG << "model:" << jsonModel.dump();
}
