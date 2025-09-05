#pragma once
#include <nlohmann/json.hpp>
#include <map>

namespace tools {
	class PathParse {
	public:
		void rootParse(nlohmann::json);
		std::map<std::string, std::string> idPath;
		std::vector<nlohmann::json> sampleChannel;
		std::vector<nlohmann::json> flowcollectionChannel;
	private:
		void devicesParse(nlohmann::json deviceObject, nlohmann::json::json_pointer ptr);
		void componentsParse(nlohmann::json component, nlohmann::json::json_pointer ptr);
	};
	class TypeConversion {
	public:
		std::uint8_t* UINT8(std::string endianness, int rc, std::uint8_t* dest);
		std::uint16_t* UINT16(std::string endianness, int rc, std::uint8_t* dest);
		std::int16_t* INT16(std::string endianness, int rc, std::uint8_t* dest);
		std::uint32_t* UINT32(std::string endianness, int rc, std::uint8_t* dest);
		std::int32_t* INT32(std::string endianness, int rc, std::uint8_t* dest);
		std::uint64_t* UINT64(std::string endianness, int rc, std::uint8_t* dest);
		std::int64_t* INT64(std::string endianness, int rc, std::uint8_t* dest);
		float* FLOAT(std::string endianness, int rc, std::uint8_t* dest);
	};
	extern std::map<std::string, int> TypeConversionCode;
}
