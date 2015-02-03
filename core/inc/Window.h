#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

namespace Core
{
	class RenderableWindow
	{
	public:
		RenderableWindow(HINSTANCE instance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow);
		~RenderableWindow();
		
		bool Init(void);
		void Release(void);

	private:
		LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	private:
		HINSTANCE instance, prevInstance;
		PSTR pSCmdline;
		int iCmdShow;
	};
}
#endif