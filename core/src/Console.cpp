#include "Console.h"

namespace Core
{

	Console::Console()
	{
		this->consoleSize = Vector2(0.0f);
		this->windowSizeX = 0;
		this->windowSizeY = 0;
		this->backgroundColor = Color((DirectX::XMVECTOR)Colors::White);
		this->fontSize = 0;
		this->quadVBuff = nullptr;
		this->renderer = nullptr;
		this->colorCBuff = nullptr;
		ZeroMemory(&this->screenRect, sizeof(RECT));
		this->transparentBS = nullptr;
		ZeroMemory(&this->verts, sizeof(Vector4) * 4);
	}

	Console::~Console(void)
	{
		SafeRelease<ID3D11BlendState>(&this->transparentBS);
		SafeRelease<ID3D11Buffer>(&this->quadVBuff);
		SafeRelease<ID3D11Buffer>(&this->colorCBuff);
	}

	bool Console::Init(
		NBody::Renderer *renderer,
		const Vector2 & pos, 
		const Vector2 & consoleSize, 
		int windowSizeX,
		int windowSizeY
		)
	{
		

		this->consoleSize = consoleSize;
		this->windowSizeX = windowSizeX;
		this->windowSizeY = windowSizeY;
		this->position = Vector2(pos);
		
		if(renderer == nullptr)
		{
			OutputDebugString("\n Invalid parameter 'renderer' to Console::Init.  renderer cannot be null\n");
			return false;
		}

		this->renderer = renderer;

		if(windowSizeX <= 0 || windowSizeY <= 0)
		{
			OutputDebugString("\nInvalid parameter 'size' to Console::Init. windowSizeX and windowSizeY should both be greater than 0\n");
			return false;
		}

		if(consoleSize.x <= 0 || consoleSize.y <= 0)
		{
			OutputDebugString("\nInvalid parameter 'size' to Console::Init. size.x and size.y should both be greater than 0\n");
			return false;
		}

		D3D11_BLEND_DESC transparentBSDesc;
		ZeroMemory(&transparentBSDesc, sizeof(D3D11_BLEND_DESC));
		transparentBSDesc.AlphaToCoverageEnable = TRUE;
		transparentBSDesc.IndependentBlendEnable = TRUE;

		transparentBSDesc.RenderTarget[0].BlendEnable = TRUE;
		transparentBSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentBSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentBSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentBSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentBSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		transparentBSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentBSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		if(FAILED(this->renderer->GetDevice()->CreateBlendState(&transparentBSDesc, &this->transparentBS)))
		{
			OutputDebugString("\nFailed to create 'transparentBS' \n");
			return false;
		}
		
		D3D11_BUFFER_DESC vBufferDesc;
		ZeroMemory(&vBufferDesc, sizeof(D3D11_BUFFER_DESC));
		vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vBufferDesc.ByteWidth = sizeof(Vector4) * 6;
		vBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vBufferDesc.MiscFlags = 0;
		vBufferDesc.StructureByteStride = sizeof(Vector4);
		vBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		
		if(FAILED(this->renderer->GetDevice()->CreateBuffer(&vBufferDesc, NULL, &this->quadVBuff)))
		{
			OutputDebugString("\n Device->CreateBuffer() failed on Console::quadBuff\n");	
			return false;
		}
	    
		this->SetPos(this->position);

		return true;
	}

