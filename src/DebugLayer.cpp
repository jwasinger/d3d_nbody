#include "DebugLayer.h"

namespace NBody
{
	DebugLayer::DebugLayer()
	{
		this->axesVBuffer = nullptr;
		this->console = nullptr;
		this->renderer = nullptr;
		this->timer = nullptr;
	}

	DebugLayer::~DebugLayer()
	{
		delete this->console;
		this->axesVBuffer->Release();
	}

	bool DebugLayer::Init(
		DebugOptions &options, 
		Renderer *renderer, 
		InputController *const inputController, 
		Timer *const timer,
		const RECT& windowRect)
	{
		if(!inputController || !timer || !renderer || !inputController)
		{
			OutputDebugString("\nInvalid parameters passed to DebugLayer::Init()\n");
			return false;
		}

		this->windowRect = windowRect;
		this->debugOptions = options;
		this->renderer = renderer;
		this->inputController = inputController;
		this->timer = timer;

		//init 3D orientation axes ----------------------------------------------------------------------------
		D3D11_BUFFER_DESC axesVBufferDesc;
		ZeroMemory(&axesVBufferDesc, sizeof(D3D11_BUFFER_DESC));
		axesVBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		axesVBufferDesc.ByteWidth = sizeof(XMFLOAT4) * 6;
		axesVBufferDesc.CPUAccessFlags = 0;
		axesVBufferDesc.MiscFlags = 0;
		axesVBufferDesc.StructureByteStride = 0;
		axesVBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		XMFLOAT4 verts[6] = 
		{
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
			XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		};

		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = &verts;
		if(FAILED(this->renderer->GetDevice()->CreateBuffer(&axesVBufferDesc, &subresourceData, &this->axesVBuffer)))
		{
			OutputDebugString("\nDebugLayer::Init(): create axesVBuffer failed\n");
			return false;
		}

		/*this->console = new Console();
		if(!this->console->Init(this->renderer, 
								Vector2(0.0f,0.0f), 
								Vector2(0.0f, 0.0f), 
								windowRect.right - windowRect.left, 
								windowRect.bottom - windowRect.top))
		{
			return false;
		}*/

		return true;
	}

	

	bool testFileMap(void)
	{	
		return false;
	}

	/*void updatePipe(void)
	{
		if(!clientConnected)
			return;

		char readBuff[256];
		DWORD bytesRead;

		if(!ReadFile(hConsolePipe, readBuff, sizeof(char) * 256, &bytesRead, NULL))
		{
			if(GetLastError() == ERROR_IO_PENDING)
				return;
			else
			{
				OutputDebugString("\nUnknown pipe error\n");
				return;
			}
		}
	}*/

	void DebugLayer::Render(void)
	{
		//Matrix tmp = this->projMat;
		//this->projMat = Matrix::CreateOrthographic(1.0f, 1.0f, 0.0f, 1.0f);
		/*this->renderer->PushMatrix(TRANSFORM_PROJECTION, Matrix::CreateOrthographicOffCenter(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f));*/
		
		//create a projection so that the origin is at the top left of the physical screen
		//and the screen bounds are [0.0f, 1.0f]
		//Matrix proj = Matrix::CreateTranslation(-0.5f, 0.5f, 0.0f) * Matrix::CreateScale(2.0f, 2.0f, 1.0f); //* Matrix::CreateReflection(Plane(0.0f, 0.0f, 1.0f, 1.0f));
		//Matrix proj2 = Matrix::CreateOrthographicOffCenter(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		//proj.m[2][2] = 1.0f;

		Matrix proj = Matrix(Vector4( 2.0f,  0.0f,  0.0f,  0.0f), 
							 Vector4( 0.0f, -2.0f,  0.0f,  0.0f), 
							 Vector4( 0.0f,  0.0f,  1.0f,  0.0f), 
							 Vector4(-1.0f,  1.0f,  0.0f,  1.0f));

		this->renderer->PushMatrix(TRANSFORM_PROJECTION, proj);
		this->renderer->BeginText();

		if(this->debugOptions.RenderFPS)
		{
			std::wstring FPSTxt = L"frame rate: ";
			FPSTxt = FPSTxt.append( std::to_wstring(timer->GetFramerate()) );

			this->renderer->RenderText(FPSTxt, 0.0f, 0.0f);
		}
		if(this->debugOptions.RenderRawMouseInput && this->inputController->GetCameraMode() == CAMERA_MODE_FPS_CAMERA)
		{
			std::wstring mouseText = L"Mouse Delta (";
			mouseText = mouseText.append( std::to_wstring(this->inputController->GetMouseDeltaX() ) );
			mouseText = mouseText.append( L" , " );
			mouseText = mouseText.append( std::to_wstring(this->inputController->GetMouseDeltaY() ) );
			mouseText = mouseText.append( L")" );

			this->renderer->RenderText(mouseText, 0.0f, 0.0625f);
		}
		if(this->debugOptions.RenderMatrices)
		{
			/*RECTF viewScreenRect;
			viewScreenRect.top = 0.0f;
			viewScreenRect.bottom = 
			this->debug_drawMatrix(this->Camera.GetView(), )*/
		}
		

		this->renderer->EndText();

		/*this->projMat = tmp;
		this->SetTransform(TRANSFORM_PROJECTION, this->projMat);*/

		//this->console->Render();

		this->renderer->PopMatrix(TRANSFORM_PROJECTION);

		if(this->debugOptions.RenderAxes)
		{
			//axes should be transformed so that they appear in the upper lefthand corner of the screen
			//axes should take up one quarter of the screen when the orientation of the view is such that it is aligned
			//with the world x,y,z axes
			
			Matrix transform = Matrix::CreateScale(0.25f, 0.25f, 1.0f) * this->renderer->GetInvTransform(TRANSFORM_TYPE::TRANSFORM_VIEW);
			this->renderer->PushMatrix(TRANSFORM_TYPE::TRANSFORM_WORLD, transform);

			this->renderer
		}
	}
}