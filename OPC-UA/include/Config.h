#ifndef CONFIG
#define CONFIG
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>


class Config
{
private:
	std::string configPath;
	
public:
	nlohmann::json config;
	nlohmann::json opcconfig;
	std::string dev_uuid;
	Config();
	void setPath(std::string path);
	void setDevuuid(std::string uuid);

};
#endif // !CONFIG
