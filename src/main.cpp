#include "common_include.h"
#include "NBodySim.h"
#include "Timer.h"
#include "InputController.h"
#include "DebugLayer.h"

using namespace NBody;

HWND hWnd;
Timer time;

NBodySim *simulation;
Renderer *renderer;
InputController *inputController;
DebugLayer *debugLayer;

const int windowPosX = 300;
const int windowPosY = 300;
const int windowWidth = 400;
const int windowHeight = 400;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void freeAppResources(void);
bool Init(WNDPROC wndProcFunction, HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow);
void createSimulationResources(void);
void Update(double elapsed);
void Render(void);

bool Init(WNDPROC wndProcFunction, HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
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
	
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	inputController = new InputController(windowRect, renderer);

	if(!renderer->Init(windowRect, hWnd))
		return false;

	//renderer->HookDebug(options, &*inputController, &time);

	if(!simulation->Init(renderer->GetDeviceContext(), renderer->GetDevice(), 10))
		return false;

	if(!inputController->Init(hWnd))
		return false;
	
	/*console = std::shared_ptr<Console>(new NBody::Console());
	if(!console->Init(
		renderer, 
		simulation, 
		Vector2(0.0f, 0.0f), 
		Vector2(0.5f, 0.5f), 
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top))
	{
		return false;
	}*/

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
	
	ShellExecute(hWnd, "open", "DebugConsole.exe", NULL, NULL, 0);

	
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
	debugLayer->Render();
	
	//renderer->RenderParticles();

	renderer->EndRender();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
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
	delete debugLayer;
	delete inputController;
	delete renderer;
	delete simulation;
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
		inputController->OnWndProc(message, wParam, lParam);
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}