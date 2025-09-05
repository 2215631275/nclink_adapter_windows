#pragma once
#include <nlohmann/json.hpp>
#include <map>

namespace tools {
	class PathParse {
	public:
		void rootParse(nlohmann::json);
		/*����ӳ��NC-Link�ڵ�id���id��ģ���ļ��е�λ��
		* key:id
		* valueʾ��:/devices/0/dataItems/0
		*/
		std::map<std::string, std::string> idPath;
		/*�������µķ������󣬱����������ַ�Ȳ���
		* key:������
		* value:����ֵ
		*/
		std::map<std::string, std::string> methodParams;
		/*����ӳ��NC-Link�ڵ�id��opc ua�ڵ����������
		* key:id
		* value:identity
		*/
		std::map<std::string, nlohmann::json> idIdentity;
		//������
		std::pair<std::string, std::string> methodName;
		std::vector<nlohmann::json> sampleChannel;
		std::vector<nlohmann::json> flowcollectionChannel;
		/*�����ڵ㼯��
		*/
		std::vector<std::string> sampleId;

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
