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

		Vector3 mouseDelta(-this->mouseDeltaX, this->mouseDeltaY, 0.0f);
		Matrix view = this->renderer->GetCamera().GetView();
		
		this->renderer->GetCamera().Yaw(-mouseDamping*this->mouseDeltaY);
		this->renderer->GetCamera().Pitch(-mouseDamping*this->mouseDeltaX);
	}

	void InputController::Update(double ms)
	{
		float dist = (ms / 1000.0)*this->cameraMoveSpeed;
		Matrix view = this->renderer->GetCamera().GetView();
		Vector3 axis;

		switch (this->motionState)
		{
		case MOTION_STATE_FORWARD:
			this->renderer->GetCamera().Walk(dist);
			break;
		case MOTION_STATE_BACK:
			this->renderer->GetCamera().Walk(-dist);
			break;
		case MOTION_STATE_LEFT:
			this->renderer->GetCamera().Strafe(-dist);
			break;
		case MOTION_STATE_RIGHT:
			this->renderer->GetCamera().Strafe(dist);
			break;
		case MOTION_STATE_ROT_RIGHT:
			this->renderer->GetCamera().Roll(0.01f);
			break;
		case MOTION_STATE_ROT_LEFT:
			this->renderer->GetCamera().Roll(-0.01f);
			break;
		}
	}	

	/*control scheme description:
	* w- move forward
	* a- move left
	* s - move backwards
	* d - move right
	* q - rotate left over camera's z-axis
	* e - rotate right over camera's z-axis
	* , - enable/disable FPS mode
	*/
	void InputController::onKeyDown(WPARAM wParam)
	{
		switch (wParam)
		{
		case 0x57: // 'w' key
			this->motionState = MOTION_STATE_FORWARD;
			break;
		case 0x41: // 'a' key
			this->motionState = MOTION_STATE_LEFT;
			break;
		case 0x53: // 's' key
			this->motionState = MOTION_STATE_BACK;
			break;
		case 0x44: // 'd' key
			this->motionState = MOTION_STATE_RIGHT;
			break;
		case 0x51: // 'q' key
			this->motionState = MOTION_STATE_ROT_LEFT;
			break;
		case 0x45: // 'e' key
			this->motionState = MOTION_STATE_ROT_RIGHT;
			break;
		case VK_OEM_COMMA:
			if (this->cameraMode == CAMERA_MODE_FPS_CAMERA)
			{
				this->cameraMode = CAMERA_MODE_STATIC_CAMERA;
				ShowCursor(TRUE);
			}
			else if (this->cameraMode == CAMERA_MODE_STATIC_CAMERA)
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
			break;
		}
	}

	void InputController::onKeyUp(WPARAM wParam)
	{
		if(wParam == 0x57 || wParam == 0x41 || wParam == 0x53 || wParam == 0x44 || wParam == 0x51 || wParam == 0x45)
		{
			this->motionState = MOTION_STATE_STATIONARY;
		}
	}
}

