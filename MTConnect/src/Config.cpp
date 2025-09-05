#include "Config.h"

Config::Config() {}


// 设置新的路径，重新读取配置文件
void Config::setPath(std::string path)
{
	configPath = path;
	std::ifstream file(configPath);
	config = nlohmann::json::parse(file);
}