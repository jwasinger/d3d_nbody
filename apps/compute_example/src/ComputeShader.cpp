#include "ComputeShader.h"


ComputeShader::ComputeShader()
{
	// Try to create a new hardware device, if this fails
	//	there's no use continuing
	if (!CreateDevice())
		throw exception("Couldn't create a new DirectX hardware device");
}

ComputeShader::~ComputeShader() { m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL); }

bool ComputeShader::CreateDevice()
{
	UINT CreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef DEBUG
	CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL OutputFeatureLevel;
	static const D3D_FEATURE_LEVEL RequestedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDevice(NULL,
									D3D_DRIVER_TYPE_HARDWARE,
									NULL,
									CreationFlags,
									&RequestedFeatureLevel,
									sizeof(RequestedFeatureLevel) / sizeof(D3D_FEATURE_LEVEL),
									D3D11_SDK_VERSION,
									&m_pDevice,
									&OutputFeatureLevel,
									&m_pContext)))
		return false;

#ifdef DEBUG
	if (SUCCEEDED(m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug)))
	{
		if (SUCCEEDED(m_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_pDebugInfoQueue)))
		{
			m_pDebugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			m_pDebugInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
		}
	}
#endif

	return true;
}

bool ComputeShader::CreateStructuredBuffer(LPVOID pInitData,
											UINT SizeElement,
											UINT NumElements,
											CComPtr<ID3D11Buffer>& pBufferOut)
{
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));

	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.ByteWidth = SizeElement * NumElements;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = SizeElement;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return SUCCEEDED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &pBufferOut));
	}

	return SUCCEEDED(m_pDevice->CreateBuffer(&BufferDesc, NULL, &pBufferOut));
}

bool ComputeShader::CreateConstantBuffer(LPVOID pInitData,
										  UINT SizeElement,
										  UINT NumElements,
										  CComPtr<ID3D11Buffer>& pBufferOut)
{
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));

	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = SizeElement * NumElements;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = NumElements;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pInitData;

	return SUCCEEDED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &pBufferOut));
}

bool ComputeShader::CreateBufferSRV(ID3D11Buffer* pBuffer, CComPtr<ID3D11ShaderResourceView>& pSRVOut)
{
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	pBuffer->GetDesc(&BufferDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc;
	ZeroMemory(&ViewDesc, sizeof(ViewDesc));
	ViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	ViewDesc.BufferEx.FirstElement = 0;

	if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)	// This is a Structured Buffer
	{
		ViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		ViewDesc.BufferEx.NumElements = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
	}
	else
		return false;

	return SUCCEEDED(m_pDevice->CreateShaderResourceView(pBuffer, &ViewDesc, &pSRVOut));
}

