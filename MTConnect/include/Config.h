#pragma once
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

	Config();
	void setPath(std::string path);

};
