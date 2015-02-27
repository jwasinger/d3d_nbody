#include "Window.h"

namespace Core
{
	RenderableWindow::RenderableWindow(HINSTANCE instance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow)
	{
		this->instance = instance;
		this->prevInstance = hPrevInstance;
		this->pSCmdline = pScmdline;
		this->iCmdShow = iCmdShow;
	}

}