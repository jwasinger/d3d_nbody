#ifndef APPLICATION_H
#define APPLICATION_H

#include "Scene.h"
#include "Window.h"
#include "InputController.h"

namespace Core
{
	class Application
	{
	private:

	public:
		Application();
		~Application();
		
		Scene *GetScene(void);
		InputController *GetInputController(void);
		RenderableWindow *GetWindow(void);
		Timer *GetTimer(void);

		void DebugStr(char *str, ...);

		virtual void Update(void);
	};
}

#endif