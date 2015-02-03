#include "media.h"

static const std::wstring base_path = L".\\";//L"C:\\Users\\Jared\\projects\\d3d_nbody\\";
static const std::wstring shader_path = L"shaders\\";
static const std::wstring media_path = L"media\\";
static const std::wstring fontFileName = L"arial.spritefont";

std::wstring GetShaderPath(const std::wstring &shader_name)
{
	std::wstring rv = std::wstring(base_path).append(shader_path).append(shader_name);
	//C:\Users\Jared\projects\d3d_nbody\shaders
	return rv;
}

std::wstring GetMediaPath(const std::wstring &file_name)
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