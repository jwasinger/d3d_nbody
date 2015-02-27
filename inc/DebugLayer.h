#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

#include<memory>

#include "InputController.h"
#include "Console.h"
#include "Renderer.h"
#include "MyMath.h"
#include "log.h"
#include "misc.h"
#include "common_include.h"

namespace Core
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
		
		RECT windowRect;
		DebugOptions debugOptions;

		ID3D11Buffer *axesVBuffer;

		ID3D11Buffer *test_tri_vbuffer;
		
		bool create_test_triangle(void);
		void render_test_triangle(void);
		void free_test_triangle(void);
	};
}
#endif