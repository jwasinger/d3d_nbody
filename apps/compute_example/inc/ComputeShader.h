#pragma once
#pragma warning(disable: 4005)
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
#include <dxgidebug.h>
#pragma warning(default: 4005)
#include <atlbase.h>
#include <vector>
#include <tuple>
#include <memory>


using namespace std;


class ComputeShader
{
private:
	IDXGIAdapter* m_pAdapter;
	CComPtr<ID3D11Device> m_pDevice;
	CComPtr<ID3D11DeviceContext> m_pContext;
	CComPtr<ID3D11ComputeShader> m_pComputeShader;
	vector<CComPtr<ID3D11Buffer>> m_listOutput;
	bool m_bTimeProfilingOK = false;
#ifdef DEBUG
	float m_timeGPUExecution = 0.0f;
	CComPtr<ID3D11Debug> m_pDebug;
	CComPtr<ID3D11InfoQueue> m_pDebugInfoQueue;
#endif

	bool CreateDevice();

	bool CreateStructuredBuffer(LPVOID pInitData,
								UINT SizeElement,
								UINT NumElements,
								CComPtr<ID3D11Buffer>& pBufferOut);
	bool CreateConstantBuffer(LPVOID pInitData,
							  UINT SizeElement,
							  UINT NumElements,
							  CComPtr<ID3D11Buffer>& pBufferOut);
	bool CreateBufferSRV(ID3D11Buffer* pBuffer, CComPtr<ID3D11ShaderResourceView>& pSRVOut);
	bool CreateBufferUAV(ID3D11Buffer* pBuffer, CComPtr<ID3D11UnorderedAccessView>& pUAVOut);
	bool CollectOutputFromGPU(const CComPtr<ID3D11Buffer> listBuffer[],
							  size_t Count,
							  vector<CComPtr<ID3D11Buffer>>& listOutput);

public:
	enum DATA_TYPE
	{
		StructuredBuffer,
		Texture1D,
		Texture2D,
		Texture3D
	};

	ComputeShader();
	~ComputeShader();

	bool CompileShader(wstring ShaderFile,
					   string StartFunctionName,
					   unsigned int numThreadsX,
					   unsigned int numThreadsY,
					   unsigned int numThreadsZ);
	bool RunShader(unsigned int numBlocksX,
					unsigned int numBlocksY,
					unsigned int numBlocksZ,
					const vector<tuple<void *, unsigned int, unsigned int>>& listInputData,
					const vector<tuple<unsigned int, unsigned int>>& listOutputData,
					const vector<tuple<void *, unsigned int, unsigned int>>& listConstantData = vector<tuple<void *, unsigned int, unsigned int>>());
	template<typename T> T *Result(size_t Index)
	{
		if (Index >= m_listOutput.size())
			throw exception("Index is out of bounds");

		D3D11_MAPPED_SUBRESOURCE MappedResource;

		if (FAILED(m_pContext->Map(m_listOutput[Index], 0, D3D11_MAP_READ, 0, &MappedResource)))
			return nullptr;

		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		// This is also a common trick to debug CS programs.
		return static_cast<T *>(MappedResource.pData);
	}

	ID3D11Device *GetDevice() { return m_pDevice; }
#ifdef DEBUG
	float GetExecutionTime(bool *pProfilingDisjoint = NULL)
	{
		if (pProfilingDisjoint)
			*pProfilingDisjoint = m_bTimeProfilingOK;

		return m_timeGPUExecution;
	}
#endif
};

