#ifndef SIMULATION_H
#define SIMULATION_H

#include "common_include.h"
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include <d3d11.h>
#include "Renderer.h"

using namespace DirectX;

namespace NBody
{
	struct PosVel
	{
		XMFLOAT4 Position;
		XMFLOAT4 Velocity;
	};

	struct SimParams
	{
		UINT param[4];		//param[0] = number of particles

		float param_f[4];	//param_f[0] = delta time in milliseconds
	};

	class NBodySim
	{
	private:
		enum {STOPPED, PAUSED, RUNNING};

		CComPtr<ID3D11DeviceContext> context;
		CComPtr<ID3D11Device> device;

		CComPtr<ID3D11ComputeShader> cShader;

		CComPtr<ID3D11Texture2D> uavTexture;
		CComPtr<ID3D11UnorderedAccessView> uav;

		CComPtr<ID3D11Texture2D> csOutputTexture;
		CComPtr<ID3D11ShaderResourceView> csOutputTextureSRV;

		CComPtr<ID3D11UnorderedAccessView> posVelUAV0;
		CComPtr<ID3D11UnorderedAccessView> posVelUAV1;

		CComPtr<ID3D11ShaderResourceView> posVelSRV0;
		CComPtr<ID3D11ShaderResourceView> posVelSRV1;

		CComPtr<ID3D11Buffer> posVelBuffer0;
		CComPtr<ID3D11Buffer> posVelBuffer1;
		CComPtr<ID3D11Buffer> simCBuffer;

		int frameCount;
		int frameRate;
		int state;
		int numBodies;
		// amount of elapsed time in seconds since the frame rate was last calculated
		double frameTimerElapsedSec; 

	public:
		NBodySim(void);
		~NBodySim(void);
		
		const CComPtr<ID3D11ShaderResourceView> GetPosVelSRV0() const { return this->posVelSRV0; }
		const CComPtr<ID3D11ShaderResourceView> GetPosVelSRV1() const { return this->posVelSRV1; }
		
		const CComPtr<ID3D11UnorderedAccessView> GetPosVelUAV0() const { return this->posVelUAV0; }
		const CComPtr<ID3D11UnorderedAccessView> GetPosVelUAV1() const { return this->posVelUAV1; }
		
		const CComPtr<ID3D11Buffer> GetPosVelBuffer1() const { return this->posVelBuffer1; }
		const CComPtr<ID3D11Buffer> GetPosVelBuffer0() const { return this->posVelBuffer0; }

		int GetNumBodies() const { return this->numBodies; }

		bool Init(ID3D11DeviceContext *deviceContext, ID3D11Device *device, int numBodies);

		void Tick(double elapsedMS);

	private:
		bool createComputeShader(LPCWSTR path);
		void setCBs(double elapsedMS);
		XMFLOAT3 *createBodies(void);

	};
}
#endif

