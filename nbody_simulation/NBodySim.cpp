#include "NBodySim.h"

namespace NBody
{

	NBodySim::NBodySim(void)
	{

	}

	XMFLOAT3 *NBodySim::createBodies(void)
	{
		XMFLOAT3 *bodyArr = new XMFLOAT3[this->numBodies];
		
		int count = 0;
		for(int x = 0; x < 100; x++)
		{
			for(int y = 0; y < 100; y++)
			{
				for(int z = 0; z < 100; z++)
				{
					if(count == this->numBodies)
					{
						return bodyArr;
					}

					bodyArr[count] = XMFLOAT3(x,y,z);
					count++;
				}
			}
		}

		return NULL; // execution should not reach this statement.
	}

	bool NBodySim::Init(ID3D11DeviceContext *deviceContext, ID3D11Device *device, int numBodies)
	{
		this->device = device;
		this->context = deviceContext;
		this->numBodies = numBodies;

		if(!this->createComputeShader(GetFilePath(L"compute.hlsl").data()))
			return false;
	
		//create particle structured buffers -----------------------------------------------------------------------------------------
		D3D11_BUFFER_DESC pvBufferDesc;
		pvBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		pvBufferDesc.ByteWidth = sizeof(PosVel) * this->numBodies;
		pvBufferDesc.CPUAccessFlags = 0;
		pvBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		pvBufferDesc.StructureByteStride = sizeof(PosVel);
		pvBufferDesc.Usage = D3D11_USAGE_DEFAULT;


		D3D11_SUBRESOURCE_DATA initialBodyData;
		ZeroMemory(&initialBodyData, sizeof(D3D11_SUBRESOURCE_DATA));
		XMFLOAT3 *data = this->createBodies();
		initialBodyData.pSysMem = (void *)data;
		
		if(FAILED(this->device->CreateBuffer(&pvBufferDesc, NULL, &this->posVelBuffer0)))
		{
			OutputDebugString("\ncreate posVelBuffer0 failed\n");
			return false;
		}

		if(initialBodyData.pSysMem)
			delete [] initialBodyData.pSysMem;
		
		if(FAILED(this->device->CreateBuffer(&pvBufferDesc, NULL, &this->posVelBuffer1)))
		{
			OutputDebugString("\ncreate posVelBuffer1 failed\n");
			return false;
		}

		//Create SRVs-------------------------------------------------------------------------------------------------------------
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		
		srvDesc.Buffer.ElementOffset = 0;
		srvDesc.Buffer.ElementWidth = sizeof(float)*8;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = this->numBodies;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;

		if(FAILED(this->device->CreateShaderResourceView(&(*this->posVelBuffer0), &srvDesc, &this->posVelSRV0)))
		{
			OutputDebugString("\nCreate posVelSRV0 failed\n");
			return false;
		}
			
		if(FAILED(this->device->CreateShaderResourceView(&(*this->posVelBuffer1), &srvDesc, &this->posVelSRV1)))
		{
			OutputDebugString("\nCreate posVelSRV1 failed\n");
			return false;
		}

		//create UAVs----------------------------------------------------------------------------------------------------------------------
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = this->numBodies;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		
		if(FAILED(this->device->CreateUnorderedAccessView(&(*this->posVelBuffer0), &uavDesc, &this->posVelUAV0)))
		{
			OutputDebugString("\ncreate posVelUAV0 failed\n");
			return false;
		}

		if(FAILED(this->device->CreateUnorderedAccessView(&(*this->posVelBuffer1), &uavDesc, &this->posVelUAV1)))
		{
			OutputDebugString("\ncreate posVelUAV1 failed\n");
			return false;
		}

		D3D11_BUFFER_DESC cBufferDesc;
		ZeroMemory(&cBufferDesc, sizeof(D3D11_BUFFER_DESC));
		cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cBufferDesc.ByteWidth = sizeof(NBody::SimParams);
		cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		if(FAILED(this->device->CreateBuffer(&cBufferDesc, nullptr, &this->simCBuffer)))
		{
			OutputDebugString("\nCreate simCBuffer failed\n");
			return false;
		}

		return true;
	}

	bool NBodySim::createComputeShader(LPCWSTR path)
	{
		HRESULT result = 0;
		//LPCSTR target = ( this->device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";
		LPCSTR target = "cs_5_0";
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		ID3DBlob* blob = nullptr;
		ID3DBlob* errorMessageBlob = nullptr;

	#ifdef _DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG;
	#endif

		result = D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,  "main", target, shaderFlags, 0, &blob, &errorMessageBlob);
		if(FAILED(result))
		{
			if(errorMessageBlob)
			{
				OutputDebugString("\n");
				OutputDebugStringA((LPCSTR)errorMessageBlob->GetBufferPointer()); //access violation on this line
				OutputDebugString("\n");
			}
			SafeRelease<ID3DBlob>(&blob);
			SafeRelease<ID3DBlob>(&errorMessageBlob);

			return false;
		}
	
		if(FAILED(this->device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(),nullptr, &this->cShader)))
		{
			return false;
		}

		return true;
	}

	inline void swap(void **a, void **b)
	{
		void *tmp = *a;
		(*a) = (*b);
		(*b) = (tmp);
	}

	void NBodySim::setCBs(double elapsedMS)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->context->Map(this->simCBuffer, 0, D3D11_MAP_WRITE, 0, &mappedSubresource)))
		{
			OutputDebugString("\nMap() on simCBuffer failed\n");
			return;
		}

		NBody::SimParams params;
		ZeroMemory(&params, sizeof(NBody::SimParams));
		params.param[0] = this->numBodies;
		params.param_f[0] = elapsedMS;

		memcpy(mappedSubresource.pData, &params, sizeof(NBody::SimParams));

		this->context->Unmap(this->simCBuffer, 0);
	}

	void NBodySim::Tick(double elapsed)
	{
		//-------------------------------------
		//test harness for rendering only
		//-------------------------------------

		//this->context->CSSetShader(this->cShader, nullptr, 0);

		//ID3D11UnorderedAccessView *uav = {this->posVelUAV1};
		//ID3D11ShaderResourceView *srv = {this->posVelSRV0};
		//ID3D11UnorderedAccessView *uavNull = {nullptr};
		//ID3D11ShaderResourceView *srvNull = {nullptr};

		//this->context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
		//this->context->CSSetShaderResources(0,1,&srv);

		//this->setCBs(elapsed);

		//this->context->Dispatch(this->numBodies / 128,1,1);
		//
		////remove SRVs, UAVs, Buffers from the pipeline.
		//this->context->CSSetShaderResources(0, 1, &srvNull);
		//this->context->CSSetUnorderedAccessViews(0,1,&uavNull,nullptr);
		//this->context->CSSetShader(nullptr, nullptr, 0);

		////does the following code produce correct results...?
		//swap((void**)&this->posVelBuffer0, (void**)&this->posVelBuffer1);
		//swap((void**)&this->posVelUAV0, (void**)&this->posVelUAV1);
		//swap((void**)&this->posVelSRV0, (void**)&this->posVelSRV1);
	}

	/*void NBodySim::Update(double elapsed)
	{
		this->frameTimerElapsedSec += elapsed;
		this->frameCount++;

		if(this->frameTimerElapsedSec >= 1.0)
		{
			this->frameRate = frameCount;
			this->frameCount = 0;
			this->frameTimerElapsedSec = 0.0;

			this->tickSim(elapsed);
		}
	}*/

	NBodySim::~NBodySim(void)
	{

	}
}