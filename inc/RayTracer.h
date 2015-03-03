#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Renderer.h"
#include "Vertex.h"
#include "log.h"

#define CS_OUTPUT_WIDTH 512
#define CS_OUTPUT_HEIGHT 512

namespace Core
{
	struct Material
	{
		Vector3 ambient_color;
		float ambient_intensity;
		float reflectivity;
		Vector3 padding;
	};

	struct Sphere
	{
		Vector3 pos;
		float radius;
		int material_index;
	};

	struct Plane
	{
		Vector3 v1;
		float padding;
		Vector3 v2;
		float padding1;
		int material_index;
	};

	struct RayTracerParams
	{
		float epsilon;
		Vector3 padding1;
		Vector4 bgr_color;
	};

	HRESULT CreateStructuredBuffer(
		ID3D11Device *device,
		UINT element_size,
		UINT count,
		void *initial_data,
		ID3D11Buffer **out);
	
	HRESULT CreateStructuredBufferSRV(
		ID3D11Device *device,
		UINT element_size,
		UINT element_count,
		ID3D11Buffer *resource,
		ID3D11ShaderResourceView **out_srv);

	class RayTracer
	{
	private:
		Renderer *renderer = NULL;
		ID3D11Texture2D *compute_output = NULL;
		ID3D11Buffer *quadVBuffer = NULL;
		ID3D11UnorderedAccessView *output_uav = NULL;
		ID3D11ShaderResourceView *compute_output_view = NULL;

		ID3D11ComputeShader *ray_tracer_shader = NULL;

		Matrix view;
		bool has_view = false;

		RayTracerParams params;
		ID3D11Buffer *params_c_buffer = NULL;
		TexturedVertex vertices[6];

		int num_spheres = 0;
		int num_materials = 0;
		int num_planes = 0;

		Sphere spheres[16];
		Material materials[16];
		Plane planes[16];

		ID3D11Buffer *plane_buffer = NULL;
		ID3D11Buffer *material_buffer = NULL;
		ID3D11Buffer *sphere_buffer = NULL;
		
		ID3D11ShaderResourceView *plane_buf_srv = NULL;
		ID3D11ShaderResourceView *material_buf_srv = NULL;
		ID3D11ShaderResourceView *sphere_buf_srv = NULL;

	public:
		RayTracer(Renderer *renderer);
		bool Init(void);
		void Run(void);
		void Render(void);

		void AddSphere(Sphere s);
		int AddMaterial(Material m);
		void AddPlane(Plane p);
		
		void SetViewTransform(Matrix m);
	private:
		bool createCS(void);
		void updateBuffers(void);
	};
}
#endif