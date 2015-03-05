#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Renderer.h"
#include "Vertex.h"
#include "log.h"

namespace Core
{
	struct Light
	{
		int type;
		Vector3 pos_direction;
	};

	struct Material
	{
		Vector3 ambient_color;
		float ambient_intensity;
		float reflectivity;
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
		Vector4 bgr_color;
		int num_spheres;
		int num_materials;
		int num_planes;
		int num_lights;
	};

	class RayTracer
	{
	private:
		Renderer *renderer = NULL;
		ID3D11Buffer *quadVBuffer = NULL;

		ID3D11ComputeShader *ray_tracer_shader = NULL;

		Matrix view;
		bool has_view = false;

		RayTracerParams params;
		TexturedVertex vertices[6];
		ID3D11Texture2D *ray_tracer_output;

		Sphere spheres[16];
		Material materials[16];
		Plane planes[16];
		Light lights[16];

	public:
		RayTracer(Renderer *renderer);
		bool Init(void);
		void Run(void);
		void Render(void);

		void AddSphere(Sphere s);
		int AddMaterial(Material m);
		void AddPlane(Plane p);
		void AddLight(Light l);

		void SetViewTransform(Matrix m);
	};
}
#endif