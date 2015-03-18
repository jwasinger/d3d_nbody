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
		this->epsilon = 10.0f;

		this->bgr_color = Vector4(0.3f, 0.0f, 0.0f, 1.0f);
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
		this->vertices[0].TexCoords = Vector2(0.0f, 1.0f);

		this->vertices[1].Position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		this->vertices[1].TexCoords = Vector2(1.0f, 1.0f);

		this->vertices[2].Position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[2].TexCoords = Vector2(0.0f, 0.0f);

		this->vertices[3].Position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		this->vertices[3].TexCoords = Vector2(1.0f, 1.0f);

		this->vertices[4].Position = Vector4(1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[4].TexCoords = Vector2(1.0f, 0.0f);

		this->vertices[5].Position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
		this->vertices[5].TexCoords = Vector2(0.0f, 0.0f);

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

		this->init_black_hole();

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
		output.y = output.y;
		output.y *= -1;
		output.x *= -1;
		return output;
	}

	void RayTracer::AddTriangleCC(Triangle t)
	{
		if (num_tris == 16)
		{
			log_str("trying to add too many planes (max is 16).\n");
			return;
		}

		this->triangles[num_tris] = t;
		num_tris++;
	}

	Vector3 reflect(Vector3 v, Vector3 n)
	{
		if (v == n)
			return v;

		Vector3 r = v - (2 * v.Dot( n))*n;
		r.Normalize();
		return r;
	}

	bool RayTracer::vec_sphere_intersection(Vector3 dir, Vector3 pos, Sphere sphere)
	{
		CollisionInfo col_info;
		float step_dist = this->epsilon / iterations;

		float determinant = pow(dir.Dot(pos - sphere.pos), 2) -
			pow((pos - sphere.pos).Length(), 2) +
			pow(sphere.radius, 2);

		if (determinant > 0.0f)
		{
			//calculate near/far distance from the origin of the ray to the points where it intersects the sphere
			float d_far = -dir.Dot(pos - sphere.pos) + sqrt(determinant);
			float d_near = -dir.Dot(pos - sphere.pos) - sqrt(determinant);

			if (d_near > 0.0) // ray intersects sphere and is in range
			{
				return true;
			}
		}

		return false;
	}

	bool RayTracer::ray_triangle_collision(Ray &ray, Triangle tri)
	{
		Vector3 ab = tri.b - tri.a;
		Vector3 ac = tri.c - tri.a;
		Vector3	qp = ray.dir * ray.len;

		Vector3 n = ab.Cross(ac);

		float d = qp.Dot(n);
		if (d <= 0.0f) //ray is parallel or points away from triangle
			return false;

		Vector3 ap = ray.pos - tri.a;
		float t = ap.Dot(n);
		if (t < 0.0f || t > d)
			return false;

		//compute barycentric coordinates
		Vector3 e = qp.Cross(ap);
		float v = ac.Dot(e);
		if (v < 0.0f || v > d)
			return false;

		float w = -ab.Dot(e);
		if (w < 0.0f || v + w > d)
			return false;

		float ood = 1.0f / d;
		t *= ood;
		v *= ood;
		w *= ood;
		float u = 1.0f - v - w;
		
		n *= -1;
		n.Normalize();

		CollisionInfo ci;
		ci.obj_material = this->materials[tri.material_index];
		ci.outgoing_dir = reflect(-ray.dir, n);

		//what is the position
		ci.position = ray.pos + t*ray.dir;
		ci.surface_normal = n;

		ray.pos = ci.position;
		ray.dir = ci.outgoing_dir;
		ray.collisions.push_back(ci);

		return true;
	}

	bool RayTracer::vec_triangle_intersection(Vector3 pos, Vector3 dir, Triangle tri)
	{
		Vector3 ab = tri.b - tri.a;
		Vector3 ac = tri.c - tri.a;
		Vector3	qp = dir * 1000.0f;

		Vector3 n = ab.Cross(ac);

		float d = qp.Dot(n);
		if (d <= 0.0f) //ray is parallel or points away from triangle
			return false;

		Vector3 ap = pos - tri.a;
		float t = ap.Dot(n);
		if (t < 0.0f || t > d)
			return false;

		//compute barycentric coordinates
		Vector3 e = qp.Cross(ap);
		float v = ac.Dot(e);
		if (v < 0.0f || v > d)
			return false;

		float w = -ab.Dot(e);
		if (w < 0.0f || v + w > d)
			return false;

		return true;
	}

	bool RayTracer::ray_sphere_collision(Ray &ray, Sphere sphere)
	{
		CollisionInfo col_info;
		float step_dist = this->epsilon / iterations;
		bool collided = false;

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
				collided = true;

				Vector3 normal = (ray.pos + ray.dir*d_near) - sphere.pos;
				normal.Normalize();
				ray.pos = ray.pos + (ray.dir)*(d_near);
				ray.dir = reflect(ray.dir, normal);

				CollisionInfo ci;
				ci.obj_material = this->materials[sphere.material_index];
				ci.position = ray.pos;
				ci.surface_normal = normal;
				ci.outgoing_dir = ray.dir;

				ray.collisions.push_back(ci);
				//calculate whether the ray is actually currently intersecting the sphere
				

			}
			else if (d_near < 0.0f && d_far > 0.0f) // ray is inside sphere... this is BAD
			{
				ray.error = true;
			}
		}

		return collided;
	}

	bool RayTracer::ray_bh_collision(const Ray &ray)
	{
		Vector3 ray_bh_vec = ray.pos - this->black_hole_pos;
		if (ray_bh_vec.Length() <= this->black_hole_sr)
			return true;
		
		ray_bh_vec = ray.pos + ray.dir*ray.len;
		ray_bh_vec = ray_bh_vec - this->black_hole_pos;
		if (ray_bh_vec.Length() <= this->black_hole_sr)
			return true;

		return false;
	}

	void RayTracer::ray_bh_interaction(Ray &ray)
	{
		//if the ray is intersecting the the plane of interaction:
		//	alter its course

		Vector3 ray_plane_vec = this->black_hole_pos - ray.pos;
		ray_plane_vec.z = abs(ray_plane_vec.z);

		if (ray_plane_vec.z < 0.0f)
			return;
		if (ray_plane_vec.z > ray.len)
			return;

		black_hole_bend_ray(ray);
	}

	Vector4 RayTracer::ray_trace(UINT x, UINT y)
	{
		Ray ray;
		Vector2 pos2D = this->get_ndc_coords(x, y);
		ray.pos = Vector3(pos2D.x, pos2D.y, 0.0f);
		ray.dir = Vector3(0.0f, 0.0f, -1.0f);
		ray.len = this->epsilon / this->iterations;
		ray.collisions = std::vector<CollisionInfo>(0);
		Vector4 out_color = this->bgr_color;
		Vector3 out_color3;

		if (x == 10 && y == 10)
		{
			int test = 5;
		}

		for (int i = 0; i < this->iterations; i++)
		{
			//run a timestep
			//calculate the deflection of the ray by the black hole
			if (ray_bh_collision(ray))
				return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			ray_bh_interaction(ray);

			bool collided = false;

			for (int j = 0; j < num_spheres; j++)
			{
				if (ray_sphere_collision(ray, spheres[j]))
				{
					if (ray.error)
						out_color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
					else
					{
						//out_color3 = materials[spheres[j].material_index].ambient_color *
						//	materials[spheres[j].material_index].ambient_intensity * 
						//	materials[spheres[j].material_index].reflectivity;
						
						//if (compute_shadows(ray.pos))
						//out_color += Vector4(out_color3.x, out_color3.y, out_color3.z, 1.0f);
					
					}

					break;
				}
			}

			for (int j = 0; j < num_tris; j++)
			{
				if (ray_triangle_collision(ray, triangles[j]))
				{
					//ray.pos = ci.outgoing.pos;
					//ray.dir = ci.outgoing.dir;
					collided = true;

					break;
				}
			}

			ray.pos += ray.dir*(this->epsilon / this->iterations);
			ray.len = this->epsilon / this->iterations;
		}
		
		if (ray.error)
			out_color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		else
			out_color = this->compute_total_color(ray.collisions);

		return out_color;
	}
	
	void RayTracer::init_black_hole(void)
	{
		this->black_hole_pos = Vector3(-0.015f, -0.015f, -3.0f);
		this->black_hole_sr = 0.1f;
	}

	//Vector3 RayTracer::calc_near_point(const Ray &ray)
	//{	
		//in the simple case as is here, the ray will be closest to the black hole 
	//	Vector3 np;
	//}


	/*
	void RayTracer::black_hole_bend_ray(Ray &ray)
	{
		//find the closest point (point of action) between ray and black hole

		Vector3 bh_ray = this->black_hole_pos - ray.pos;
		// TODO position-based optimization here
		//if (bh_ray.Length() )
		//
		float theta = 2.0 * (this->black_hole_sr / bh_ray.Length());
		bh_ray.Normalize();
		Vector3 tangent = bh_ray.Cross(ray.dir);
		tangent.Normalize();
		if (tangent == Vector3(0.0f, 0.0f, 0.0f))
		{
			return; //ray direction is directly in the direction of black hole.
		}

		Matrix rotation = Matrix::CreateFromAxisAngle(-tangent, theta);
		ray.dir = Vector3::TransformNormal(ray.dir, rotation);
		ray.dir.Normalize();
	}
	*/

	void RayTracer::black_hole_bend_ray(Ray &ray)
	{
		//find the closest point (point of action) between ray and black hole
		Vector3 closest_pt = Vector3(ray.pos.x, ray.pos.y, this->black_hole_pos.z);
		float r = (closest_pt - this->black_hole_pos).Length();
		float theta = 2.0 * (this->black_hole_sr / r);
		theta *= 0.07f;

		Vector3 bh_ray = this->black_hole_pos - ray.pos;
		bh_ray.Normalize();
		Vector3 tangent = bh_ray.Cross(ray.dir);
		tangent.Normalize();
		if (tangent == Vector3(0.0f, 0.0f, 0.0f))
		{
			return; //ray direction is directly in the direction of black hole.
		}

		Matrix rotation = Matrix::CreateFromAxisAngle(-tangent, theta);
		ray.dir = Vector3::TransformNormal(ray.dir, rotation);
		ray.dir.Normalize();
	}

	std::vector<Light> RayTracer::compute_lighting(Vector3 position)
	{
		std::vector<Light> lights(0);
		Vector3 dir;

		for (int i = 0; i < this->num_lights; i++)
		{
			if (this->lights[i].type == DIRECTIONAL_LIGHT)
			{
				dir = -this->lights[i].pos_direction;
				dir.Normalize();
			}
			else
			{
				dir = this->lights[i].pos_direction - position;
				dir.Normalize();
			}

			bool lit = true;

			for (int j = 0; j < this->num_spheres; j++)
			{
				if (vec_sphere_intersection(dir, position, this->spheres[j]))
					lit = false;
			}

			if (!lit)
				continue;

			for (int j = 0; j < this->num_tris; j++)
			{
				if (vec_triangle_intersection(dir, position, this->triangles[j]))
					lit = false;
			}
			
			if (lit)
				lights.push_back(this->lights[i]);
		}
		
		return lights;
	}
	
	Vector4 RayTracer::compute_collision_color(const CollisionInfo &info)
	{
		std::vector<Light> lights = this->compute_lighting(info.position);
		Vector3 light_color = Vector3(0.0f, 0.0f, 0.0f);
		float diffuse = 0.0f;
		float specular = 0.0f;
		Vector3 dir_to_view = this->view_pos - info.position;
		dir_to_view.Normalize();

		for (int i = 0; i < lights.size(); i++)
		{
			if (lights[i].type == DIRECTIONAL_LIGHT)
			{
				diffuse = info.surface_normal.Dot(-lights[i].pos_direction);
				if (diffuse <= 0.0f)
					diffuse = 0.0f;
				else
					diffuse *= lights[i].diffuse_intensity;

				
				specular = info.outgoing_dir.Dot(dir_to_view);

				if (specular <= 0.0f)
					specular = 0.0f;
				else
					specular = pow(specular, lights[i].spec_coefficient); // assume spec intensity == 1.0
			}
			else
			{
				Vector3 dir_to_light = lights[i].pos_direction - info.position;
				dir_to_light.Normalize();

				diffuse = info.surface_normal.Dot(dir_to_light);
				diffuse *= lights[i].diffuse_intensity;
				
				if (diffuse < 0.0f)
					diffuse = 0.0f;
				
				specular = info.outgoing_dir.Dot(dir_to_view);

				if (specular <= 0.0f)
					specular = 0.0f;
				else
					specular = pow(specular, lights[i].spec_coefficient); // assume spec intensity == 1.0
			}

			specular = 0.0f;
			light_color += Vector3(1.0f, 1.0f, 1.0f) * diffuse + 
						   Vector3(1.0f, 1.0f, 1.0f) * specular;
			specular = 0.0f;
			diffuse = 0.0f;
		}

		light_color += info.obj_material.ambient_color * info.obj_material.ambient_intensity;
		light_color.Clamp(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
		return Vector4(light_color.x, light_color.y, light_color.z, 1.0f);
	}

	Vector4 RayTracer::compute_total_color(std::vector<CollisionInfo> collisions)
	{
		if (collisions.size() == 0)
			return this->bgr_color;

		Vector4 light_color;	
		
		UINT last = collisions.size() - 1;
		if (collisions.size() == 1)
		{
			light_color = compute_collision_color(collisions[last]) * (1.0f - collisions[last].obj_material.reflectivity);
			return light_color + bgr_color*collisions[last].obj_material.reflectivity;
		}
		else
		{
			light_color = compute_collision_color(collisions[last]) * (1.0f - collisions[last].obj_material.reflectivity);
			collisions.pop_back();
			return light_color + collisions[last-1].obj_material.reflectivity * compute_total_color(collisions);
		}
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