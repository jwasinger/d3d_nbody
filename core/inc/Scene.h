#ifndef SCENE_H
#define SCENE_H

#include "Camera3D.h"
#include "Renderer.h"
#include <vector>

namespace Core
{
	class IRenderable
	{
	public:
		virtual void Render(void) = 0;
		virtual bool Init(void) = 0;
		virtual void Release(void) = 0;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();
		
		virtual void Render(void) = 0;

		void DrawScene(void);
		bool Init(RECT window_rect, HWND window_hwnd);
    
    Renderer *const GetRenderer(void) { return this->renderer; }
    Camera *const GetCamera(void) { return &(this->camera); }

	private:
		void begin_draw(void);
		void end_draw(void);

	private:
		std::vector<IRenderable> objects;
		Camera3D camera;
		Renderer *renderer;
	};

}
#endif
