#ifndef A1_H
#define A1_H

#include "Renderer.h"
#include "log.h"
#include "Vertex.h"
#include <vector>

using namespace Core;

struct A1_ColoredVertex
{
	XMINT2 Position;
	Vector4 Color;
};

class A1
{
public:
	A1(Renderer *renderer, int width, int height);
	~A1(void);
	bool Init(void);
	void Render(void);

private:
	void batch_vertex(A1_ColoredVertex vert);
	void batch_pixel(XMINT2 position, char r, char b, char g);
	void flush_verts(void); 
	bool test_hit(XMINT2, A1_ColoredVertex v1, A1_ColoredVertex v2, A1_ColoredVertex v3);

private:
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *texture_SRV;
	ID3D11Buffer *quad_v_buffer;
	std::vector<A1_ColoredVertex> vertices;

	char *raw_data;
	int image_width, image_height;
	Renderer *renderer;
};
#endif