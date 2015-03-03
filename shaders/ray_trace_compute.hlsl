#define ITERATIONS 10

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

#define MAX_SPHERES 16
#define MAX_TRIANGLES 16
#define MAX_LIGHTS 16
#define MAX_PLANES 16

struct Material;
struct Triangle;
struct Plane;
struct Sphere;

RWTexture2D<float4> tex;
StructuredBuffer<Material> materials;
StructuredBuffer<Sphere> spheres;
StructuredBuffer<Planes> planes;

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
};

cbuffer ray_tracer_params : register(b0)
{
	float4x4 view_transform;
	
	float epsilon;
	float3 padding1;

	float4 bgr_color;
	Sphere spheres[MAX_SPHERES];
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

CollisionInfo ray_sphere_collision(Ray ray)
{
	CollisionInfo col_info =  (CollisionInfo)0;
	col_info.collided = false;
	float4 step_dist = epsilon / ITERATIONS;

	float determinant = pow(dot(ray.dir, ray.pos - ((float3)sphere)), 2) - 
						pow(length(ray.pos - (float3)sphere), 2) + 
						pow(sphere.w, 2);

	if(determinant > 0.0f)
	{
		//calculate near/far distance from the origin of the ray to the points where it intersects the sphere
		float d_far = -dot(ray.dir, ray.pos - (float3)sphere) + sqrt(determinant);
		float d_near = -dot(ray.dir, ray.pos - (float3)sphere) - sqrt(determinant);
		
		if (d_near - ray.len > 0.0)
			return col_info;
		
		col_info.collided = true;

		//calculate whether the ray is actually currently intersecting the sphere
		float3 normal = (ray.pos + ray.dir*d_near) - (float3)sphere;
		normal = normalize(normal);
		col_info.outgoing.pos = ray.pos + d_near;
		col_info.outgoing.dir = reflect(ray.dir, normal);
	}

	return col_info;
}

[numthreads(16, 16, 1)]
void ray_trace_main(uint2 tid : SV_DispatchThreadID)
{
	float step_dist = epsilon / ITERATIONS;
	float4 color = bgr_color;
	CollisionInfo ci;
	Ray ray = (Ray)0;
	ray.pos = float3(get_ndc_coords(tid.xy), -1.0f);
	ray.dir = float3(0.0f, 0.0f, -1.0f);
	ray.len = step_dist;

	[unroll]
	for (int i = 0; i < ITERATIONS; i++)
	{
		//advance the ray forward 'step_dist'
		ci = ray_sphere_collision(ray);
		if (ci.collided)
		{
			color = float4(abs(ci.outgoing.dir.x), 
						   abs(ci.outgoing.dir.y), 
						   abs(ci.outgoing.dir.z), 
						   1.0f);//sphere_material;
			color.w = 1.0f;
			break;
		}

		ray.pos += -step_dist;
		ray.len = step_dist;
	}

	
	AllMemoryBarrierWithGroupSync();
	tex[tid.xy] = color;

	//tex[tid.xy] = float4(cos(tid.x/512.0f), 0.0f, 0.0f, 1.0f);
	//cast rays out until they have gone epsilon distance
}