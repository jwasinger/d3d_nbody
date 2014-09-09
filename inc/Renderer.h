#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcommon.h>

#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <WICTextureLoader.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <d3dcompiler.h>


#include "common_include.h"
#include "Vertex.h"
#include "Camera3D.h"

#include "Timer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace NBody
{
	enum SHADER_TYPE
	{
		SHADER_TYPE_NONE = 0,
		SHADER_TYPE_COLOR,
		SHADER_TYPE_POINT_SPRITE,
		SHADER_TYPE_TEXTURE,
	};

	enum TRANSFORM_TYPE : UINT
	{
		TRANSFORM_WORLD,
		TRANSFORM_PROJECTION,
		TRANSFORM_VIEW,
	};

	class Renderer
	{
	private:
		struct ParticleDrawParams;

	private:
		int numParticles;
		HWND hWnd;
		
		int bbWidth, bbHeight;
		SHADER_TYPE boundShader;

		IDXGIFactory *factory;
		IDXGIAdapter *adapter;
		IDXGISwapChain *swapChain;
		ID3D11Device *device;
		ID3D11DeviceContext *context;
		ID3D11RenderTargetView *RTView;
	    DirectX::SpriteBatch *SBatch;
	    DirectX::SpriteFont *SFont;

		ID3D11Resource *particleTexture;
		ID3D11ShaderResourceView *particleTextureSRV;

		ID3D11InputLayout *PSInputLayout;
		ID3D11InputLayout *textureInputLayout;

		ID3D11Buffer *quadVBuffer;
		ID3D11Buffer *particleVBuffer;
		
		CComPtr<ID3D11Buffer> particleBuffer0;
		CComPtr<ID3D11Buffer> particleBuffer1;
		CComPtr<ID3D11ShaderResourceView> particleSRV0;
		CComPtr<ID3D11ShaderResourceView> particleSRV1;

		ID3D11Buffer *pDrawParamsBuffer;

		ID3D11VertexShader *textureVShader;
		ID3D11PixelShader *texturePShader;
		
		ID3D11VertexShader *PSVShader;
		ID3D11PixelShader *PSPShader;
		ID3D11GeometryShader *PSGShader;

		ID3D11VertexShader *colorVShader;
		ID3D11PixelShader *colorPShader;
		ID3D11InputLayout *colorInputLayout;
		ID3D11Buffer *colorCBuffer;

		std::vector<Matrix> worldStack;
		std::vector<Matrix> projStack;
		std::vector<Matrix> viewStack;
		Matrix worldMat;
		Matrix projMat;
		Matrix viewMat;
		Matrix invWorldMat;
		Matrix invProjMat;
		Matrix invViewMat;
		ID3D11Buffer *worldCBuffer;
		ID3D11Buffer *viewCBuffer;
		ID3D11Buffer *projectionCBuffer;
		

		ID3D11RasterizerState *rsDisableCull;
		ID3D11RasterizerState *rsCullFront;
		ID3D11RasterizerState *rsDefault; //default rasterizer state with back-face culling
		
		ID3D11SamplerState *defaultSampler;

		//debug draw stuff----------------------------

		//camera stuff
		/*Matrix rotation;
		Vector3 translation;*/

	public:
		Renderer();
		~Renderer();

		bool Init(const RECT &windowRect, HWND hWnd);
		bool CreateParticleBuffers(int numParticles, NBody::Particle * const positions, ID3D11Buffer** buffer0, ID3D11Buffer** buffer1);

		void BeginRender(void);
		//void RenderDebugInfo(void);
		void RenderParticles(void);
		void EndRender(void);

		void BindShader(SHADER_TYPE type);
		void UnbindShader(void);

		SHADER_TYPE GetBoundShader(void) const { return this->boundShader; }
		Matrix GetTransform(TRANSFORM_TYPE type) const;
		Matrix GetInvTransform(TRANSFORM_TYPE type) const; //invert the result of multiplying all matrices on the matrix stack

		void SetColor(const Vector4 &color);
		void PushMatrix(TRANSFORM_TYPE type, const Matrix &value);
		void PopMatrix(TRANSFORM_TYPE type);
		//bool StackHasMatrix(TRANSFORM_TYPE type);
		void SetStack(bool enable);

		void SetCullMode(D3D11_CULL_MODE mode);

		void BeginText(void);
		void RenderText(const std::wstring &txt, float screenPosX, float screenPosY);
		void EndText(void);

		//camera stuff
		/*void CameraTranslateInView(const Vector3 &translation);
		void CameraTranslateInWorld(const Vector3 &translation);
		void CameraRotateInView(const Matrix &mat);
		void CameraRotateInWorld(const Matrix &mat);
		void CameraLookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)*/;

		IDXGIFactory *const GetFactory(void) { return this->factory; }
		IDXGIAdapter *const GetAdapter(void) { return this->adapter; }
		IDXGISwapChain *const GetSwapChain(void) { return this->swapChain; }
		ID3D11Device *const GetDevice(void) { return this->device; }
		ID3D11DeviceContext *const GetDeviceContext(void) { return this->context; }
		ID3D11RenderTargetView *const GetRTView(void) { return this->RTView; }
		DirectX::SpriteBatch *const GetSpriteBatch(void) { return this->SBatch; }
		DirectX::SpriteFont *const GetSpriteFont(void) {return this->SFont; }
	private:
		//bool debug_init(void);
		void debug_render(void);
		void debug_RenderText(void);
		void debug_drawMatrix(const Matrix &m, const RECTF &outputRect);

		void drawParticles(void);

		void drawTestTri(void);
		
		bool createCBuffers(void);
		bool createShadersAndInputLayouts(void);
		bool createSamplers(void);
		bool createRasterizerStates(void);

		void rebindTransformCBs(void);
		void rebindParticleCBs(void);
		bool isGShaderBound(void);

		//camera stuff
		void buildView(void);
		void buildViewFromMat(void);
	private:
		struct ParticleDrawParams
		{
			float radius;
			DirectX::XMFLOAT4X4 invView;
			float padding[3];
		};
	};
}
#endif

