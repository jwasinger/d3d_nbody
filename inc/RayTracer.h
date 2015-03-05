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

	struct Triangle
	{
		Vector3 v0, v1, v2;
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
		Vector3 v2;
		int material_index;
	};

	struct Ray
	{
		Vector3 pos;
		Vector3 dir;
		float len;
	};

	struct CollisionInfo
	{
		Ray outgoing;
		bool collided;
		bool error;
	};

	class RayTracer
	{
	private:
		Renderer *renderer = NULL;
		ID3D11Buffer *quadVBuffer = NULL;

		ID3D11ComputeShader *ray_tracer_shader = NULL;

		Matrix view;
		bool has_view = false;

		TexturedVertex vertices[6];
		ID3D11Texture2D *ray_tracer_output;
		ID3D11ShaderResourceView *ray_tracer_output_srv;

		Sphere spheres[16];
		Material materials[16];
		Plane planes[16];
		Light lights[16];
		Triangle triangles[16];

		Vector4 bgr_color;
		int num_spheres;
		int num_materials;
		int num_planes;
		int num_lights;

		UINT width, height;
		UINT iterations;
		float epsilon;

		Vector4 *raw_data;
		void write_pix(UINT x, UINT y, Vector4 val);
		void update_pix_buffer(void);
		Vector4 ray_trace(UINT x, UINT y);
		Vector2 get_ndc_coords(UINT x, UINT y);
		CollisionInfo ray_sphere_collision(Ray ray, Sphere sphere);
		bool compute_shadows(Vector3 position)
		{
			
		}

	public:
		RayTracer(UINT width, UINT height, Renderer *renderer);
		
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