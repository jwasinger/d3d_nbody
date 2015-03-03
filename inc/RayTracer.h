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
		Matrix view_transform;
		float epsilon;
		Vector3 padding1;
		Vector4 bgr_color;
		Vector4 sphere; //x,y,z-position w-radius
		Vector4 sphere_material; //x,y,z color and w reflectivity
	};

	HRESULT CreateStructuredBuffer(
		ID3D11Device *device,
		UINT element_size,
		UINT count,
		void *initial_data,
		ID3D11Buffer **out);
	
	class RayTracer
	{
	private:
		Renderer *renderer;
		ID3D11Texture2D *compute_output;
		ID3D11Buffer *quadVBuffer;
		ID3D11UnorderedAccessView *output_uav;
		ID3D11ShaderResourceView *compute_output_view;

		ID3D11ComputeShader *ray_tracer_shader;

		RayTracerParams params;
		ID3D11Buffer *params_c_buffer;
		TexturedVertex vertices[6];

		ID3D11Buffer *plane_buffer;
		ID3D11Buffer *material_buffer;
		ID3D11Buffer *sphere_buffer;

	public:
		RayTracer(Renderer *renderer);
		bool Init(void);
		void Run(void);
		void Render(void);

	private:
		bool createCS(void);
	};
}
#endif