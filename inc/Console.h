#include<SimpleMath.h>
#include<SpriteBatch.h>
#include<SpriteFont.h>

#include "Renderer.h"
#include "NBodySim.h"

#ifndef CONSOLE_H
#define CONSOLE_H

using namespace DirectX::SimpleMath;
using namespace DirectX;

namespace Core
{
	//Goals: 
	//allow the developer to interact with the simulation via a command-line interface
	class Console
	{
	private:
		int windowSizeX;
		int windowSizeY;
		Vector2 consoleSize;

		Vector2 position;
		int fontSize;

		Color backgroundColor;
		Color textColor;

		NBody::Renderer *renderer;

		ID3D11BlendState *transparentBS;
		ID3D11Buffer *quadVBuff;
		ID3D11Buffer *colorCBuff;
		float color[4];
		Vector4 verts[6];

		RECT screenRect;

	private:
		void calcScreenRect(void);

	public:
		Console(void);
		~Console(void);

		bool Init(Renderer *renderer, 
				  const Vector2 & pos,
				  const Vector2 & size, 
				  int screenSizeX,
				  int screenSizeY);

		void Render(void);

		void Update(double elapsedMS);

		void KeyDown(void);
		void KeyUp(void);

		void OnFocusLost(void);
		void OnFocusGained(void);
		
		void SetPos(const Vector2 &pos);
		void SetSize(const Vector2 &size);
	};
}
#endif
