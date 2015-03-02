#include "RayTracer.h"

namespace Core
{
	RayTracer::RayTracer(Renderer *renderer)
	{
		this->renderer = renderer;
	}

	bool RayTracer::Init(void)
	{
		HRESULT res;

		D3D11_TEXTURE2D_DESC compute_output_desc;
		compute_output_desc.ArraySize = 1;
		compute_output_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		compute_output_desc.CPUAccessFlags = 0;
		compute_output_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		compute_output_desc.Width = CS_OUTPUT_WIDTH;
		compute_output_desc.Height = CS_OUTPUT_HEIGHT;
		compute_output_desc.MipLevels = 1;
		compute_output_desc.MiscFlags = 0;
		compute_output_desc.SampleDesc.Count = 1;
		compute_output_desc.SampleDesc.Quality = 0;
		compute_output_desc.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED(res = this->renderer->GetDevice()->CreateTexture2D(
			&compute_output_desc,
			NULL,
			&this->compute_output)))
		{
			log_str("CreateTexture2D Error: %s\n", get_err_str(res));
			return false;
		}

		D3D11_BUFFER_DESC v_buf_desc;
		v_buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		v_buf_desc.ByteWidth = sizeof(TexturedVertex) * 6;
		v_buf_desc.CPUAccessFlags = 0;
		v_buf_desc.MiscFlags = 0;
		v_buf_desc.StructureByteStride = 0;
		v_buf_desc.Usage = D3D11_USAGE_DEFAULT;

		this->vertices[0].Position = Vector4(-1.0f, 1.0f, 1.0f, 1.0f);
		this->vertices[0].TexCoords = Vector2(0.0f, 0.0f);

		this->vertices[1].Position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		this->vertices[1].TexCoords = Vector2(1.0f, 0.0f);

		this->vertices[2].Position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[2].TexCoords = Vector2(0.0f, 1.0f);

		this->vertices[3].Position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		this->vertices[3].TexCoords = Vector2(1.0f, 0.0f);

		this->vertices[4].Position = Vector4(1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[4].TexCoords = Vector2(1.0f, 1.0f);

		this->vertices[5].Position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[5].TexCoords = Vector2(0.0f, 1.0f);

		D3D11_SUBRESOURCE_DATA subresource_data;
		ZeroMemory(&subresource_data, sizeof(D3D11_SUBRESOURCE_DATA));
		subresource_data.pSysMem = this->vertices;

		if (FAILED(res = this->renderer->GetDevice()->CreateBuffer(
			&v_buf_desc,
			&subresource_data,
			&this->quadVBuffer)))
		{
			log_str("CreateBuffer Failed: %s\n", get_err_str(res));
			return false;
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uav_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uav_desc.Texture2D.MipSlice = 0;

		if (FAILED(res = this->renderer->GetDevice()->CreateUnorderedAccessView(
			this->compute_output,
			&uav_desc,
			&this->output_uav)))
		{
			log_str("CreateUnorderedAccessView Failed: %s\n", get_err_str(res));
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC cs_output_srv_desc;
		cs_output_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		cs_output_srv_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		cs_output_srv_desc.Texture2D.MipLevels = 1;
		cs_output_srv_desc.Texture2D.MostDetailedMip = 0;

		if (FAILED(res = this->renderer->GetDevice()->CreateShaderResourceView(
			this->compute_output,
			&cs_output_srv_desc,
			&this->compute_output_view)))
		{
			log_str("CreateUnorderedAccessView failed: %s\n", get_err_str(res));
			return false;
		}

		D3D11_BUFFER_DESC c_buf_desc;
		c_buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		c_buf_desc.ByteWidth = sizeof(RayTracerParams);
		c_buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		c_buf_desc.MiscFlags = 0;
		c_buf_desc.StructureByteStride = 0;
		c_buf_desc.Usage = D3D11_USAGE_DYNAMIC;
		
		D3D11_SUBRESOURCE_DATA c_buf_sr_data;
		ZeroMemory(&c_buf_sr_data, sizeof(D3D11_SUBRESOURCE_DATA));
		c_buf_sr_data.pSysMem = &this->params;

		this->params.bgr_color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		this->params.epsilon = 200.0;
		this->params.sphere = Vector4(0.0f, 0.0f, -10.0f, 0.5f);
		this->params.sphere_material = Vector4(1.0f, 0.0f, 0.0f, 0.5f);
		this->params.view_transform = Matrix::Identity();

		if (FAILED(res = this->renderer->GetDevice()->CreateBuffer(
			&c_buf_desc,
			&c_buf_sr_data,
			&this->params_c_buffer
			)))
		{
			log_str("CreateBuffer Failed: %s\n", get_err_str(res));
			return false;
		}

		if (!this->createCS())
			return false;

		return true;
	}

	bool RayTracer::createCS(void)
	{
		HRESULT res;
		ID3DBlob *csCode = nullptr;
		ID3DBlob *error_code = nullptr;

		UINT flags1 = 0;

#if _DEBUG

		flags1 |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;

#endif

		if (FAILED(res = D3DCompileFromFile(
			GetShaderPath(L"ray_trace_compute.hlsl").data(), 
			nullptr, 
			nullptr, 
			"ray_trace_main", 
			"cs_5_0", 
			flags1, 
			0, 
			&csCode, 
			&error_code)))
		{
			SafeRelease<ID3DBlob>(&csCode);
			if (error_code)
			{
				OutputDebugStringA((LPCSTR)error_code->GetBufferPointer());
				SafeRelease<ID3DBlob>(&error_code);
			}

			return false;
		}

		if (FAILED(res = this->renderer->GetDevice()->CreateComputeShader(csCode->GetBufferPointer(), csCode->GetBufferSize(), nullptr, &this->ray_tracer_shader)))
		{
			OutputDebugString("\n create colored pixel shader failed \n");
			return false;
		}

		return true;
	}

	void RayTracer::Run()
	{
		UINT counts[1] = { 1 };
		
		ID3D11UnorderedAccessView *uavNull[1] = {nullptr};
		
		this->renderer->GetDeviceContext()->CSSetShader(this->ray_tracer_shader, nullptr, 0);
		this->renderer->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &this->output_uav, nullptr);
		this->renderer->GetDeviceContext()->CSSetConstantBuffers(0, 1, &this->params_c_buffer);
		
		this->renderer->GetDeviceContext()->Dispatch(32, 32, 1);

		this->renderer->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, uavNull, nullptr);
	}

	void RayTracer::Render()
	{
		this->renderer->BindShader(SHADER_TYPE_TEXTURE);
		this->renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &this->compute_output_view);
		
		ID3D11ShaderResourceView *nullSRV[1] = { nullptr };

		this->renderer->BindSampler(true);


		UINT strides[1] = { sizeof(TexturedVertex) };
		UINT offsets[1] = { 0 };
		ID3D11Buffer *null_v_buff[1] = { nullptr };

		this->renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &this->quadVBuffer, strides, offsets);

		this->renderer->GetDeviceContext()->Draw(6, 0);

		this->renderer->BindSampler(false);

		this->renderer->GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);
		this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, null_v_buff, strides, offsets);
		this->renderer->BindShader(SHADER_TYPE_NONE);
	}
}