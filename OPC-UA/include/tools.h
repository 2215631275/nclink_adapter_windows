#ifndef TOOLS
#define TOOLS
#include<iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <wchar.h>
/*
* @brief string类型转utf-8编码
* @param 输入需要转换的string
* @return 返回utf-8编码的string
*/
std::string s_utf8(const std::string& str);
/*
* @brief utf-8编码类型转普通string类型
* @param 输入需要转换的string
* @return 返回普通类型string
*/
std::string utf8_s(const std::string& str);
#endif