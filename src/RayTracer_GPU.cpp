#include "RayTracer.h"

namespace Core
{
	HRESULT CreateStructuredBuffer(
		ID3D11Device *device,
		UINT element_size,
		UINT count,
		void *initial_data,
		ID3D11Buffer **out)
	{
		*out = NULL;
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.BindFlags =  D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = element_size * count;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = element_size;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		
		if (initial_data)
		{
			D3D11_SUBRESOURCE_DATA subresource_data;
			subresource_data.pSysMem = &initial_data;
			return device->CreateBuffer(&desc, &subresource_data, out);
		}
		else
		{
			return device->CreateBuffer(&desc, NULL, out);
		}
	}

	HRESULT CreateStructuredBufferSRV(
		ID3D11Device *device,
		UINT element_size,
		UINT element_count,
		ID3D11Buffer *resource,
		ID3D11ShaderResourceView **out_srv)
	{
		*out_srv = NULL;

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.ElementOffset = 0;
		desc.Buffer.ElementWidth = element_size;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = element_count;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		
		return device->CreateShaderResourceView(resource, &desc, out_srv);
	}

	RayTracer::RayTracer(Renderer *renderer)
	{
		this->renderer = renderer;
		ZeroMemory(spheres, sizeof(Sphere) * 16);
		ZeroMemory(materials, sizeof(Material) * 16);
		ZeroMemory(planes, sizeof(Plane) * 16);
		ZeroMemory(lights, sizeof(Light) * 16);
		ZeroMemory(vertices, sizeof(TexturedVertex) * 6);
	}

	void RayTracer::SetViewTransform(Matrix m)
	{
		this->view = m;
		this->has_view = true;
	}

	void RayTracer::AddLight(Light l)
	{
		if (params.num_lights == 16)
		{
			log_str("trying to add too many lights to the ray tracer (max is 16). \n");
			return;
		}
		this->lights[params.num_lights] = l;
		params.num_lights++;
	}

	void RayTracer::AddSphere(Sphere s)
	{
		if (params.num_spheres == 16)
		{
			log_str("trying to add too many spheres to the ray tracer (max is 16).\n");
			return;
		}
		
		this->spheres[params.num_spheres] = s;
		params.num_spheres++;
	}

	int RayTracer::AddMaterial(Material m)
	{
		if (params.num_materials == 16)
		{
			log_str("trying to add too many materials (max is 16).\n");
			return -1;
		}

		this->materials[params.num_materials] = m;
		params.num_materials++;
		return params.num_materials - 1;
	}

	void RayTracer::AddPlane(Plane p)
	{
		if (params.num_planes == 16)
		{
			log_str("trying to add too many planes (max is 16).\n");
			return;
		}

		this->planes[params.num_planes] = p;
		params.num_planes++;
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
		this->params.epsilon = 100.0;

		if (FAILED(res = this->renderer->GetDevice()->CreateBuffer(
			&c_buf_desc,
			&c_buf_sr_data,
			&this->params_c_buffer
			)))
		{
			log_str("CreateBuffer Failed: %s\n", get_err_str(res));
			return false;
		}

		//create structured buffers --------------------------------------------------

		if (FAILED(res = CreateStructuredBuffer(
			this->renderer->GetDevice(),
			sizeof(Plane),
			16,
			NULL,
			&this->plane_structured_buffer)))
		{
			log_str("Failed to create plane buffer: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBuffer(
			this->renderer->GetDevice(),
			sizeof(Sphere),
			16,
			NULL,
			&this->sphere_structured_buffer)))
		{
			log_str("Failed to create sphere buffer: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBuffer(
			this->renderer->GetDevice(),
			sizeof(Material),
			16,
			NULL,
			&this->material_structured_buffer)))
		{
			log_str("Failed to create material buffer: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBuffer(
			this->renderer->GetDevice(),
			sizeof(Light),
			16,
			NULL,
			&this->light_structured_buffer)))
		{
			log_str("Failed to create material buffer: %s", get_err_str(res));
			return false;
		}

		//create structured buffer SRVs -----------------------------------------------

		if (FAILED(res = CreateStructuredBufferSRV(
			this->renderer->GetDevice(),
			sizeof(Plane),
			16,
			this->plane_structured_buffer,
			&this->plane_buf_srv)))
		{
			log_str("failed to create plane buffer SRV: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBufferSRV(
			this->renderer->GetDevice(),
			sizeof(Sphere),
			16,
			this->sphere_structured_buffer,
			&this->sphere_buf_srv)))
		{
			log_str("failed to create sphere buffer SRV: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBufferSRV(
			this->renderer->GetDevice(),
			sizeof(Material),
			16,
			this->material_structured_buffer,
			&this->material_buf_srv)))
		{
			log_str("failed to create material buffer SRV: %s", get_err_str(res));
			return false;
		}

		if (FAILED(res = CreateStructuredBufferSRV(
			this->renderer->GetDevice(),
			sizeof(Light),
			16,
			this->light_structured_buffer,
			&this->light_buf_srv)))
		{
			log_str("failed to create material buffer SRV: %s", get_err_str(res));
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

	void RayTracer::updateBuffers(void)
	{
		//update structured buffers
		D3D11_MAPPED_SUBRESOURCE subresource;

		this->renderer->GetDeviceContext()->Map(this->sphere_structured_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		memcpy(subresource.pData, this->spheres, sizeof(Sphere) * 16);
		this->renderer->GetDeviceContext()->Unmap(this->sphere_structured_buffer, 0);

		this->renderer->GetDeviceContext()->Map(this->material_structured_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		memcpy(subresource.pData, this->materials, sizeof(Material) * 16);
		this->renderer->GetDeviceContext()->Unmap(this->material_structured_buffer, 0);

		this->renderer->GetDeviceContext()->Map(this->plane_structured_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		memcpy(subresource.pData, this->planes, sizeof(Plane) * 16);
		this->renderer->GetDeviceContext()->Unmap(this->plane_structured_buffer, 0);
	
		this->renderer->GetDeviceContext()->Map(this->light_structured_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		memcpy(subresource.pData, this->planes, sizeof(Light) * 16);
		this->renderer->GetDeviceContext()->Unmap(this->light_structured_buffer, 0);

		//update constant buffers
		this->renderer->GetDeviceContext()->Map(this->params_c_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		memcpy(subresource.pData, &this->params, sizeof(RayTracerParams));
		this->renderer->GetDeviceContext()->Unmap(this->params_c_buffer, 0);
	}

	void RayTracer::Run()
	{
		//TODO: transform all objects in the buffers from world space to camera space
		this->updateBuffers();
		
		UINT counts[1] = { 1 };
		
		ID3D11UnorderedAccessView *uavNull[1] = {nullptr};
		ID3D11ShaderResourceView *srvNull[1] = {nullptr};

		this->renderer->GetDeviceContext()->CSSetShader(this->ray_tracer_shader, nullptr, 0);
		this->renderer->GetDeviceContext()->CSSetShaderResources(0, 1, &this->material_buf_srv);
		this->renderer->GetDeviceContext()->CSSetShaderResources(1, 1, &this->sphere_buf_srv);
		this->renderer->GetDeviceContext()->CSSetShaderResources(2, 1, &this->plane_buf_srv);
		this->renderer->GetDeviceContext()->CSSetShaderResources(3, 1, &this->light_buf_srv);

		this->renderer->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, &this->output_uav, nullptr);
		this->renderer->GetDeviceContext()->CSSetConstantBuffers(0, 1, &this->params_c_buffer);
		
		this->renderer->GetDeviceContext()->Dispatch(32, 32, 1);

		//todo unset resources

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