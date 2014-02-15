#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

#include<memory>

#include "InputController.h"
#include "Console.h"
#include "Renderer.h"

namespace NBody
{
	
	struct DebugOptions
	{
	public:
		bool RenderRawMouseInput;
		bool RenderFPS;
		bool RenderAxes;
		bool RenderMatrices;
	};

	class DebugLayer
	{
	public:
		DebugLayer();
		~DebugLayer();

		bool DebugLayer::Init(
			DebugOptions &options, 
			Renderer *renderer, 
			InputController *const inputController, 
			Timer *const timer,
			const RECT& windowRect);
	

		void Render(void);
		void OnMouseKeyInput(WPARAM wparam, LPARAM lparam);

	private:
		Console* console;
		Renderer* renderer;
		const Timer* timer;
		const InputController *inputController;

		DebugOptions debugOptions;

		ID3D11Buffer *axesVBuffer;

	};
}
#endif