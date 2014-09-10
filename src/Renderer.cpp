#include "Renderer.h"

using namespace DirectX;

namespace NBody
{
	Renderer::Renderer()
	{  
		this->factory = nullptr;
		this->adapter = nullptr;
		this->swapChain = nullptr;
		this->device = nullptr;
		this->context = nullptr;
		this->RTView = nullptr;
		this->SBatch = nullptr;
		this->SFont = nullptr;
		this->particleTexture = nullptr;
		this->particleTextureSRV = nullptr;
		this->PSInputLayout = nullptr;
		this->textureInputLayout = nullptr;
		this->quadVBuffer = nullptr;
		this->particleVBuffer = nullptr;
		this->particleBuffer0 = nullptr;
		this->particleBuffer1 = nullptr;
		this->particleSRV0 = nullptr;
		this->particleSRV1 = nullptr;
		this->pDrawParamsBuffer = nullptr;
		this->textureVShader = nullptr;
		this->texturePShader = nullptr;
		this->boundShader = SHADER_TYPE_NONE;
		this->PSVShader = nullptr;
		this->PSPShader = nullptr;
		this->PSGShader = nullptr;
		this->colorVShader = nullptr;
		this->colorPShader = nullptr;
		this->colorInputLayout = nullptr;
		this->colorCBuffer = nullptr;
		this->worldCBuffer = nullptr;
		this->viewCBuffer = nullptr;
		this->projectionCBuffer = nullptr;
		this->rsDisableCull = nullptr;
		this->rsCullFront = nullptr;
		this->rsDefault = nullptr;
		this->defaultSampler = nullptr;
	}

	Renderer::~Renderer()
	{
		factory->Release();
		adapter->Release();
		swapChain->Release();
		device->Release();
		context->Release();
		RTView->Release();
		delete SBatch;
		delete SFont;
		particleTexture->Release();
		particleTextureSRV->Release();
		PSInputLayout->Release();
		textureInputLayout->Release();
		quadVBuffer->Release();
		particleVBuffer->Release();
		pDrawParamsBuffer->Release();

		textureVShader->Release();
		texturePShader->Release();
		
		PSVShader->Release();
		PSPShader->Release();
		PSGShader->Release();

		colorVShader->Release();
		colorPShader->Release();
		colorInputLayout->Release();
		colorCBuffer->Release();

		worldCBuffer->Release();
		viewCBuffer->Release();
		projectionCBuffer->Release();
		
		rsDisableCull->Release();
		rsCullFront->Release();
		rsDefault->Release();

		defaultSampler->Release();
	}

	bool Renderer::Init(const RECT &windowRect, HWND hWnd)
	{
		if(!hWnd)
			return false;
		
		this->hWnd = hWnd;
		this->bbWidth = windowRect.right - windowRect.left;
		this->bbHeight = windowRect.bottom - windowRect.top;

		HRESULT result;

		result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&this->factory);
		if(FAILED(result))
		{
			OutputDebugString("\nCreateDXGIFactory() failed\n");
			return false;
		}

		result = this->factory->EnumAdapters(0, (IDXGIAdapter**)&this->adapter);
		if(FAILED(result))
		{
			OutputDebugString("\nfactory->EnumAdapters() failed\n");
			return false;
		}

		D3D_FEATURE_LEVEL featureLvl11 = D3D_FEATURE_LEVEL_11_0;

		DXGI_SWAP_CHAIN_DESC swapChainDescription;
		ZeroMemory(&swapChainDescription, sizeof(swapChainDescription));
	
		swapChainDescription.BufferCount = 1;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		//swapChainDescription.Flags = 0;

		swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDescription.BufferDesc.Width = this->bbWidth;
		swapChainDescription.BufferDesc.Height = this->bbHeight;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
		swapChainDescription.Flags = 0;
		swapChainDescription.OutputWindow = this->hWnd;
	
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;

		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDescription.Windowed = TRUE;
		
	#ifdef _DEBUG
	
