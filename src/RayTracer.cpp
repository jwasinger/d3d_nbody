#include "RayTracer.h"

namespace Core
{
	RayTracer::RayTracer(UINT width, UINT height, Renderer *renderer)
	{
		this->width = width;
		this->height = height;
		this->renderer = renderer;
		this->num_lights = 0;
		this->num_materials = 0;
		this->num_spheres = 0;
		this->num_planes = 0;
		this->iterations = 100;
		this->epsilon = 100.0f;

		this->bgr_color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		ZeroMemory(spheres, sizeof(Sphere) * 16);
		ZeroMemory(materials, sizeof(Material) * 16);
		ZeroMemory(planes, sizeof(Plane) * 16);
		ZeroMemory(lights, sizeof(Light) * 16);
		ZeroMemory(vertices, sizeof(TexturedVertex) * 6);

		this->raw_data = new Vector4[width*height];
	}

	void RayTracer::SetViewTransform(Matrix m)
	{
		this->view = m;
		this->has_view = true;
	}

	void RayTracer::AddLight(Light l)
	{
		if (num_lights == 16)
		{
			log_str("trying to add too many lights to the ray tracer (max is 16). \n");
			return;
		}
		this->lights[num_lights] = l;
		num_lights++;
	}

	void RayTracer::AddSphere(Sphere s)
	{
		if (num_spheres == 16)
		{
			log_str("trying to add too many spheres to the ray tracer (max is 16).\n");
			return;
		}

		this->spheres[num_spheres] = s;
		num_spheres++;
	}

	int RayTracer::AddMaterial(Material m)
	{
		if (num_materials == 16)
		{
			log_str("trying to add too many materials (max is 16).\n");
			return -1;
		}

		this->materials[num_materials] = m;
		num_materials++;
		return num_materials - 1;
	}

	void RayTracer::AddPlane(Plane p)
	{
		if (num_planes == 16)
		{
			log_str("trying to add too many planes (max is 16).\n");
			return;
		}

		this->planes[num_planes] = p;
		num_planes++;
	}

	void RayTracer::write_pix(UINT x, UINT y, Vector4 val)
	{
		if (y > height - 1 || x > width - 1)
		{
			log_str("coordinates out of bounds...\n");
			return;
		}

		this->raw_data[width * y + x] = val;
	}

	void RayTracer::update_pix_buffer(void)
	{
		D3D11_MAPPED_SUBRESOURCE subresource;
		this->renderer->GetDeviceContext()->Map(this->ray_tracer_output, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
		
		memcpy(subresource.pData, this->raw_data, sizeof(Vector4)*width*height);

		this->renderer->GetDeviceContext()->Unmap(this->ray_tracer_output, 0);
	}

	bool RayTracer::Init(void)
	{
		HRESULT res;

		D3D11_TEXTURE2D_DESC ray_tracer_output_desc;
		ray_tracer_output_desc.ArraySize = 1;
		ray_tracer_output_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		ray_tracer_output_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ray_tracer_output_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		ray_tracer_output_desc.Width = this->width;
		ray_tracer_output_desc.Height = this->height;
		ray_tracer_output_desc.MipLevels = 1;
		ray_tracer_output_desc.MiscFlags = 0;
		ray_tracer_output_desc.SampleDesc.Count = 1;
		ray_tracer_output_desc.SampleDesc.Quality = 0;
		ray_tracer_output_desc.Usage = D3D11_USAGE_DYNAMIC;

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
		cs_output_srv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		cs_output_srv_desc.Texture2D.MipLevels = 1;
		cs_output_srv_desc.Texture2D.MostDetailedMip = 0;

		if (FAILED(res = this->renderer->GetDevice()->CreateShaderResourceView(
			this->ray_tracer_output,
			&cs_output_srv_desc,
			&this->ray_tracer_output_srv)))
		{
			log_str("CreateShaderResourceView failed: %s\n", get_err_str(res));
			return false;
		}

		//initialize all pixels to red
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				this->write_pix(j, i, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
			}
		}
		this->update_pix_buffer();

		return true;
	}

