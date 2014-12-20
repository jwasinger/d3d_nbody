#include "misc.h"

std::vector<std::wstring> split_no_remove(const std::wstring &str, wchar_t delim)
{
	std::vector<std::wstring> elems;
	std::wstring cur_str;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == delim)
		{
			elems.push_back(cur_str);
			cur_str = std::wstring();
			cur_str.push_back(str[i]);
		}
		else
		{
			cur_str.push_back(str[i]);
		}
	}

	elems.push_back(cur_str);
	return elems;
}

std::vector<std::string> split_no_remove(const std::string &str, char delim)
{
	std::vector<std::string> elems;
	std::string cur_str;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == delim)
		{
			elems.push_back(cur_str);
			cur_str = std::string();
			cur_str.push_back(str[i]);
		}
		else
		{
			cur_str.push_back(str[i]);
		}
	}

	elems.push_back(cur_str);
	return elems;
}


std::vector<std::wstring> split(const std::wstring &str, wchar_t delim)
{
	std::vector<std::wstring> elems;
	std::wstring cur_str;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == delim)
		{
			elems.push_back(cur_str);
			cur_str = std::wstring();
		}
		else
		{
			cur_str.push_back(str[i]);
		}
	}

	elems.push_back(cur_str);
	return elems;
}

std::vector<std::string> split(const std::string &str, char delim)
{
	std::vector<std::string> elems;
	std::string cur_str;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == delim)
		{
			elems.push_back(cur_str);
			cur_str = std::string();
		}
		else
		{
			cur_str.push_back(str[i]);
		}
	}

	elems.push_back(cur_str);
	return elems;
}

std::string get_err_str(HRESULT hres)
{
	_com_error err(hres);
	return std::string(err.ErrorMessage());
}