#ifndef COMMON_INCLUDE_H
#define COMMON_INCLUDE_H

#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <memory>
#include <string>
#include <shellapi.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
};

struct RECTF
{
	float left, right, top, bottom;	
};

const std::wstring appPath = L"C:\\Users\\Jared\\Documents\\Visual Studio 2012\\Projects\\nbody_simulation\\nbody_simulation\\";
const std::wstring base_path = L"C:\\Users\\Jared\\Documents\\Visual Studio 2012\\Projects\\nbody_simulation\\nbody_simulation\\";
const std::wstring shader_path 
const std::wstring fontFileName = L"arial.spritefont";

inline std::wstring GetShaderPath(const std::wstring &shader_name)
{
	std::wstring rv = std::wstring(base_path).append(shader_path).append(shader_name);
	return rv;
}

inline std::wstring GetFilePath(const std::wstring &fileName)
{
	std::wstring tmp = std::wstring(appPath.data());
	return tmp.append(fileName.data());
}

namespace NBody
{
	class SimulationWindow;
}

extern NBody::SimulationWindow window;

#endif