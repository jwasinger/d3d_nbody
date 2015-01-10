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

		Vector3 mouseDelta(-this->mouseDeltaX, this->mouseDeltaY, 0.0f);
		Matrix view = this->renderer->GetCamera().GetView();
		
		this->renderer->GetCamera().RotateAxisAngle(view.Right(), mouseDamping*this->mouseDeltaY);
		this->renderer->GetCamera().RotateAxisAngle(view.Up(), mouseDamping*this->mouseDeltaX);
	}

	Vector3 InputController::__transform_local(Vector3 &v)
	{
		Matrix inv_view = this->renderer->GetInvTransform(TRANSFORM_VIEW);
		return Vector3::TransformNormal(v, inv_view);
	}

	void InputController::Update(double ms)
	{
		/*Matrix translation = Matrix::Identity();
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
		}*/

		float dist = (ms / 1000.0)*this->cameraMoveSpeed;
		Matrix cur_transform = this->renderer->GetTransform(TRANSFORM_VIEW);
		Matrix new_transform = Matrix::Identity();
		Matrix inv_view = this->renderer->GetInvTransform(TRANSFORM_VIEW);
		Matrix rot_x = Matrix::CreateRotationX(0.1f);
		Vector3 axis;

		switch (this->motionState)
		{
		case MOTION_STATE_FORWARD:
			new_transform = inv_view * Matrix::CreateTranslation(0.0f, 0.0f, dist);
			cur_transform *= new_transform;
			this->renderer->SetTransform(TRANSFORM_VIEW, 
					cur_transform);
			break;
		case MOTION_STATE_BACK:
			new_transform = inv_view * Matrix::CreateTranslation(0.0f, 0.0f, -dist);
			cur_transform *= new_transform;
			this->renderer->SetTransform(TRANSFORM_VIEW,
				cur_transform);
			break;
		case MOTION_STATE_ROT_LEFT:
			axis = __transform_local(Vector3(0, 0, 1));
			new_transform = cur_transform * Matrix::CreateFromAxisAngle(axis, 0.1f);
			this->renderer->SetTransform(TRANSFORM_VIEW,
				new_transform);
			break;
		case MOTION_STATE_ROT_RIGHT:
			axis = __transform_local(Vector3(0, 0, 1));
			new_transform = cur_transform * Matrix::CreateFromAxisAngle(axis, -0.1f);
			this->renderer->SetTransform(TRANSFORM_VIEW,
				new_transform);
			break;
		case MOTION_STATE_ROT_DOWN:
			axis = __transform_local(Vector3(1, 0, 0));
			new_transform = cur_transform * Matrix::CreateFromAxisAngle(axis, 0.1f);
			this->renderer->SetTransform(TRANSFORM_VIEW,
				new_transform);
			break;
		case MOTION_STATE_ROT_UP:
			axis = __transform_local(Vector3(1, 0, 0));
			new_transform = cur_transform * Matrix::CreateFromAxisAngle(axis, -0.1f);
			this->renderer->SetTransform(TRANSFORM_VIEW,
				new_transform);
			break;
		}
	}	

	//Description of control scheme:
	//'w','a','s','d' move forward, right, back, left respectively
	//'q', toggle between camera modes
	/*void InputController::onKeyDown(WPARAM wParam)
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
	}*/

	void InputController::onKeyDown(WPARAM wParam)
	{
		switch (wParam)
		{
		case VK_UP:
			this->motionState = MOTION_STATE_FORWARD;
			break;
		case VK_DOWN:
			this->motionState = MOTION_STATE_BACK;
			break;
		case 0x57: // 'w' key
			this->motionState = MOTION_STATE_ROT_DOWN;
			break;
		case 0x41: // 'a' key
			this->motionState = MOTION_STATE_ROT_LEFT;
			break;
		case 0x53:
			this->motionState = MOTION_STATE_ROT_UP;
			break;
		case 0x44: // 'd' key
			this->motionState = MOTION_STATE_ROT_RIGHT;
			break;
		case 0x51:
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
		if(wParam == 0x57 || wParam == 0x41 || wParam == 0x53 || wParam == 0x44 || wParam == VK_UP || wParam == VK_DOWN)
		{
			this->motionState = MOTION_STATE_STATIONARY;
		}
	}
}

