#define ITERATIONS 100

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

#define MAX_SPHERES 16
#define MAX_TRIANGLES 16
#define MAX_LIGHTS 16
#define MAX_PLANES 16

#define LIGHT_DIRECTIONAL 0 
#define LIGHT_POINT 1

struct Light
{
	int type;
	int3 padding;
	float3 pos_direction;
};

struct Material
{
	float3 ambient_color;
	float ambient_intensity;
	float reflectivity;
	float3 padding;
};

struct Triangle
{
	float3 v0;
	float padding;
	float3 v1;
	float padding1;
	float3 v2;
	float padding2;
	int material_index;
};

struct Plane
{
	float3 v1;
	float padding;
	float3 v2;
	float padding1;
	int material_index;
};

struct Sphere
{
	float3 pos;
	float radius;
	int material_index;
};

struct Ray
{
	float3 pos;
	float padding;
	float3 dir;
	float len;
};

struct CollisionInfo
{
	Ray outgoing;
	bool collided;
	bool error;
};

RWTexture2D<float4> tex;
StructuredBuffer<Material> materials;
StructuredBuffer<Sphere> spheres;
StructuredBuffer<Plane> planes;
StructuredBuffer<Light> lights;

cbuffer ray_tracer_params : register(b0)
{
	float epsilon;
	float3 padding1;
	float4 bgr_color;
	int num_spheres;
	int3 padding2;
	int num_materials;
	int3 padding3;
	int num_planes;
	int3 padding4;
	int num_lights;
	int3 padding5;
};

float2 get_ndc_coords(uint2 tid)
{
	float2 output = (float2)tid;
	output.x = 2 * ((SCREEN_WIDTH - output.x) / SCREEN_WIDTH);
	output.y = 2 * ((SCREEN_HEIGHT - output.y) / SCREEN_HEIGHT);
	output.x = output.x - 1;
	output.y = output.y - 1;
	output.y = -output.y;

	return output;
}

float3 reflect(float3 v, float3 n)
{
	float3 r = v - (2 * dot(v, n))*n;
	return normalize(r);
}

//compute whether a location is completely occluded from light sources
bool compute_shadows(float3 position)
{
	return false; // no light sources for now
}

float3 compute_diffuse(Material m, float3 normal)
{
	
}

CollisionInfo ray_sphere_collision(Ray ray, Sphere sphere)
{
	CollisionInfo col_info =  (CollisionInfo)0;
	col_info.collided = false;
	col_info.error = false;
	float4 step_dist = epsilon / ITERATIONS;

	float determinant = pow(dot(ray.dir, ray.pos - sphere.pos), 2) - 
						pow(length(ray.pos - sphere.pos), 2) + 
						pow(sphere.radius, 2);

	if(determinant > 0.0f)
	{
		//calculate near/far distance from the origin of the ray to the points where it intersects the sphere
		float d_far = -dot(ray.dir, ray.pos - sphere.pos) + sqrt(determinant);
		float d_near = -dot(ray.dir, ray.pos - sphere.pos) - sqrt(determinant);

		if (d_near > 0.0 && d_near - ray.len < 0.0) // ray intersects sphere and is in range
		{
			col_info.collided = true;

			//calculate whether the ray is actually currently intersecting the sphere
			float3 normal = (ray.pos + ray.dir*d_near) - sphere.pos;
				normal = normalize(normal);

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

[numthreads(32, 32, 1)]
void ray_trace_main(uint2 tid : SV_DispatchThreadID)
{
	float step_dist = epsilon / ITERATIONS;
	CollisionInfo ci;
	Ray ray = (Ray)0;
	ray.pos = float3(get_ndc_coords(tid.xy), -1.0f);
	ray.dir = float3(0.0f, 0.0f, -1.0f);
	ray.len = step_dist;
	float4 out_color = bgr_color;

	[loop]
	for (int i = 0; i < ITERATIONS; i++)
	{	
		[loop]
		for (int j = 0; j < num_spheres; j++)
		{
			Sphere s = spheres[j];

			//advance the ray forward 'step_dist'
			ci = ray_sphere_collision(ray, s);
			if (ci.collided)
			{
				ray.pos = ci.outgoing.pos;
				ray.dir = ci.outgoing.dir;

				//compute shadows by tracing a ray to every light source
				if (!compute_shadows(ray.pos))
				{

				}

				if (ci.error)
					out_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
				else
					out_color += float4(materials[s.material_index].ambient_color * materials[s.material_index].ambient_intensity, 1.0f);

			}
			ray.pos += normalize(ray.dir)*step_dist;
			ray.len = step_dist;
		}

		//test planes
		//test triangles
	}
	
	AllMemoryBarrierWithGroupSync();
	
	tex[tid.xy] = saturate(out_color);

	//tex[tid.xy] = float4(cos(tid.x/512.0f), 0.0f, 0.0f, 1.0f);
}