#ifndef TOOLS
#define TOOLS
#include<iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <wchar.h>
/*
* @brief string����תutf-8����
* @param ������Ҫת����string
* @return ����utf-8�����string
*/
std::string s_utf8(const std::string& str);
/*
* @brief utf-8��������ת��ͨstring����
* @param ������Ҫת����string
* @return ������ͨ����string
*/
std::string utf8_s(const std::string& str);
#endif