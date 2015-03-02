#define ITERATIONS 100
#define SCREEN_WIDTH 360
#define SCREEN_HEIGHT 360

RWTexture2D<float4> tex;

/*
struct Triangle
{

};

struct Box
{

};

struct Sphere
{

};
*/

struct Ray
{
	float3 pos;
	float padding;
	float3 dir;
	float len;
};

struct CollisionInfo
{
	float4 col; //x,y,z normal, w- collided (1.0) or not (0.0)
};

cbuffer ray_tracer_params : register(b0)
{
	float4x4 view_transform;
	
	float epsilon;
	float3 padding1;

	float4 bgr_color;
	float4 sphere; //x,y,z-position w-radius
	float4 sphere_material; //x,y,z color and w reflectivity
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

CollisionInfo ray_sphere_collision(Ray ray)
{
	CollisionInfo out =  (CollisionInfo)0;
	
}

[numthreads(16, 16, 1)]
void ray_trace_main(uint2 tid : SV_DispatchThreadID)
{
	float step_dist = epsilon / ITERATIONS;
	float4 color = bgr_color;
	CollisionInfo ci;
	Ray ray;
	ray.pos = float3(get_ndc_coords(tid.xy), -1.0f);
	ray.dir = float3(0.0f, 0.0f, -1.0f);

	[unroll]
	for (int i = 0; i < ITERATIONS; i++)
	{
		ray.pos_dist += -step_dist;
		ray.pos_dist.w = step_dist;

		//advance the ray forward 'step_dist'
		ci = ray_sphere_collision(ray);
		if (ci.col.w == 1.0f)
		{
			color = sphere_material;
			color.w = 1.0f;
			break;
		}
	}

	
	AllMemoryBarrierWithGroupSync();
	tex[tid.xy] = color;

	//tex[tid.xy] = float4(cos(tid.x/512.0f), 0.0f, 0.0f, 1.0f);
	//cast rays out until they have gone epsilon distance
}