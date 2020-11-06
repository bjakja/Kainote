// Utils
#pragma once
#include "stdafx.h"

typedef std::vector<std::wstring> nStrArray;

std::wstring _SW(const std::string &str, UINT cp = CP_UTF8);
std::string _WS(const std::wstring &wstr, UINT cp = CP_ACP);

std::string LowerCase(std::string str);
std::wstring LowerCase(std::wstring str);

void SaveMemory(std::vector<char> &dst, const std::wstring wstr);

std::wstring format(const char *fmt, ...);

int hextoint(std::wstring str);
void memsetd(void* dst, unsigned int c, int nbytes);

std::wstring get_s(std::wstring& buff, std::wstring sep = L",");
int get_i(std::wstring& buff, std::wstring sep = L",");
double get_f(std::wstring& buff, std::wstring sep = L",");

std::wstring trimLeft(std::wstring buff, std::wstring sym);