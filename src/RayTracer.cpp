#include "RayTracer.h"

namespace Core
{
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

		D3D11_TEXTURE2D_DESC ray_tracer_output_desc;
		ray_tracer_output_desc.ArraySize = 1;
		ray_tracer_output_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		ray_tracer_output_desc.CPUAccessFlags = 0;
		ray_tracer_output_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ray_tracer_output_desc.Width = 256;
		ray_tracer_output_desc.Height = 256;
		ray_tracer_output_desc.MipLevels = 1;
		ray_tracer_output_desc.MiscFlags = 0;
		ray_tracer_output_desc.SampleDesc.Count = 1;
		ray_tracer_output_desc.SampleDesc.Quality = 0;
		ray_tracer_output_desc.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED(res = this->renderer->GetDevice()->CreateTexture2D(
			&ray_tracer_output_desc,
			NULL,
			&this->ray_tracer_output)))
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

		D3D11_SHADER_RESOURCE_VIEW_DESC cs_output_srv_desc;
		cs_output_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		cs_output_srv_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		cs_output_srv_desc.Texture2D.MipLevels = 1;
		cs_output_srv_desc.Texture2D.MostDetailedMip = 0;

		if (FAILED(res = this->renderer->GetDevice()->CreateShaderResourceView(
			this->ray_tracer_output,
			&cs_output_srv_desc,
			&this->ray_tracer_output_view)))
		{
			log_str("CreateUnorderedAccessView failed: %s\n", get_err_str(res));
			return false;
		}

		return true;
	}

	void RayTracer::Run()
	{

	}

	void RayTracer::Render()
	{
		this->renderer->BindShader(SHADER_TYPE_TEXTURE);
		this->renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &this->ray_tracer_output_view);

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