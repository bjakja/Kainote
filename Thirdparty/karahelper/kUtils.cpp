// some utils
#include "stdafx.h"

std::string LowerCase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower); // to lower case
    return str;
};

std::wstring LowerCase(std::wstring str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower); // to lower case
    return str;
};

// wstring <= string
std::wstring _SW(const std::string &str, UINT cp)
{
    // Convert an ASCII string to a Unicode String
    std::wstring wstrTo;
    wchar_t *wszTo = new wchar_t[str.length() + 1];
    wszTo[str.size()] = L'\0';
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wszTo, (int)str.length());
    wstrTo = wszTo;
    delete[] wszTo;
    return wstrTo;
}

// string <= wstring
std::string _WS(const std::wstring &wstr, UINT cp)
{
    // Convert a Unicode string to an ASCII string
    std::string strTo;
    char *szTo = new char[wstr.length() + 1];
    szTo[wstr.size()] = '\0';
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
    strTo = szTo;
    delete[] szTo;
    return strTo;
}

void SaveMemory(std::vector<char> &dst, const std::wstring wstr)
{
    size_t targetSize = wstr.length();
    std::string str = _WS(wstr);
    // Init
    dst.clear();
    dst.reserve(targetSize);

    // Write
    for(size_t i = 0; i < targetSize; i++)
        dst.push_back(str[i]);
}

// format std string
#include <stdio.h>
#include <stdarg.h>
std::wstring format_arg_list(const char *fmt, va_list args)
{
    if(!fmt) return L"";
    int   result = -1, length = 256;
    char *buffer = 0;
    while(result == -1)
    {
        if(buffer) delete [] buffer;
        buffer = new char [length + 1];
        memset(buffer, 0, length + 1);
        result = _vsnprintf(buffer, length, fmt, args);
        length *= 2;
    }
    std::string s(buffer);
    std::wstring ws = _SW(s);
    delete [] buffer;
    return ws;
}

std::wstring format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::wstring s = format_arg_list(fmt, args);
    va_end(args);
    return s;
}

int hextoint(std::wstring str)
{
    str = LowerCase(str);
    int ret = str.length() > 2 && (str.substr(0, 2) == L"&h" || str.substr(0, 2) == L"0x")
              ? str = str.substr(2), wcstol(str.c_str(), NULL, 16) : wcstol(str.c_str(), NULL, 10);

    return(ret);
}

void memsetd(void* dst, unsigned int c, int nbytes)
{
#ifdef WIN64
    for(int i = 0; i < nbytes / sizeof(DWORD); i++)
        ((DWORD*)dst)[i] = c;
#else
    __asm
    {
        mov eax, c
        mov ecx, nbytes
        shr ecx, 2
        mov edi, dst
        cld
        rep stosd
    }
#endif
}

std::wstring get_s(std::wstring& buff, std::wstring sep)
{
    buff = trimLeft(buff, L" ");
    ptrdiff_t pos = buff.find(sep);
    std::wstring ret;
    if(pos < 0)
    {
        ret = buff;
        buff.clear();
    }
    else
    {
        ret = buff.substr(0, pos);
        if(pos < (ptrdiff_t)buff.length()) buff = buff.substr(pos + 1);
    }
    return(ret);
}

int get_i(std::wstring& buff, std::wstring sep)
{
    std::wstring str = get_s(buff, sep);
    LowerCase(str);
    int ret = str.length() > 2 && (str.substr(0, 2) == L"&h" || str.substr(0, 2) == L"0x")
              ? str = str.substr(2), wcstol(str.c_str(), NULL, 16) : wcstol(str.c_str(), NULL, 10);

    return(ret);
}

double get_f(std::wstring& buff, std::wstring sep)
{
    return(wcstod(get_s(buff, sep).c_str(), NULL));
}

inline std::wstring trimLeft(std::wstring buff, std::wstring sym)
{
    ptrdiff_t i = buff.find_first_not_of(sym);
    buff.erase(0, i);
    return buff;
}