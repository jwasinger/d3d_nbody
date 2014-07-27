#include "InputController.h"

namespace NBody
{
	InputController::InputController(void)
	{
		this->renderer = nullptr;
		ZeroMemory(&this->windowRect, sizeof(RECT));

		this->mouseDamping = 1.0f;
	}

	InputController::InputController(const RECT &windowRect, Renderer *renderer)
	{
		this->renderer = renderer;
		this->windowRect = windowRect;
		this->windowWidth = windowRect.right - windowRect.left;
		this->windowHeight = windowRect.bottom - windowRect.top;

		this->mouseDamping = 0.01f;
		this->cameraMoveSpeed = 2.0f;

		this->screenOriginX = 0;
		this->screenOriginY = 0;
	}

	bool InputController::Init(HWND hWnd)
	{
		this->cameraMode = CAMERA_MODE_STATIC_CAMERA;
		this->motionState = MOTION_STATE_STATIONARY;
		RAWINPUTDEVICE mouseRID[1];
		mouseRID[0].usUsagePage = 0x01;
		mouseRID[0].usUsage = 0x02;
		mouseRID[0].dwFlags = 0;
		mouseRID[0].hwndTarget = NULL;
		
		if(!RegisterRawInputDevices(mouseRID, 1, sizeof(mouseRID[0])))
			return false;

		return true;
	}

	InputController::~InputController(void) { }

	void InputController::readRawInput(LPARAM lParam)
	{
		UINT dwSize;

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, 
						sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL) 
		{
			throw RuntimeError();
		} 	

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
		{
			throw RuntimeError("GetRawInputData does not return correct size !");
		}
		
		RAWINPUT *raw = (RAWINPUT *)lpb;

		if(raw->header.dwType == RIM_TYPEMOUSE)
		{
			this->FPSMouseUpdate(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
		}

		delete lpb;
	}

	void InputController::OnWndProc(UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch(message)
		{
		case WM_MOVE:
			{
				RECT newWindowRect;
				newWindowRect.left = GET_X_LPARAM(lparam);
				newWindowRect.top = GET_Y_LPARAM(lparam);
				newWindowRect.right = newWindowRect.left + this->windowWidth;
				newWindowRect.bottom = newWindowRect.top + this->windowHeight;

				this->windowRect = newWindowRect;
				break;
			}
		case WM_SIZE:
			{
				this->windowRect.right = this->windowRect.left + LOWORD(lparam);
				this->windowRect.bottom = this->windowRect.top + HIWORD(lparam);

				this->windowHeight = HIWORD(lparam);
				this->windowWidth = LOWORD(lparam);
				break;
			}
		case WM_KEYDOWN:
			{
				this->onKeyDown(wparam);
				break;
			}
		case WM_KEYUP:
			{
				this->onKeyUp(wparam);
				break;
			}
		case WM_INPUT:
			{
				if(this->cameraMode == CAMERA_MODE_FPS_CAMERA)
				{
					this->readRawInput(lparam);

					int newCursorPosX = this->windowRect.left + (int)(0.5f * (float)(this->windowWidth));
					int newCursorPosY = this->windowRect.top + (int)(0.5f * (float)(this->windowHeight));

					SetCursorPos(
						newCursorPosX,
						newCursorPosY);
				}
				break;
			}
		case WM_MOUSEMOVE:
			{
				if(this->cameraMode == CAMERA_MODE_STATIC_CAMERA)
				{
					
				}

				break;
			}
		}
	}

