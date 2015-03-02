#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Renderer.h"
#include "Vertex.h"
#include "log.h"

namespace Core
{
	struct RayTracerParams
	{
		Matrix view_transform;
		float epsilon;
		Vector3 padding1;
		Vector4 bgr_color;
		Vector4 sphere; //x,y,z-position w-radius
		Vector4 sphere_material; //x,y,z color and w reflectivity
	};

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