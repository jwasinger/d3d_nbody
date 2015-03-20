#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Renderer.h"
#include "Vertex.h"
#include "log.h"

#define DIRECTIONAL_LIGHT 0 
#define POINT_LIGHT		  1

namespace Core
{
	struct Light
	{
		int type;
		Vector3 pos_direction;
		float diffuse_intensity;
		float spec_coefficient;
	  //float spec_intensity;
	};

	struct Triangle
	{
		Vector3 a, b, c;
		UINT material_index;
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

	struct CollisionInfo
	{
		Vector3 position;
		Vector3 surface_normal;
		Vector3 outgoing_dir;
		Material obj_material;
	};

	struct Ray
	{
		Vector3 pos;
		Vector3 dir;
		float len;
		std::vector<CollisionInfo> collisions;
		bool error = false;
		bool absorbed = false;
	};

	class RayTracer
	{
	private:
		Renderer *renderer = NULL;
		ID3D11Buffer *quadVBuffer = NULL;

		Matrix view;
		bool has_view = false;
		Vector3 view_pos = Vector3(0.0f, 0.0f, 0.0f);

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
		int num_tris;

		UINT width, height;
		UINT iterations;
		float epsilon;

		//define positions of four 'jittered' rays.  These values are defined with respect
		//to the boundary of a pixel (origin lower lefthand corner) with range [0,0] -> [1,1]
		//Vector2 jitters[4]; 
		std::vector<Vector2> jitters;

		float focal_len;

		Vector4 *raw_data;
		void write_pix(UINT x, UINT y, Vector4 val);
		void update_pix_buffer(void);

		Vector4 trace_pixel(UINT x, UINT y);
		Vector4 ray_trace(const Vector3 &ray_pos, const Vector3 &ray_dir);
		Vector2 get_ndc_coords(float x, float y);
		
		void generate_circle_jitters(int num, float r);

		bool ray_sphere_collision(Ray &ray, Sphere sphere);
		bool vec_sphere_intersection(Vector3 dir, Vector3 pos, Sphere sphere);

		bool ray_triangle_collision(Ray &ray, Triangle tri);
		bool vec_triangle_intersection(Vector3 pos, Vector3 dir, Triangle tri);


		std::vector<Light> compute_lighting(Vector3 position);
		Vector4 compute_total_color(std::vector<CollisionInfo> collisions);
		Vector4 compute_collision_color(const CollisionInfo &info);
		
		void enable_view(bool enable);
		void set_view(Matrix m);

	public:
		RayTracer(UINT width, UINT height, Renderer *renderer);
		
		bool Init(void);
		void Run(void);
		void Render(void);

		void AddSphere(Sphere s);
		int AddMaterial(Material m);
		void AddPlane(Plane p);
		void AddLight(Light l);
		void AddTriangleCC(Triangle t);

		void SetViewTransform(Matrix m);
	};
}
#endif