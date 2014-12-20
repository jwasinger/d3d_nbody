#ifndef MISC_H
#define MISC_H

#include <vector>
#include <Windows.h>
#include <comdef.h>

std::vector<std::wstring> split(const std::wstring &str, wchar_t delim);
std::vector<std::string> split(const std::string &str, char delim);

std::vector<std::wstring> split_no_remove(const std::wstring &str, wchar_t delim);
std::vector<std::string> split_no_remove(const std::string &str, char delim);

std::string get_err_str(HRESULT hres);

#endif