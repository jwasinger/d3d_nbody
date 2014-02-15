SamplerState SampleType;
Texture2D Texture;

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

struct VS_INPUT
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

PS_INPUT VShader(VS_INPUT input)
{ 
	PS_INPUT output = (PS_INPUT)0;

	output.TexCoord = input.TexCoord;
	output.Position = input.Position;
	output.Position = mul(world, input.Position);
	output.Position = mul(view, output.Position);
	output.Position = mul(projection, output.Position);

	return output;
}

float4 PShader(PS_INPUT input) : SV_TARGET
{
	return Texture.Sample(SampleType, input.TexCoord);
}