#ifndef MEDIA_H
#define MEDIA_H

#include <string>

#include "misc.h"

std::wstring GetShaderPath(const std::wstring &shader_name);
std::wstring GetMediaPath(const std::wstring &file_name);

#endif