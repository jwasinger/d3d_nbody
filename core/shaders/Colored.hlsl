cbuffer worldBuffer : register(b0)
{
	#pragma pack_matrix( column_major )
	float4x4 world;
};

cbuffer viewBuffer : register(b1)
{
	#pragma pack_matrix( column_major )
	float4x4 view;
};

cbuffer projectionBuffer : register(b2)
{
	#pragma pack_matrix( column_major )
	float4x4 projection;
};

cbuffer colorBuffer : register(b3)
{
	float4 color;	
};

struct VS_INPUT
{
	float4 Position : POSITION0;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
};

PS_INPUT VShader( VS_INPUT input )
{
	PS_INPUT output = (PS_INPUT)0;
	output.Position = input.Position;
	output.Position = mul(world, input.Position);
	output.Position = mul(view, output.Position);
	output.Position = mul(projection, output.Position);

	return output;
}

float4 PShader( PS_INPUT input ) : SV_TARGET
{
	return color;
}