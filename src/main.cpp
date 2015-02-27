#include "common_include.h"
#include "NBodySim.h"
#include "Timer.h"
#include "InputController.h"
#include "DebugLayer.h"
#include "log.h"
#include "ProcSphere.h"
#include "A1.h"

using namespace Core;

HWND hWnd;
Timer time;

NBodySim *simulation;
Renderer *renderer;
InputController *inputController;
DebugLayer *debugLayer;

A1 *a1 = NULL;

const int windowPosX = 300;
const int windowPosY = 300;
const int windowWidth = 400;
const int windowHeight = 400;
static bool init = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void freeAppResources(void);
bool Init(WNDPROC wndProcFunction, HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow);
void createSimulationResources(void);
void Update(double elapsed);
void Render(void);

bool Init(WNDPROC wndProcFunction, HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	//create the logger first since everything forthecoming will depend on it
	/*if (!init_log(NULL))
		return false;*/

	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass1";

	// register the window class
	RegisterClassEx(&wc);

	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
							"WindowClass1",    // name of the window class
							"Our First Windowed Program",   // title of the window
							(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),    // window style
							windowPosX,    // x-position of the window
							windowPosY,    // y-position of the window
							windowWidth,    // width of the window
							windowHeight,    // height of the window
							NULL,    // we have no parent window, NULL
							NULL,    // we aren't using menus, NULL
							hInstance,    // application handle
							NULL);    // used with multiple windows, NULL

	if(!hWnd)
		return false;

	// display the window on the screen
	ShowWindow(hWnd, SW_MAXIMIZE);

	simulation = new NBodySim();
	renderer = new Renderer();
	a1 = new A1(renderer, 256, 256);

	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	inputController = new InputController(windowRect, renderer);
	if (!inputController->Init(hWnd))
		return false;

	if(!renderer->Init(windowRect, hWnd))
		return false;

	if (!a1->Init())
		return false;

	//renderer->HookDebug(options, &*inputController, &time);

	if(!simulation->Init(renderer->GetDeviceContext(), renderer->GetDevice(), 10))
		return false;

	DebugOptions options;
	options.RenderAxes = true;
	options.RenderFPS = true;
	options.RenderRawMouseInput = true;
	options.RenderMatrices = true;

	debugLayer = new DebugLayer();
	if(!debugLayer->Init(options, renderer, inputController, &time, windowRect)) // is the use of std::make_shared correct here?
	{
		freeAppResources();
		return false;
	}
	
	/*p_sphere = new ProcSphere();
	if (!p_sphere->Init(renderer))
		return false;
	
	if (!p_sphere->Generate(5))
		return false;
	
	p_sphere2 = new ProcSphere();
	if (!p_sphere2->Init(renderer))
		return false;

	if (!p_sphere2->Generate(0))
		return false;*/

	init = true;
	//ShellExecute(hWnd, "open", "DebugConsole.exe", NULL, NULL, 0);
	return true;
}

void createSimulationResources(void)
{
		
}

void Update(double elapsed)
{
	simulation->Tick(elapsed);
	inputController->Update(elapsed);
}

void Render(void)
{
	renderer->BeginRender();

	//renderer->RenderDebugInfo();
	//debugLayer->Render();
	
	/*renderer->SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));*/
 = 0/*	p_sphere->Render();*/
	a1->Render();

	/*renderer->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	p_sphere2->Render();*/

	//renderer->RenderParticles();

	renderer->EndRender();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if (!init_log(NULL))
		return -1;
	
	if(!Init(NULL, hInstance, hPrevInstance, pScmdline, iCmdshow))
	{
		return 1;
	}

	MSG msg = {0};

	while(WM_QUIT != msg.message)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//no messages waiting, step the simulation forward a frame
			if(time.Tick())
			{
				Update(time.GetElapsed());
				Render();
			}
		}
	}

    // return this part of the WM_QUIT message to Windows
    return msg.wParam;
}

void freeAppResources(void)
{
	SafeDelete(&debugLayer);
	SafeDelete(&inputController);
	SafeDelete(&renderer);
	SafeDelete(&simulation);
	close_log();
	return;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		freeAppResources();
		break;

	default:
		if (init)
			inputController->OnWndProc(message, wParam, lParam);
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}
