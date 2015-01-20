#ifndef A1_H
#define A1_H

#include "Renderer.h"
#include "log.h"

using namespace NBody;

class A1
{
public:
	A1(Renderer *renderer, int width, int height);
	~A1(void);
	bool Init(void);
	void Render(void);

private:
	
	void batch_vertex(Vector2 v, Vector4 color);
	void batch_pixel(XMINT2 position, short r, short b, short g);
	void flush_verts(void); 

private:
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *texture_SRV;
	ID3D11Buffer *quad_v_buffer;

	short *raw_data;
	int image_width, image_height;
	Renderer *renderer;
};
#endif