	void InputController::FPSMouseUpdate(int x, int y)
	{
		/*if(x > 0)
		{
			x = 1;
			y = -1;
		}
		else
		{
			x = -1;
			y = 1;
		}*/


		/*if( x > -5 && x < 5)
		{
			x = 0;
		}
		
		if( y > -5 && y < 5)
		{
			y = 0;
		}*/

		if(x == 0 && y == 0)
		{
			return;
		}
		
		this->mouseDeltaX = x;
		this->mouseDeltaY = y;

		Vector3 mouseDelta(this->mouseDeltaX, -this->mouseDeltaY, 0.0f);

		/*Matrix mouseDeltaMat = Matrix::Identity();
		mouseDeltaMat.m[3][0] = mouseDelta.x;
		mouseDeltaMat.m[3][1] = mouseDelta.y;
		mouseDeltaMat.m[3][2] = mouseDelta.z;
		mouseDeltaMat.m[3][3] = mouseDelta.w;*/

		//transform the mouse delta vector from view space to world space coords
		//mouseDeltaMat = mouseDeltaMat * this->camera->GetInvView();
		
		//transform the mouse delta vector from view space to world space coords
		mouseDelta = Vector3::TransformNormal(mouseDelta, this->renderer->GetInvTransform(TRANSFORM_VIEW));

		Vector3 rotationDir = mouseDelta.Cross(this->renderer->GetTransform(TRANSFORM_VIEW).Forward());
		rotationDir.Normalize();

		float rotAngle = rotationDir.Length() * this->mouseDamping;
		
		Quaternion rot = Quaternion::CreateFromAxisAngle(rotationDir, rotAngle);
		Matrix rotMat = Matrix::CreateFromQuaternion(rot);
		//Matrix rotMat = Matrix::CreateFromAxisAngle(rotationDir, rotAngle);
		//this->renderer->CameraRotateInWorld(rotMat);
	}

	void InputController::Update(double ms)
	{
		Matrix translation = Matrix::Identity();
		float dist = (ms/1000.0)*this->cameraMoveSpeed;

		switch(this->motionState)
		{
		case MOTION_STATE_FORWARD:
			
			//this->renderer->CameraTranslateInView(Vector3(0.0f, 0.0f, -dist));
			break;
		case MOTION_STATE_LEFT:

			//this->renderer->CameraTranslateInView(Vector3(-dist, 0.0f, 0.0f));

			break;
		case MOTION_STATE_RIGHT:

			//this->renderer->CameraTranslateInView(Vector3(dist, 0.0f, 0.0f));
			break;
		case MOTION_STATE_BACK:

			//this->renderer->CameraTranslateInView(Vector3(0.0f, 0.0f, dist));
			break;
		}

		
	}	

	//Description of control scheme:
	//'w','a','s','d' move forward, right, back, left respectively
	//'q', toggle between camera modes
	void InputController::onKeyDown(WPARAM wParam)
	{
		if(wParam == 0x51) // 'q' key
		{
			if(this->cameraMode == CAMERA_MODE_FPS_CAMERA)
			{
				this->cameraMode = CAMERA_MODE_STATIC_CAMERA;
				ShowCursor(TRUE);
			}
			else if(this->cameraMode == CAMERA_MODE_STATIC_CAMERA)
			{
				this->cameraMode = CAMERA_MODE_FPS_CAMERA;
				SetCursorPos(
					this->windowRect.left + (int)(0.5f * (float)(this->windowWidth)),
					this->windowRect.top + (int)(0.5f * (float)(this->windowHeight)));

#ifdef _DEBUG
				ShowCursor(TRUE);
#else
				ShowCursor(FALSE);
#endif
			}
		}
		else if(wParam == 0x57) // 'w' key
		{
			this->motionState = MOTION_STATE_FORWARD;
		}
		else if(wParam == 0x41) // 'a' key
		{
			this->motionState = MOTION_STATE_LEFT;
		}
		else if(wParam == 0x53) // 's' key
		{
			this->motionState = MOTION_STATE_BACK;
		}
		else if(wParam == 0x44)  // 'd' key
		{
			this->motionState = MOTION_STATE_RIGHT;
		}
	}

	void InputController::onKeyUp(WPARAM wParam)
	{
		if(wParam == 0x57 || wParam == 0x41 || wParam == 0x53 || wParam == 0x44)
		{
			this->motionState = MOTION_STATE_STATIONARY;
		}
	}
}