	Vector2 RayTracer::get_ndc_coords(UINT x, UINT y)
	{
		Vector2 output = Vector2(x, y);

		output.x = 2 * ((this->width - output.x) / this->width);
		output.y = 2 * ((this->height - output.y) / this->height);
		output.x = output.x - 1;
		output.y = output.y - 1;
		output.y = -output.y;

		return output;
	}

	Vector3 reflect(Vector3 v, Vector3 n)
	{
		Vector3 r = v - (2 * v.Dot( n))*n;
		r.Normalize();
		return r;
	}

	CollisionInfo RayTracer::ray_sphere_collision(Ray ray, Sphere sphere)
	{
		CollisionInfo col_info;
		col_info.collided = false;
		col_info.error = false;
		float step_dist = this->epsilon / iterations;

		float determinant = pow(ray.dir.Dot(ray.pos - sphere.pos), 2) -
			pow((ray.pos - sphere.pos).Length(), 2) +
			pow(sphere.radius, 2);

		if (determinant > 0.0f)
		{
			//calculate near/far distance from the origin of the ray to the points where it intersects the sphere
			float d_far = -ray.dir.Dot(ray.pos - sphere.pos) + sqrt(determinant);
			float d_near = -ray.dir.Dot(ray.pos - sphere.pos) - sqrt(determinant);

			if (d_near > 0.0 && d_near - ray.len < 0.0) // ray intersects sphere and is in range
			{
				col_info.collided = true;

				//calculate whether the ray is actually currently intersecting the sphere
				Vector3 normal = (ray.pos + ray.dir*d_near) - sphere.pos;
				normal.Normalize();

				col_info.outgoing.pos = ray.pos + (ray.dir)*(d_near);
				col_info.outgoing.dir = reflect(ray.dir, normal);
			}
			else if (d_near < 0.0f && d_far > 0.0f) // ray is inside sphere... this is BAD
			{
				col_info.error = true;
			}
		}

		return col_info;
	}

	bool compute_shadows(Vector3 position)
	{
		return false; // no light sources for now
	}

	Vector4 RayTracer::ray_trace(UINT x, UINT y)
	{
		Ray ray;
		Vector2 pos2D = this->get_ndc_coords(x, y);
		ray.pos = Vector3(pos2D.x, pos2D.y, 0.0f);
		ray.dir = Vector3(0.0f, 0.0f, -1.0f);
		ray.len = this->epsilon / this->iterations;
		Vector4 out_color = this->bgr_color;
		Vector3 out_color3;

		for (int i = 0; i < this->iterations; i++)
		{
			for (int j = 0; j < num_spheres; j++)
			{
				CollisionInfo ci = ray_sphere_collision(ray, spheres[j]);
				if (ci.collided)
				{
					ray.pos = ci.outgoing.pos;
					ray.dir = ci.outgoing.dir;

					if (ci.error)
						out_color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
					else
					{
						out_color3 = materials[spheres[j].material_index].ambient_color *
							materials[spheres[j].material_index].ambient_intensity * 
							materials[spheres[j].material_index].reflectivity;
						
						//if (compute_shadows(ray.pos))
						out_color += Vector4(out_color3.x, out_color3.y, out_color3.z, 1.0f);
					}
				}
			}

			ray.pos += ray.dir*(this->epsilon / this->iterations);
			ray.len = this->epsilon / this->iterations;
		}

		return out_color;
	}

	void RayTracer::Run()
	{
		for (UINT i = 0; i < this->height; i++)
		{
			for (UINT j = 0; j < this->width; j++)
			{
				Vector4 color = this->ray_trace(j, i);
				this->write_pix(j, i, color);
			}
		}
		this->update_pix_buffer();
	}

	void RayTracer::Render()
	{
		this->renderer->BindShader(SHADER_TYPE_TEXTURE);
		this->renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &this->ray_tracer_output_srv);

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