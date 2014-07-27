#ifndef VERTEX_H
#define VERTEX_H

#include<DirectXMath.h>

namespace NBody
{
	//Point Sprite Vertex
	struct PSVertex
	{
	public:
		DirectX::XMFLOAT4 Position;
	};

	struct ColoredVertex
	{
	public:
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Color;
	};

	struct TexturedVertex
	{
	public:
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT2 TexCoords;
	};

	struct Particle
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Velocity;
	};
}

#endif