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
	
	ray_tracer = new RayTracer(256, 256, renderer);
	if (!ray_tracer->Init())
		return false;

	init_ray_tracer_scene();
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

	init = true;

	return true;
}

void black_hole_scene(void)
{
	Sphere s1;
	Sphere s2;
	Sphere s3;

	Material m1;
	m1.ambient_color = Vector3(1.0f, 0.0f, 0.0f);
	m1.ambient_intensity = 0.3f;
	m1.reflectivity = 0.5f;
	int i1 = ray_tracer->AddMaterial(m1);

	Material m2;
	m2.ambient_color = Vector3(0.5f, 0.5f, 0.0f);
	m2.ambient_intensity = 0.7f;
	m2.reflectivity = 0.5f;
	int i2 = ray_tracer->AddMaterial(m2);

	s1.material_index = i1;
	s1.pos = Vector3(0.0f, 0.0f, -6.0f);
	s1.radius = 1.0f;

	/*
	s2.material_index = i1;
	s2.pos = Vector3(-0.5f, -0.5f, -5.0f);
	s2.radius = 0.5f;
	*/

	s3.material_index = i2;
	s3.pos = Vector3(-1.0f, -1.0f, -4.3f);
	s3.radius = 0.5f;

	ray_tracer->AddSphere(s1);
	//ray_tracer->AddSphere(s2);
	ray_tracer->AddSphere(s3);

	Light l;
	l.type = DIRECTIONAL_LIGHT;
	l.pos_direction = Vector3(0.0f, 1.0f, -1.0f);
	l.pos_direction.Normalize();
	l.diffuse_intensity = 0.8f;

	ray_tracer->AddLight(l);
}

void ray_tracer_main_scene(void)
{
	Material m1;
	m1.ambient_color = Vector3(1.0f, 0.5f, 0.0f);
	m1.ambient_intensity = 0.3f;
	m1.reflectivity = 0.4f;

	Material m2;
	m2.ambient_color = Vector3(0.0f, 1.0f, 0.0f);
	m2.ambient_intensity = 0.3f;
	m2.reflectivity = 0.4f;

	int index1 = ray_tracer->AddMaterial(m1);
	int index2 = ray_tracer->AddMaterial(m2);


	Sphere s1;
	s1.material_index = index1;
	s1.pos = Vector3(0.5f, 0.0f, -14.5f);
	s1.radius = 0.4f;

	Sphere s2;
	s2.material_index = index2;
	s2.pos = Vector3(-1.0f, 0.0f, -15.0f);
	s2.radius = 0.5f;


	Triangle t1;
	t1.a = Vector3(0.0f, 0.5f, -7.0f);
	t1.b = Vector3(0.5f, 0.0f, -5.0f);
	t1.c = Vector3(0.0f, 0.0f, -3.0f);
	Vector3 up = Vector3(0.0f, 0.3f, 0.0f);
	t1.a += up;
	t1.b += up;
	t1.c += up;

	t1.material_index = index1;

	Triangle t2;
	t2.a = Vector3(-1.0f, 0.0f, -3.0f);
	t2.b = Vector3(-1.0f, 1.0f, -5.0f);
	t2.c = Vector3(0.0f, 1.0f, -7.0f);
	t2.material_index = index2;

	Triangle t3;
	t3.a = Vector3(0.0f, 0.5f, -2.0f);
	t3.b = Vector3(0.5f, 0.0f, -5.0f);
	t3.c = Vector3(0.0f, 0.0f, -9.0f);
	t3.a += -2 * up;
	t3.b += -2 * up;
	t3.c += -2 * up;
	t3.material_index = index2;


	Triangle t4;
	t4.a = Vector3(-1.0f, -1.0f, -22.0f);
	t4.b = Vector3(-1.0f, 1.0f, -18.0f);
	t4.c = Vector3(1.0f, -1.0f, -15.0f);

	t4.material_index = index1;

	Light l1;
	l1.diffuse_intensity = 0.7f;
	l1.type = DIRECTIONAL_LIGHT;
	l1.pos_direction = Vector3(-1.0f, 0.0f, 0.0f);
	l1.pos_direction.Normalize();
	l1.spec_coefficient = 0.02f;

	ray_tracer->AddLight(l1);

	ray_tracer->AddTriangleCC(t1);
	ray_tracer->AddTriangleCC(t2);
	ray_tracer->AddTriangleCC(t3);
	ray_tracer->AddTriangleCC(t4);

	ray_tracer->AddSphere(s1);
	ray_tracer->AddSphere(s2);
}

void ray_tracer_scene_2(void)
{
	Sphere s1;
	Sphere s2;
	Sphere s3;
	Sphere s4;

	Material m1;
	Material m2;
	Material m3;
	Material m4;

	int i1, i2, i3, i4;

	m1.ambient_color = Vector3(1.0f, 0.0f, 0.0f);
	m1.ambient_intensity = 0.2f;
	m1.reflectivity = 0.5f;

	m2 = m1;
	m2.ambient_color = Vector3(0.0f, 1.0f, 0.0f);
	
	m3 = m2;
	m3.ambient_color = Vector3(0.0f, 0.0f, 1.0f);

	m4 = m3;
	m4.ambient_color = Vector3(0.5f, 0.5f, 0.5f);

	i1 = ray_tracer->AddMaterial(m1);
	i2 = ray_tracer->AddMaterial(m2);
	i3 = ray_tracer->AddMaterial(m3);
	i4 = ray_tracer->AddMaterial(m4);

	s1.material_index = i1;
	s1.pos = Vector3(1.0f, 1.0f, -5.0f);
	s1.radius = 0.5f;
	
	s2.material_index = i2;
	s2.pos = Vector3(1.0f, -1.0f, -5.0f);
	s2.radius = 0.5f;

	s3.material_index = i3;
	s3.pos = Vector3(-1.0f, -1.0f, -5.0f);
	s3.radius = 0.5f;

	s4.material_index = i4;
	s4.pos = Vector3(-1.0f, 1.0f, -5.0f);
	s4.radius = 0.5f;
	
	ray_tracer->AddSphere(s1);
	ray_tracer->AddSphere(s2);
	ray_tracer->AddSphere(s3);
	ray_tracer->AddSphere(s4);

	Light l;
	l.type = DIRECTIONAL_LIGHT;
	l.pos_direction = Vector3(0.0f, 1.0f, -1.0f);
	l.pos_direction.Normalize();
	l.diffuse_intensity = 0.8f;

	ray_tracer->AddLight(l);
}

void init_ray_tracer_scene(void)
{
	//ray_tracer_scene_2();
	//ray_tracer_main_scene();
	black_hole_scene();
}

void Update(double elapsed)
{
	inputController->Update(elapsed);
}

void Render(void)
{
	renderer->BeginRender();

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
