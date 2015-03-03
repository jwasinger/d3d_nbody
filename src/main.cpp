#include "common_include.h"
#include "NBodySim.h"
#include "Timer.h"
#include "InputController.h"
#include "DebugLayer.h"
#include "log.h"
#include "ProcSphere.h"
#include "RayTracer.h"

using namespace Core;

HWND hWnd;
Timer time;

Renderer *renderer;
InputController *inputController;
DebugLayer *debugLayer;
RayTracer *ray_tracer;

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
void init_ray_tracer_scene(void);

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

	renderer = new Renderer();
	
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	inputController = new InputController(windowRect, renderer);
	if (!inputController->Init(hWnd))
		return false;

	if(!renderer->Init(windowRect, hWnd))
		return false;

	ray_tracer = new RayTracer(renderer);
	if (!ray_tracer->Init())
		return false;
	
	ray_tracer->Run();

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

	init_ray_tracer_scene();
	init = true;

	return true;
}

void init_ray_tracer_scene(void)
{
	Material material;
	material.ambient_color = Vector3(1.0f, 0.0f, 0.0f);
	material.ambient_intensity = 0.7f;
	material.reflectivity = 0.7f;

	int index = ray_tracer->AddMaterial(material);

	Sphere s;
	s.material_index = index;
	s.pos = Vector3(0.0f, 0.0f, -10.0f);
	s.radius = 1.0f;

	ray_tracer->AddSphere(s);
}

void Update(double elapsed)
{
	inputController->Update(elapsed);
}

void Render(void)
{
	renderer->BeginRender();

	ray_tracer->Run();

	ray_tracer->Render();

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