bool ComputeShader::CreateBufferUAV(ID3D11Buffer* pBuffer, CComPtr<ID3D11UnorderedAccessView>& pUAVOut)
{
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	pBuffer->GetDesc(&BufferDesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC Desc;
	ZeroMemory(&Desc, sizeof(Desc));
	Desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	Desc.Buffer.FirstElement = 0;

	if (BufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)	// This is a Structured Buffer
	{
		Desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		Desc.Buffer.NumElements = BufferDesc.ByteWidth / BufferDesc.StructureByteStride;
	}
	else
		return false;

	return SUCCEEDED(m_pDevice->CreateUnorderedAccessView(pBuffer, &Desc, &pUAVOut));
}

bool ComputeShader::CollectOutputFromGPU(const CComPtr<ID3D11Buffer> listBuffer[],
										  size_t Count,
										  vector<CComPtr<ID3D11Buffer>>& listOutput)
{
	for (size_t i = 0; i < Count; i++)
	{
		D3D11_BUFFER_DESC Desc;
		ZeroMemory(&Desc, sizeof(Desc));

		listBuffer[i]->GetDesc(&Desc);
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		Desc.Usage = D3D11_USAGE_STAGING;
		Desc.BindFlags = 0;
		Desc.MiscFlags = 0;

		CComPtr<ID3D11Buffer> pDebugBuffer;

		if (SUCCEEDED(m_pDevice->CreateBuffer(&Desc, NULL, &pDebugBuffer)))
		{
			m_pContext->CopyResource(pDebugBuffer, listBuffer[i]);
			listOutput.push_back(pDebugBuffer);
		}
		else
			return false;
	}

	return true;
}

bool ComputeShader::CompileShader(wstring ShaderFile,
								  string StartFunctionName,
								  unsigned int numThreadsX,
								  unsigned int numThreadsY,
								  unsigned int numThreadsZ)
{
	char szNUM_OF_THREADS_X[8], szNUM_OF_THREADS_Y[8], szNUM_OF_THREADS_Z[8];
	_itoa(numThreadsX, szNUM_OF_THREADS_X, 10);
	_itoa(numThreadsY, szNUM_OF_THREADS_Y, 10);
	_itoa(numThreadsZ, szNUM_OF_THREADS_Z, 10);

	const D3D_SHADER_MACRO MacroDefines[] =
	{
		"NUM_OF_THREADS_X", szNUM_OF_THREADS_X,
		"NUM_OF_THREADS_Y", szNUM_OF_THREADS_Y,
		"NUM_OF_THREADS_Z", szNUM_OF_THREADS_Z,
		NULL, NULL
	};

	DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	ShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	CComPtr<ID3DBlob> pBlob, pErrorMsgs;

	if (FAILED(D3DX11CompileFromFile(ShaderFile.c_str(),
		MacroDefines,
		NULL,
		StartFunctionName.c_str(),
		"cs_5_0",
		ShaderFlags,
		0,
		NULL,
		&pBlob,
		&pErrorMsgs,
		NULL)))
	{
#ifdef DEBUG
		if (pErrorMsgs != NULL)
			OutputDebugStringA((LPCSTR) pErrorMsgs->GetBufferPointer());
#endif

		return false;
	}

	return SUCCEEDED(m_pDevice->CreateComputeShader(pBlob->GetBufferPointer(),
													pBlob->GetBufferSize(),
													NULL,
													&m_pComputeShader));
}

bool ComputeShader::RunShader(unsigned int numBlocksX,
								unsigned int numBlocksY,
								unsigned int numBlocksZ,
								const vector<tuple<void *, unsigned int, unsigned int>>& listInputData,
								const vector<tuple<unsigned int, unsigned int>>& listOutputData,
								const vector<tuple<void *, unsigned int, unsigned int>>& listConstantData)
{
	if (listInputData.size() > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT ||
		listOutputData.size() > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
		return false;


	// Create structured buffer for input data
	unique_ptr<CComPtr<ID3D11Buffer>[]> listInputBufferDataPtr(new CComPtr<ID3D11Buffer>[listInputData.size()]);

	for (size_t i = 0; i < listInputData.size(); i++)
	{
		CComPtr<ID3D11Buffer> pBufferData;

		if (CreateStructuredBuffer(get<0>(listInputData[i]),
			get<1>(listInputData[i]),
			get<2>(listInputData[i]),
			pBufferData))
			listInputBufferDataPtr[i] = pBufferData;
		else
			return false;
	}

	// Create structured buffer for output data
	unique_ptr<CComPtr<ID3D11Buffer>[]> listOutputBufferDataPtr(new CComPtr<ID3D11Buffer>[listOutputData.size()]);

	for (size_t i = 0; i < listOutputData.size(); i++)
	{
		CComPtr<ID3D11Buffer> pBufferData;

		if (CreateStructuredBuffer(NULL,
			get<0>(listOutputData[i]),
			get<1>(listOutputData[i]),
			pBufferData))
			listOutputBufferDataPtr[i] = pBufferData;
		else
			return false;
	}

	// Create resource view for input structured buffer
	unique_ptr<CComPtr<ID3D11ShaderResourceView>[]> listInputResourceView(new CComPtr<ID3D11ShaderResourceView>[listInputData.size()]);

	for (size_t i = 0; i < listInputData.size(); i++)
	{
		CComPtr<ID3D11ShaderResourceView> pResourceView;

		if (CreateBufferSRV(listInputBufferDataPtr[i], pResourceView))
			listInputResourceView[i] = pResourceView;
		else
			return false;
	}

	// Create resource view for output structured buffer
	unique_ptr<CComPtr<ID3D11UnorderedAccessView>[]> listOutputResourceView(new CComPtr<ID3D11UnorderedAccessView>[listOutputData.size()]);

	for (size_t i = 0; i < listOutputData.size(); i++)
	{
		CComPtr<ID3D11UnorderedAccessView> pResourceView;

		if (CreateBufferUAV(listOutputBufferDataPtr[i], pResourceView))
			listOutputResourceView[i] = pResourceView;
		else
			return false;
	}

	// Create array version of input
	unique_ptr<ID3D11ShaderResourceView *[]> plistInputResourceView(new ID3D11ShaderResourceView *[listInputData.size()]);
	for (size_t i = 0; i < listInputData.size(); i++)
		plistInputResourceView[i] = listInputResourceView[i].p;

	// Create array version of output
	unique_ptr<ID3D11UnorderedAccessView *[]> plistOutputResourceView(new ID3D11UnorderedAccessView *[listOutputData.size()]);
	for (size_t i = 0; i < listOutputData.size(); i++)
		plistOutputResourceView[i] = listOutputResourceView[i].p;

	m_pContext->CSSetShader(m_pComputeShader, NULL, 0);
	m_pContext->CSSetShaderResources(0, (UINT)listInputData.size(), plistInputResourceView.get());
	m_pContext->CSSetUnorderedAccessViews(0, (UINT)listOutputData.size(), plistOutputResourceView.get(), NULL);

	// This section is required to implement constant handling
	unique_ptr<CComPtr<ID3D11Buffer>[]> listConstantBufferDataPtr;
	unique_ptr<ID3D11Buffer *[]> plistConstantBuffer;
	if (listConstantData.size() != 0)
	{
		// Create structured buffer for constant data
		listConstantBufferDataPtr.reset(new CComPtr<ID3D11Buffer>[listConstantData.size()]);
		plistConstantBuffer.reset(new ID3D11Buffer *[listConstantData.size()]);

		for (size_t i = 0; i < listConstantData.size(); i++)
		{
			CComPtr<ID3D11Buffer> pBufferData;

			if (CreateConstantBuffer(get<0>(listConstantData[i]),
				get<1>(listConstantData[i]),
				get<2>(listConstantData[i]),
				pBufferData))
			{
				listConstantBufferDataPtr[i] = pBufferData;
				plistConstantBuffer[i] = pBufferData;
			}
			else
				return false;
		}

		m_pContext->CSSetConstantBuffers(0, (UINT)listConstantData.size(), plistConstantBuffer.get());
	}

#ifdef DEBUG
	// Profile execution in GPU
	CComPtr<ID3D11Query> pQueryFreq, pQueryTicks;
	D3D11_QUERY_DESC queryDesc;

	queryDesc.MiscFlags = 0;
	queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	m_pDevice->CreateQuery(&queryDesc, &pQueryFreq);
	queryDesc.Query = D3D11_QUERY_TIMESTAMP;
	m_pDevice->CreateQuery(&queryDesc, &pQueryTicks);

	UINT64 timeStart = 0, timeEnd = 0;

	m_pContext->Begin(pQueryFreq);	// Begin profiling

	m_pContext->End(pQueryTicks);
	while (m_pContext->GetData(pQueryTicks, NULL, 0, 0) == S_FALSE);
	m_pContext->GetData(pQueryTicks, &timeStart, sizeof(timeStart), 0);
#endif

	// Dispatch work to GPU using the specified no of blocks
	m_pContext->Dispatch(numBlocksX, numBlocksY, numBlocksZ);

#ifdef DEBUG
	m_pContext->End(pQueryTicks);
	while (m_pContext->GetData(pQueryTicks, NULL, 0, 0) == S_FALSE);
	m_pContext->GetData(pQueryTicks, &timeEnd, sizeof(timeEnd), 0);

	m_pContext->End(pQueryFreq);	// End profiling

	while (m_pContext->GetData(pQueryFreq, NULL, 0, 0) == S_FALSE);

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT querydataDisjoint = { 0 };
	m_pContext->GetData(pQueryFreq, &querydataDisjoint, sizeof(querydataDisjoint), 0);

	if (querydataDisjoint.Frequency != 0)
		m_timeGPUExecution = static_cast<float>(timeEnd - timeStart) / querydataDisjoint.Frequency;
	
	m_bTimeProfilingOK = querydataDisjoint.Disjoint == FALSE;
#endif

	// Release shader
	m_pContext->CSSetShader(NULL, NULL, 0);

	// According to documentations, this house keeping is required
	//	We need to set resource view pointers to null for this device context
	ZeroMemory(plistInputResourceView.get(), sizeof(ID3D11ShaderResourceView *)* listInputData.size());
	ZeroMemory(plistOutputResourceView.get(), sizeof(ID3D11ShaderResourceView *)* listOutputData.size());

	m_pContext->CSSetUnorderedAccessViews(0, (UINT)listOutputData.size(), plistOutputResourceView.get(), NULL);
	m_pContext->CSSetShaderResources(0, (UINT)listInputData.size(), plistInputResourceView.get());

	if (listConstantData.size() != 0)	// If there were constants specified, release them too
	{
		ZeroMemory(plistConstantBuffer.get(), sizeof(ID3D11Buffer *)* listConstantData.size());

		m_pContext->CSSetConstantBuffers(0, (UINT)listConstantData.size(), plistConstantBuffer.get());
	}

	return CollectOutputFromGPU(listOutputBufferDataPtr.get(), listOutputData.size(), m_listOutput);
}