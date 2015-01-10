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
#include "misc.h"

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

#endif