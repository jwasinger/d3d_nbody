#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <memory>
#include <string>
#include <shellapi.h>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>
#include <vector>
#include <comdef.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
};

template <class T> void SafeDelete(T **ppT)
{
	if (*ppT)
	{
		delete(*ppT);
		*ppT = nullptr;
	}
};

struct RECTF
{
	float left, right, top, bottom;	
};

const std::wstring base_path = L"C:\\Users\\Jared\\projects\\d3d_nbody\\";
const std::wstring shader_path = L"shaders\\";
const std::wstring media_path = L"media\\";
const std::wstring fontFileName = L"arial.spritefont";

inline std::wstring GetShaderPath(const std::wstring &shader_name)
{
	std::wstring rv = std::wstring(base_path).append(shader_path).append(shader_name);
	//C:\Users\Jared\projects\d3d_nbody\shaders
	return rv;
}

inline std::vector<std::wstring> split(const std::wstring &str, wchar_t delim)
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

inline std::wstring GetMediaPath(const std::wstring &file_name)
{
	std::wstring file_type = split(file_name, '.').back();
	
	//only support bmp files for now
	if (file_type == std::wstring(L"bmp"))
	{
		return std::wstring(base_path).append(media_path).append(L"\\images\\").append(file_name);
	}
	else if (file_type == std::wstring(L"spritefont"))
	{
		return std::wstring(base_path).append(media_path).append(L"\\fonts\\").append(file_name);
	}
	else
	{
		return std::wstring(); //file type not recognized
	}
}

inline std::string get_err_str(HRESULT hres)
{
	_com_error err(hres);
	return std::string(err.ErrorMessage());
}

namespace NBody
{
	class SimulationWindow;
}

extern NBody::SimulationWindow window;

#endif