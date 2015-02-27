#define BLOCK_SIZE_X 128 

static float softeningSquared = 0.0012500000f*0.0012500000f;

struct PosVel
{
	float4 Pos;
	float4 Vel;
};

cbuffer g_param : register(b0)
{
	uint4 param;//X- Number of particles
				//
	
	uint4 param_f;//X- delta time (milliseconds)
}

StructuredBuffer<PosVel> posVelBufferOld;
RWStructuredBuffer<PosVel> posVelBufferNew;

void bodyBodyInteraction(inout float3 accel_i, float4 body_j, float4 body_i, float mass, int particles);

groupshared PosVel posVelCache[BLOCK_SIZE_X];

[numthreads(BLOCK_SIZE_X, 1, 1)]
void main( uint3 threadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
	PosVel outputPosVel = posVelBufferOld[threadID.x];
	float3 accel = float3(0.0f,0.0f,0.0f);

	[loop]
	for(uint i = 0; i < param[0]/ BLOCK_SIZE_X; i += BLOCK_SIZE_X)
	{
		posVelCache[groupThreadID.x] = posVelBufferOld[i * BLOCK_SIZE_X + groupThreadID.x];

		GroupMemoryBarrierWithGroupSync();

		[unroll]
		for(int j = 0; j < BLOCK_SIZE_X; j++)
		{
			bodyBodyInteraction(accel, outputPosVel.Pos, posVelCache[j].Pos, 1.0f, 1);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	outputPosVel.Vel.xyz = accel.xyz * param_f.x;
	outputPosVel.Pos.xyz = outputPosVel.Vel.xyz * param_f.x;

	return;
}

void bodyBodyInteraction(inout float3 accel_i, float4 body_j, float4 body_i, float mass, int particles ) 
{
    float3 radius = body_j.xyz - body_i.xyz;

    float distSqr = dot(radius, radius);
    distSqr += softeningSquared;

    float invDist = 1.0f / sqrt(distSqr);
	float invDistCube =  invDist * invDist * invDist;
    
    float s = mass * invDistCube * particles;

    accel_i += radius * s;    
}