	void Console::SetPos(const Vector2 &pos)
	{
		if(pos.x < 0.0f || pos.x > 1.0f || pos.y > 1.0f || pos.y < 0.0f)
		{
			OutputDebugString("\nInvalid parameter 'pos'\n");
			return;
		}

		this->position = pos;
		this->calcScreenRect();
		//this->screenRect.top = (int)clampToSize(this->position.x, 0, this->windowSizeY);
		//this->screenRect.left = (int)clampToSize(this->position.y, 0, this->windowSizeX);
		//this->screenRect.bottom = this->screenRect.top + consoleSize.y;//(int)clampToSize(consoleSize.y, 0, this->windowSizeY);
		//this->screenRect.right = this->screenRect.left + consoleSize.x;//(int)clampToSize(consoleSize.x, 0, this->windowSizeX);

		float right = this->position.x + this->consoleSize.x;
		float bottom = this->position.y + this->consoleSize.y;

		this->verts[0] = Vector4(this->position.x, this->position.y, 0.5f, 1.0f);
		this->verts[1] = Vector4(right, bottom, 0.5f, 1.0f);
		this->verts[2] = Vector4(this->position.x, bottom, 0.5f, 1.0f);

		this->verts[3] = Vector4(right, this->position.y, 0.5f, 1.0f);
		this->verts[4] = Vector4(right, bottom, 0.5f, 1.0f);
		this->verts[5] = Vector4(this->position.x, this->position.y, 0.5f, 1.0f);
		
		
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->renderer->GetDeviceContext()->Map(this->quadVBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
		{
			OutputDebugString("\nContext->Map() on mappedSubresource failed\n");
			return;
		}

		memcpy(mappedSubresource.pData, this->verts,sizeof(Vector4) * 6);
		
		this->renderer->GetDeviceContext()->Unmap(this->quadVBuff, 0);
	}

	void Console::SetSize(const Vector2 &size)
	{
		if(size.x <= 0 || size.y <= 0)
		{
			OutputDebugString("\nInvalid parameter 'size' to Console::Init. size.x and size.y should both be greater than 0\n");
			return;
		}

		if(size == this->consoleSize)
			return;

		this->consoleSize = size;
		this->calcScreenRect();
	}

	//expand number in range [0.0, 1.0] to [min, max]
	double clampToSize(double val, double min, double max)
	{
		if(val < 0.0f || val > 1.0f)
		{
			return -1;
		}

		return val * (max-min) + min;
	}

	void Console::calcScreenRect(void)
	{
		this->screenRect.left = clampToSize(this->position.x, 0, windowSizeX);
		this->screenRect.top = clampToSize(this->position.y, 0, this->windowSizeY);
		this->screenRect.right = clampToSize(this->position.x + this->consoleSize.x, 0, this->windowSizeX);
		this->screenRect.bottom = clampToSize(this->position.y + this->consoleSize.y, 0, this->windowSizeY);
	}

	void Console::Render(void)
	{
		this->renderer->SetCullMode(D3D11_CULL_BACK);

		this->SetPos(this->position);
		//draw quad
		//reset blend state
		//this->renderer->Context->RSSetState(

		//set color shader
		this->renderer->BindShader(SHADER_TYPE_COLOR);

		//pass the color cbuffer to the shader
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->renderer->GetDeviceContext()->Map(this->colorCBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
		{
			OutputDebugString("\ncontext->Map() failed on colorCBuff\n");
		}

		memcpy(mappedSubresource.pData, this->color, sizeof(float) * 4);
		
		this->renderer->GetDeviceContext()->Unmap(this->colorCBuff, 0);

		//set scissor to the rectangle containing the Console
		UINT numRects = 1;
		this->renderer->GetDeviceContext()->RSGetScissorRects(&numRects, nullptr);
		
		RECT *rects = (RECT *)malloc(sizeof(RECT) * numRects);
		this->renderer->GetDeviceContext()->RSGetScissorRects(&numRects, rects);
		
		RECT testRect;
		testRect.left = 0;
		testRect.right = 1;
		testRect.bottom = 1;
		testRect.top = 0;

		//RECT newSR[] = {this->screenRect};
		RECT newSR[] = {testRect};

		//this->renderer->GetDeviceContext()->RSSetScissorRects(1, newSR);

		//set blend state with transparency
		ID3D11BlendState *prevBS;
		FLOAT prevBlendFactor[4];
		UINT prevSampleMask;

		this->renderer->GetDeviceContext()->OMGetBlendState(&prevBS,prevBlendFactor, &prevSampleMask);

		this->renderer->GetDeviceContext()->OMSetBlendState(this->transparentBS, NULL, 0xffffffff);

		ID3D11Buffer *vBuff[] = {this->quadVBuff};
		ID3D11Buffer *nullVBuff[] = {nullptr};
		UINT stride[] = {sizeof(Vector4)};
		UINT offset[] = {0};

		this->renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, vBuff, stride, offset);
		this->renderer->SetColor(this->textColor.ToVector4());
		this->renderer->GetDeviceContext()->Draw(6, 0);

		//this->renderer->Context->IASetVertexBuffers(0, 1, nullVBuff, nullptr, nullptr);
		//this->renderer->Context->RSSetScissorRects(numRects, rects);

		this->renderer->GetDeviceContext()->OMSetBlendState(prevBS, prevBlendFactor, prevSampleMask);

		//reset transform
		/*this->renderer->SetTransform(TRANSFORM_PROJECTION, oldProj);
		this->renderer->SetTransform(TRANSFORM_WORLD, oldWorld);
		this->renderer->SetTransform(TRANSFORM_VIEW, oldView);*/

		/*this->renderer->PopMatrix(TRANSFORM_PROJECTION);
		this->renderer->PopMatrix(TRANSFORM_WORLD);
		this->renderer->PopMatrix(TRANSFORM_VIEW);*/

		//reset scissor rect
		free(rects);

		this->renderer->UnbindShader();
	}
}