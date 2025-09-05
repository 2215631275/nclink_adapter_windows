#include "Config.h"

Config::Config() {}


// �����µ�·�������¶�ȡ�����ļ�
void Config::setPath(std::string path)
{
	configPath = path;
	std::ifstream file(configPath);
	config = nlohmann::json::parse(file);
}
