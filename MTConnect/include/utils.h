#pragma once
#include <nlohmann/json.hpp> 
#include <pugixml/pugixml.hpp>
#include <string>

namespace utils
{
    nlohmann::json parseStreams(std::string data);
    std::string getTimeStamp();
    
    class tools{
        public:
        std::vector<nlohmann::json> flowcollectionChannel;
        void rootParse(nlohmann::json rootObject);
		std::map<std::string, std::string> idPath;
    };
}