		result = D3D11CreateDeviceAndSwapChain(
					nullptr, 
					D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, 
					nullptr, 
					D3D11_CREATE_DEVICE_DEBUG,
					&featureLvl11, 
					1, 
					D3D11_SDK_VERSION,
					&swapChainDescription,
					(IDXGISwapChain**)&this->swapChain,
					(ID3D11Device**)&this->device,
					nullptr,
					(ID3D11DeviceContext**)&this->context);

		if(FAILED(result))
		{
			OutputDebugString("\nD3D11CreateDeviceAndSwapChain() failed\n");
			return false;
		}

	#else

		result = D3D11CreateDeviceAndSwapChain(
					nullptr, 
					D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, 
					nullptr, 
					nullptr, 
					&featureLvl11, 
					1, 
					D3D11_SDK_VERSION,
					&swapChainDescription,
					(IDXGISwapChain**)&this->SwapChain,
					(ID3D11Device**)&this->Device,
					nullptr,
					(ID3D11DeviceContext**)&this->Context);

		if(FAILED(result))
		{
			return false;
		}

	#endif

		ID3D11Texture2D *bbSurfacePtr = nullptr;
		result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bbSurfacePtr);
		if(FAILED(result))
		{
			OutputDebugString("\nswapChain->GetBuffer() failed\n");
			return false;
		}
	
		result = this->device->CreateRenderTargetView(bbSurfacePtr, nullptr, (ID3D11RenderTargetView**)&this->RTView);
		if(FAILED(result))
		{
			OutputDebugString("\ndevice->createRenderTargetView failed\n");
			return false;
		}

		if(!this->createShadersAndInputLayouts())
			return false;

		this->context->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&this->RTView, nullptr);
	
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.Height = this->bbHeight;
		viewport.Width = this->bbWidth;
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		this->context->RSSetViewports(1, &viewport);

		//load the texture for the particle point sprite
		if(FAILED(DirectX::CreateWICTextureFromFile(this->device, this->context, GetFilePath(L"particle.bmp").data(), &this->particleTexture, &this->particleTextureSRV)))
		{
			OutputDebugString("\nCreateWICTextureFromFile() failed\n");
			return false;
		}

		//create vBuffer to hold quad verts
		D3D11_BUFFER_DESC quadBufferDesc;
		ZeroMemory(&quadBufferDesc, sizeof(D3D11_BUFFER_DESC));
		quadBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		quadBufferDesc.ByteWidth = sizeof(NBody::TexturedVertex) * 6;
		quadBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		quadBufferDesc.MiscFlags = 0;
		quadBufferDesc.StructureByteStride = sizeof(NBody::TexturedVertex);
		quadBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		if(FAILED(this->device->CreateBuffer(&quadBufferDesc, nullptr, &this->quadVBuffer)))
			return false;

		//create a vertex buffer for the particle point sprites
		D3D11_BUFFER_DESC particleVBufferDesc;
		ZeroMemory(&particleVBufferDesc, sizeof(D3D11_BUFFER_DESC));
		particleVBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		particleVBufferDesc.ByteWidth = sizeof(unsigned int) * numParticles;
		particleVBufferDesc.CPUAccessFlags = 0;
		particleVBufferDesc.MiscFlags = 0;
		particleVBufferDesc.StructureByteStride = sizeof(unsigned int);
		particleVBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if(FAILED(this->device->CreateBuffer(&particleVBufferDesc, NULL, &this->particleVBuffer)))
		{
			return false;
		}

		if(!this->createRasterizerStates())
			return false;

		this->viewMat = Matrix::Identity();

		if(!this->createCBuffers())
			return false;

		if(!this->createSamplers())
			return false;

		try
		{
			this->SBatch = new SpriteBatch(this->context);
			this->SFont = new SpriteFont(this->device, GetFilePath(L"arial.spritefont").data());
		}
		catch(std::exception e)
		{
			OutputDebugString("\n");
			OutputDebugString(e.what());
			OutputDebugString("\n");
			return false;
		}

		return true;
	}

	bool Renderer::createCBuffers(void)
	{
		//----------------------- set default constant buffers on the cpu --------------------------------------------------------------

		this->worldMat = Matrix::Identity();
		this->projMat = Matrix::Identity();
		//XMStoreFloat4x4(&this->viewMat, mat);

		//------------------------- pass the constant buffers to the GPU --------------------------------------------------------------

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		XMFLOAT4X4 defaultTransform;
		XMStoreFloat4x4(&defaultTransform, XMMatrixIdentity());

		D3D11_SUBRESOURCE_DATA subresourceData;
		ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		subresourceData.pSysMem = &defaultTransform;

		if(FAILED(this->device->CreateBuffer(&bufferDesc, &subresourceData, &this->worldCBuffer)))
			return false;

		if(FAILED(this->device->CreateBuffer(&bufferDesc, &subresourceData, &this->projectionCBuffer)))
			return false;

		subresourceData.pSysMem = &this->viewMat;
		if(FAILED(this->device->CreateBuffer(&bufferDesc, &subresourceData, &this->viewCBuffer)))
			return false;

		ID3D11Buffer *cBuffers[3] = { this->worldCBuffer, this->projectionCBuffer, this->viewCBuffer };
		this->context->VSSetConstantBuffers(0, 3, cBuffers);
		
		bufferDesc.ByteWidth = sizeof(ParticleDrawParams);
		bufferDesc.StructureByteStride = 0;

		if(FAILED(this->device->CreateBuffer(&bufferDesc, NULL, &this->pDrawParamsBuffer)))
			return false;
		
		bufferDesc.ByteWidth = sizeof(XMFLOAT4);
		bufferDesc.StructureByteStride = 0;

		if(FAILED(this->device->CreateBuffer(&bufferDesc, NULL, &this->colorCBuffer)))
			return false;

		return true;
	}

	void Renderer::SetCullMode(D3D11_CULL_MODE mode)
	{
		switch(mode)
		{
		case D3D11_CULL_NONE:
			this->context->RSSetState(this->rsDisableCull);
			return;
		case D3D11_CULL_BACK:
			this->context->RSSetState(this->rsDefault);
			return;
		case D3D11_CULL_FRONT:
			this->context->RSSetState(this->rsCullFront);
			return;
		}

		OutputDebugString("\n invalid parameters to Renderer::SetCullMode()");
	}

	bool Renderer::createRasterizerStates(void)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		
		if(FAILED(this->device->CreateRasterizerState(&rasterizerDesc, &this->rsDefault)))
		{
			OutputDebugString("\ncreate rsDefault failed\n");
			return false;
		}

		rasterizerDesc.CullMode = D3D11_CULL_NONE;

		if(FAILED(this->device->CreateRasterizerState(&rasterizerDesc, &this->rsDisableCull)))
		{
			OutputDebugString("\ncreate rsDisableCull failed\n");
			return false;
		}

		rasterizerDesc.CullMode = D3D11_CULL_FRONT;

		if(FAILED(this->device->CreateRasterizerState(&rasterizerDesc, &this->rsCullFront)))
		{
			OutputDebugString("\ncreate rsCullFront failed\n");
		}

		return true;
	}

	bool Renderer::createSamplers(void)
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc,sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if(FAILED(this->device->CreateSamplerState(&samplerDesc, &this->defaultSampler)))
		{
			OutputDebugString("\nCreate defaultSampler failed\n");
			return false;
		}

		return true;
	}

	Matrix Renderer::GetInvTransform(TRANSFORM_TYPE type) const
	{
		switch(type)
		{
		case TRANSFORM_WORLD:
			return this->invWorldMat;
			break;
		case TRANSFORM_VIEW:
			return this->invViewMat;
			break;
		case TRANSFORM_PROJECTION:
			return this->invProjMat;
			break;
		}

		OutputDebugString("\nInvalid parameters to Renderer::GetInvTransform()\n");
	}

	//test code to compile Test.hlsl
	bool Renderer::createShadersAndInputLayouts(void)
	{
		UINT flags1 = 0;

#if _DEBUG
		
		flags1 |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG; 

#endif
		ID3DBlob *vShaderCode = nullptr;
		ID3DBlob *pShaderCode = nullptr;
		ID3DBlob *gShaderCode = nullptr;
		ID3DBlob *errorCode = nullptr;

		//create Point Sprite shaders ----------------------------------------------------------------------------------------------------
		if(FAILED(D3DCompileFromFile(GetFilePath(L"PointSprite.hlsl").data(), nullptr, nullptr, "VShader", "vs_5_0", flags1, 0, &vShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&vShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}
		
		if(FAILED(D3DCompileFromFile(GetFilePath(L"PointSprite.hlsl").data(), nullptr, nullptr, "PShader", "ps_5_0", flags1, 0, &pShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&pShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}

		if(FAILED(D3DCompileFromFile(GetFilePath(L"PointSprite.hlsl").data(), nullptr, nullptr, "GShader", "gs_5_0", flags1, 0, &gShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&pShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}

		if(FAILED(this->device->CreateGeometryShader(gShaderCode->GetBufferPointer(), gShaderCode->GetBufferSize(), nullptr, &this->PSGShader)))
		{
			OutputDebugString("\ncreate point sprite geometry shader failed\n");
			return false;	
		}

		if(FAILED(this->device->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), nullptr, &this->PSPShader)))
		{
			OutputDebugString("\ncreate point sprite pixel shader failed\n");
			return false;
		}

		if(FAILED(this->device->CreateVertexShader(vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), nullptr, &this->PSVShader)))
		{
			OutputDebugString("\n create point sprite vertex shader failed\n");
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC positionInputElement;
		positionInputElement.AlignedByteOffset = 0;
		positionInputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		positionInputElement.InputSlot = 0;
		positionInputElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		positionInputElement.InstanceDataStepRate = 0;
		positionInputElement.SemanticName = "POSITION";
		positionInputElement.SemanticIndex = 0;

		if(FAILED(this->device->CreateInputLayout(&positionInputElement, 1, vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), &this->PSInputLayout)))
		{
			OutputDebugString("\nCreate positionInputElement failed\n");
			return false;
		}

		SafeRelease<ID3DBlob>(&vShaderCode);
		SafeRelease<ID3DBlob>(&pShaderCode);

		//create Texture shaders------------------------------------------------------------------------------------------------------------
		
		if(FAILED(D3DCompileFromFile(GetFilePath(L"Texture.hlsl").data(), nullptr, nullptr, "VShader", "vs_5_0", flags1, 0, &vShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&vShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}
		
		if(FAILED(D3DCompileFromFile(GetFilePath(L"Texture.hlsl").data(), nullptr, nullptr, "PShader", "ps_5_0", flags1, 0, &pShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&pShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}

		if(FAILED(this->device->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), nullptr, &this->texturePShader)))
		{
			OutputDebugString("\n create texture pixel shader failed \n");
			return false;
		}

		if(FAILED(this->device->CreateVertexShader(vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), nullptr, &this->textureVShader)))
		{
			OutputDebugString("\n create texture vertex shader failed \n");
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC textureInputElement;
		textureInputElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		textureInputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
		textureInputElement.InputSlot = 0;
		textureInputElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		textureInputElement.InstanceDataStepRate = 0;
		textureInputElement.SemanticIndex = 0;
		textureInputElement.SemanticName = "TEXCOORD";

		D3D11_INPUT_ELEMENT_DESC textureShaderInput[] = {positionInputElement, textureInputElement};

		if(FAILED(this->device->CreateInputLayout(textureShaderInput, 2, vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), &this->textureInputLayout)))
		{
			OutputDebugString("\n Create textureShaderInput failed \n");
			return false;
		}

		SafeRelease<ID3DBlob>(&vShaderCode);
		SafeRelease<ID3DBlob>(&pShaderCode);

		//create color shaders and input layout ---------------------------------------------------------------------------------------------

		if(FAILED(D3DCompileFromFile(GetFilePath(L"Colored.hlsl").data(), nullptr, nullptr, "VShader", "vs_5_0", flags1, 0, &vShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&vShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}
		
		if(FAILED(D3DCompileFromFile(GetFilePath(L"Colored.hlsl").data(), nullptr, nullptr, "PShader", "ps_5_0", flags1, 0, &pShaderCode, &errorCode)))
		{
			SafeRelease<ID3DBlob>(&pShaderCode);	
			if(errorCode)
			{
				OutputDebugStringA((LPCSTR)errorCode->GetBufferPointer());
				SafeRelease<ID3DBlob>(&errorCode);
			}

			return false;
		}

		if(FAILED(this->device->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), nullptr, &this->colorPShader)))
		{
			OutputDebugString("\n create colored pixel shader failed \n");
			return false;
		}

		if(FAILED(this->device->CreateVertexShader(vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), nullptr, &this->colorVShader)))
		{
			OutputDebugString("\n create colored vertex shader failed \n");
			return false;
		}

		/*D3D11_INPUT_ELEMENT_DESC colorInputElement;
		ZeroMemory(&colorInputElement, sizeof(D3D11_INPUT_ELEMENT_DESC));
		colorInputElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		colorInputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		colorInputElement.InputSlot = 0;
		colorInputElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		colorInputElement.InstanceDataStepRate = 0;
		colorInputElement.SemanticIndex = 0;
		colorInputElement.SemanticName = "COLOR";*/

		D3D11_INPUT_ELEMENT_DESC colorIEArr[1] = {positionInputElement};
		if(FAILED(this->device->CreateInputLayout(colorIEArr, 1, vShaderCode->GetBufferPointer(), vShaderCode->GetBufferSize(), &this->colorInputLayout)))
		{
			OutputDebugString("\n create colorIEArr failed \n");
			return false;
		}

		return true;
	}
	
	
	bool Renderer::isGShaderBound(void)
	{
		ID3D11GeometryShader* gShader = NULL;
		this->context->GSGetShader(&gShader, NULL, NULL);

		if(!gShader)
		{
			return false;
		}

		return true;
	}

	void Renderer::SetTransform(TRANSFORM_TYPE type, const Matrix &val)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		ID3D11Buffer *cBuffer[1];

		//use memcpy to set the data in mappedSubresource?
		switch(type)
		{
		case TRANSFORM_WORLD:

			this->worldMat = val;
			this->invWorldMat = this->worldMat.Invert();

			this->context->Map(this->worldCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
			memcpy(mappedSubresource.pData, &this->worldMat, sizeof(Matrix));

			this->context->Unmap(this->worldCBuffer, 0);

			cBuffer[0] = this->worldCBuffer;
			this->context->VSSetConstantBuffers(0, 1, cBuffer);
			this->context->PSSetConstantBuffers(0, 1, cBuffer);
			this->context->GSSetConstantBuffers(0, 1, cBuffer);

			break;
		case TRANSFORM_VIEW:
			
			this->viewMat = val;
			this->invViewMat = this->viewMat.Invert();

			this->context->Map(this->viewCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
			memcpy(mappedSubresource.pData, &this->viewMat, sizeof(Matrix));
			
			this->context->Unmap(this->viewCBuffer, 0);

			cBuffer[0] = this->viewCBuffer;
			this->context->VSSetConstantBuffers(1, 1, cBuffer);
			this->context->PSSetConstantBuffers(1, 1, cBuffer);
			this->context->GSSetConstantBuffers(1, 1, cBuffer);

			break;
		case TRANSFORM_PROJECTION:
			this->projMat = val;
			this->invProjMat = this->projMat.Invert();

			this->context->Map(this->projectionCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
			memcpy(mappedSubresource.pData, &this->projMat, sizeof(Matrix));
			
			this->context->Unmap(this->projectionCBuffer, 0);

			cBuffer[0] = this->projectionCBuffer;
			this->context->VSSetConstantBuffers(2, 1, cBuffer);
			this->context->PSSetConstantBuffers(2, 1, cBuffer);
			this->context->GSSetConstantBuffers(2, 1, cBuffer);

			break;
		}
	}
	
	Matrix Renderer::GetTransform(TRANSFORM_TYPE type) const
	{
		switch(type)
		{
		case TRANSFORM_WORLD:
			return this->worldMat;
			break;
		case TRANSFORM_PROJECTION:
			return this->projMat;
			break;
		case TRANSFORM_VIEW:
			return this->viewMat;
			break;
		}
		
		OutputDebugString("\nInvalid parameters to Renderer::GetTransform()\n");
	}

	void Renderer::rebindParticleCBs(void)
	{
		ParticleDrawParams params;
		params.radius = 1.0f;
		
		//does & operator on function call produce weird results?
		XMStoreFloat4x4(&params.invView, this->invViewMat);

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		this->context->Map(this->pDrawParamsBuffer, 0, D3D11_MAP_WRITE, 0, &mappedSubresource);
		mappedSubresource.pData = &params;

		ID3D11Buffer *cBufferArr[1] = {this->pDrawParamsBuffer};
		this->context->GSSetConstantBuffers(3, 1, cBufferArr);
	}

	void Renderer::rebindTransformCBs(void)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		ID3D11Buffer *cBuffer[1];

		//bind world-----------------------------------------------------------------------------------------------------------------------------------
		//this->worldStack.push_back(val);
		//this->worldMat = calcStackVal(this->worldStack);

		this->context->Map(this->worldCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, &this->worldMat, sizeof(Matrix));

		this->context->Unmap(this->worldCBuffer, 0);

		cBuffer[0] = this->worldCBuffer;
		this->context->VSSetConstantBuffers(0, 1, cBuffer);
		this->context->PSSetConstantBuffers(0, 1, cBuffer);
		this->context->GSSetConstantBuffers(0, 1, cBuffer);

		//bind view------------------------------------------------------------------------------------------------------------------------------------
		/*this->viewStack.push_back(val);
		this->viewMat = calcStackVal(this->viewStack);
		this->invViewMat = this->viewMat.Invert();*/

		this->context->Map(this->viewCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, &this->viewMat, sizeof(Matrix));
			
		this->context->Unmap(this->viewCBuffer, 0);

		cBuffer[0] = this->viewCBuffer;
		this->context->VSSetConstantBuffers(1, 1, cBuffer);
		this->context->PSSetConstantBuffers(1, 1, cBuffer);
		this->context->GSSetConstantBuffers(1, 1, cBuffer);

		//bind projection------------------------------------------------------------------------------------------------------------------------------
		/*this->projStack.push_back(val);
		this->projMat = calcStackVal(this->viewStack);*/
			
		this->context->Map(this->projectionCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, &this->projMat, sizeof(Matrix));
			
		this->context->Unmap(this->projectionCBuffer, 0);

		cBuffer[0] = this->projectionCBuffer;
		this->context->VSSetConstantBuffers(2, 1, cBuffer);
		this->context->PSSetConstantBuffers(2, 1, cBuffer);
		this->context->GSSetConstantBuffers(2, 1, cBuffer);
	}

	//todo, see if srvs are ubound from pipeline when teh shader is switched
	void Renderer::drawParticles(void)
	{
		ID3D11ShaderResourceView *particleTextureSRVArr = {this->particleTextureSRV};
		ID3D11ShaderResourceView *nullSRVArr = { NULL };
		ID3D11SamplerState *samplerArr = {this->defaultSampler};
		ID3D11SamplerState *nullSamplerArr = {NULL};
		ID3D11ShaderResourceView *particleBufferSRVArr = {this->particleSRV0};

		/*this->Context->PSSetShader(this->PSPShader, NULL, 0);
		this->Context->VSSetShader(this->PSVShader, NULL, 0);
		this->Context->GSSetShader(this->PSGShader, NULL, 0);*/

		this->BindShader(SHADER_TYPE_POINT_SPRITE);

		/*XMMATRIX mat = XMMatrixIdentity();
		XMFLOAT4X4 identity;
		XMStoreFloat4x4(&identity, mat);*/
		//this->SetTransform(TRANSFORM_WORLD, Matrix::Identity());
		
		this->rebindParticleCBs();
		//this->rebindTransformCBs();

		this->context->PSSetShaderResources(0, 1, &particleTextureSRVArr);
		this->context->PSSetSamplers(1, 1, &samplerArr);
		this->context->GSGetShaderResources(2, 1, &particleBufferSRVArr);

		ID3D11Buffer *vBufferArr = {this->particleVBuffer};
		UINT stride[] = { sizeof(UINT) };
		UINT offset[] = { 0 };
		this->context->IASetVertexBuffers(0, 1, &vBufferArr, stride, offset);

		this->context->IASetInputLayout(this->PSInputLayout);
		this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		this->context->Draw(numParticles, 0);
		
		//this->Context->IASetVertexBuffers(0, 1, &nullVBufferArr, NULL, NULL);

		this->context->IASetInputLayout(NULL);

		this->context->PSSetShaderResources(0,1, &nullSRVArr);
		
		/*this->Context->PSSetShader(NULL, NULL, 0);
		this->Context->VSSetShader(NULL, NULL, 0);
		this->Context->GSSetShader(NULL, NULL, 0);*/
		
		this->UnbindShader();
	}
	
	void Renderer::BindShader(SHADER_TYPE type)
	{
		/*if(type == this->boundShader)
			return;*/

		this->boundShader = type;

		switch(type)
		{
		case SHADER_TYPE_COLOR:
			this->context->PSSetShader(this->colorPShader, nullptr, 0);
			this->context->VSSetShader(this->colorVShader, nullptr, 0);
			this->context->GSSetShader(nullptr, nullptr, 0);

			this->context->IASetInputLayout(this->colorInputLayout);
			break;

		case SHADER_TYPE_POINT_SPRITE:
			this->context->PSSetShader(this->PSPShader, nullptr, 0);
			this->context->VSSetShader(this->PSVShader, nullptr, 0);
			this->context->GSSetShader(this->PSGShader, nullptr, 0);

			this->context->IASetInputLayout(this->PSInputLayout);
			break;

		case SHADER_TYPE_TEXTURE:
			this->context->PSSetShader(this->texturePShader, nullptr, 0);
			this->context->VSSetShader(this->textureVShader, nullptr, 0);
			this->context->GSSetShader(nullptr, nullptr, 0);

			this->context->IASetInputLayout(this->textureInputLayout);
			break;

		case SHADER_TYPE_NONE:
			this->context->PSSetShader(nullptr, nullptr, 0);
			this->context->VSSetShader(nullptr, nullptr, 0);
			this->context->GSSetShader(nullptr, nullptr, 0);

			this->context->IASetInputLayout(nullptr);
			break;
		}

		this->rebindTransformCBs();
	}

	void Renderer::UnbindShader(void)
	{
		this->BindShader(SHADER_TYPE_NONE);
	}

	void Renderer::SetColor(const Vector4 &color)
	{
		if(this->boundShader != SHADER_TYPE_COLOR)
		{
			OutputDebugString("\nInvalid call on SetColor(), Color shader is not bound\n");	
		}

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->context->Map(this->colorCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
		{
			OutputDebugString("\nSetColor:Context->Map() failed\n");
			return;
		}

		memcpy(mappedSubresource.pData, &color, sizeof(XMFLOAT4));
		this->context->Unmap(this->colorCBuffer, 0);

		ID3D11Buffer *cBufferArr[1] = {this->colorCBuffer};

		this->context->VSSetConstantBuffers(3, 1,cBufferArr);
		this->context->PSSetConstantBuffers(3, 1, cBufferArr);
	}

	void Renderer::BeginText(void)
	{
		this->SBatch->Begin();
	}

	void Renderer::RenderText(const std::wstring &txt, float screenPosX, float screenPosY)
	{
		XMFLOAT2 windowPos = XMFLOAT2(screenPosX * this->bbWidth, screenPosY * this->bbHeight);
		this->SFont->DrawString(&(*this->SBatch), txt.data(), windowPos, Colors::White, 0.0f, XMFLOAT2(0,0), 1.0f, DirectX::SpriteEffects_None, 0.0f);
	}

	void Renderer::EndText(void)
	{
		this->SBatch->End();
		this->context->RSSetState(rsDefault);
	}

	void Renderer::BeginRender(void)
	{
		//this->worldMat = Matrix::Identity();
		//this->projMat = Matrix::CreatePerspectiveFieldOfView(3.14159f / 4.0f, 1.0f, 0.5f, 100.0f);

		float color[4] = {0.5f, 0.5f, 0.0f, 0.0f};
		this->context->ClearRenderTargetView(this->RTView, color);

		//this->context->RSSetState(this->rsDisableCull);
	}

	void Renderer::EndRender(void)
	{
		this->swapChain->Present(1, 0);
		this->context->RSSetState(NULL);
	}
}
