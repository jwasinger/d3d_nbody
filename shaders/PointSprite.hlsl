Texture2D particleTexture;
SamplerState Sampler;

struct PosVelo 
{
	float4 Position;
	float4 Velocity;
};

StructuredBuffer<PosVelo> posVelBuff;

cbuffer worldBuffer : register(b0)
{
	float4x4 world;
};

cbuffer projectionBuffer : register(b1)
{
	float4x4 projection;
};

cbuffer viewBuffer : register(b2)
{
	float4x4 view;
};

cbuffer paramsBuffer : register(b3)
{
	float particleRad;
	float4x4 invView;
	float padding[3];
};

struct VS_INPUT
{
	uint Id : SV_VERTEXID;
};

struct VS_OUTPUT
{
	float4 Position : POSITION;
};

struct GS_OUTPUT 
{
	float4 Position : SV_POSITION;
	float2 TexCoords : TEXCOORD;
};

VS_OUTPUT VShader( VS_INPUT input )
{
	VS_OUTPUT output;
	output.Position = posVelBuff[input.Id].Position;

	return output;
}

cbuffer Constants
{
	static float4 positions[4] =
	{
		float4(-1.0f, 1.0f, 0.0f, 0.0f),
		float4(-1.0f, -1.0f, 0.0f, 0.0f),
		float4(1.0f, -1.0f, 0.0f, 0.0f),
		float4(1.0f, 1.0f, 0.0f, 0.0f),
	};

	static float2 texCoords[4] = 
	{
		float2(0.0f, 0.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f),
	};
}
	
[maxvertexcount(4)]
void GShader(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> OutputStream)
{
	GS_OUTPUT output;

	for( int i = 0;  i < 4; i++)
	{
		//transform input point back into view space
		output = (GS_OUTPUT)0;
		/*output.Position = mul(input[0].Position, (float3x3)invView) + (positions[i]*particleRad);
		output.Position = mul(output.Position, (float3x3)view);
		output.Position = mul(output.Position, (float3x3)projection);*/
		
		output.Position = mul(input[0].Position, invView) + (positions[i]*particleRad);
		output.Position = mul(output.Position, view);
		output.Position = mul(output.Position, projection);

		output.TexCoords = texCoords[i];

		OutputStream.Append(output);
	}

	OutputStream.RestartStrip();
}

float4 PShader(GS_OUTPUT input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}