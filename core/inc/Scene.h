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
		virtual void Render(void);
		virtual bool Init(void);
		virtual void Release(void);
	};

	class Scene
	{
	public:
		Scene();
		~Scene();
		
		virtual void Render(void);

		void DrawScene(void);
		bool Init(void);

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