#include "a1.h"

struct Pixel
{
	char r, g, b, a;
};

float short_to_float_color(short c)
{
	return (float)c / 255.0f;
}

Vector4 pixel_to_float(Pixel pix)
{
	float a = short_to_float_color(pix.a); 
	float r = short_to_float_color(pix.r);
	float g = short_to_float_color(pix.g);
	float b = short_to_float_color(pix.b);
	return Vector4(r, g, b, a);
}

A1::A1(Renderer *renderer, int width, int height)
{
	this->renderer = renderer;
	this->image_width = width;
	this->image_height = height;
}

bool A1::Init(void)
{
	HRESULT res;
	D3D11_TEXTURE2D_DESC texture_desc;
	D3D11_SUBRESOURCE_DATA initial_data;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_desc;
	TexturedVertex verts[6];
	D3D11_SUBRESOURCE_DATA verts_data;
	Pixel default_color;

	this->raw_data = (char *)malloc(this->image_width*this->image_height*sizeof(Pixel));
	default_color.r = 0;
	default_color.g = 0;
	default_color.b = 0;
	default_color.a = 255;

	texture_desc.ArraySize = 1;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.Width = this->image_width;
	texture_desc.Height = this->image_height;
	texture_desc.MipLevels = 1;
	texture_desc.MiscFlags = 0;
	texture_desc.SampleDesc.Count = 1; 
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DYNAMIC;

	initial_data.pSysMem = this->raw_data;
	initial_data.SysMemPitch = this->image_width * sizeof(Pixel);
	initial_data.SysMemSlicePitch = 0;

	SRV_desc.Texture2D.MipLevels = 1;
	SRV_desc.Texture2D.MostDetailedMip = 0;
	SRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < this->image_height; i++)
	{
		for (int j = 0; j < this->image_width; j++)
		{
			memcpy(&this->raw_data[i*this->image_width*sizeof(Pixel) + j*sizeof(Pixel)], &default_color, sizeof(Pixel));
		}
	}

	if (FAILED(res = this->renderer->GetDevice()->CreateTexture2D(&texture_desc, &initial_data, &this->texture)))
	{
		return false;
	}

	if (FAILED(res = this->renderer->GetDevice()->CreateShaderResourceView(this->texture, &SRV_desc, &this->texture_SRV)))
	{
		return false;
	}
	
	D3D11_BUFFER_DESC v_buffer_desc;
	v_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	v_buffer_desc.ByteWidth = sizeof(TexturedVertex)* 6;
	v_buffer_desc.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	v_buffer_desc.MiscFlags = 0;
	v_buffer_desc.StructureByteStride = 0;
	v_buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	verts[0] = { Vector4(-0.9f, 0.9f, 0.3f, 1.0f), Vector2(0.0f, 0.0f) };
	verts[1] = { Vector4(0.9f, 0.9f, 0.3f, 1.0f), Vector2(1.0f, 0.0f) };
	verts[2] = { Vector4(-0.9f, -0.9f, 0.3f, 1.0f), Vector2(0.0f, 1.0f) };
	verts[3] = { Vector4(0.9f, 0.9f, 0.3f, 1.0f), Vector2(1.0f, 0.0f) };
	verts[4] = { Vector4(0.9f, -0.9f, 0.3f, 1.0f), Vector2(1.0f, 1.0f) };
	verts[5] = { Vector4(-0.9f, -0.9f, 0.3f, 1.0f), Vector2(0.0f, 1.0f) };
	
	verts_data.pSysMem = &verts;
	verts_data.SysMemPitch = 0;
	verts_data.SysMemSlicePitch = 0;

	if (FAILED(res = this->renderer->GetDevice()->CreateBuffer(&v_buffer_desc, &verts_data, &this->quad_v_buffer)))
		return false;

	A1_ColoredVertex v0, v1, v2, v3, v4, v5;
	v0.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v0.Position = XMINT2(0, 0);

	v1.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v1.Position = XMINT2(255, 0);
	
	v2.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v2.Position = XMINT2(0, 255);

	v3.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v3.Position = XMINT2(255, 255);

	v4.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v4.Position = XMINT2(0, 255);

	v5.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	v5.Position = XMINT2(255, 0);

	this->batch_vertex(v0);
	this->batch_vertex(v1);
	this->batch_vertex(v2);
	this->batch_vertex(v3);
	this->batch_vertex(v4);
	this->batch_vertex(v5);
	this->flush_verts();

	//write data to the texture

	return true;
}

void A1::batch_pixel(XMINT2 position, char r, char g, char b)
{
	if (position.x >= this->image_width || position.y >= this->image_height || position.x < 0 || position.y < 0)
	{
		log_str("attempting to write to pixel outside image bounds\n");
		return;
	}

	Pixel pix;
	pix.r = r;
	pix.g = g;
	pix.b = b;
	pix.a = 255;

	memcpy((char *)this->raw_data + position.y*this->image_width*sizeof(Pixel)+position.x*sizeof(Pixel), &pix, sizeof(Pixel));
}

void A1::Render(void)
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	ID3D11Buffer *vBuffs = {this->quad_v_buffer};
	UINT strides[] = {sizeof(TexturedVertex)};
	UINT offsets[] = {0};
	ID3D11SamplerState *sampler_states = { this->renderer->GetSamplerState() };
	ID3D11ShaderResourceView *textureSRVs = { this->texture_SRV };

	HRESULT res = this->renderer->GetDeviceContext()->Map(this->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
	ZeroMemory(mapped_subresource.pData, this->image_width*this->image_height*sizeof(Pixel));
	memcpy(mapped_subresource.pData, this->raw_data, this->image_width*this->image_height*sizeof(Pixel));
	//mapped_subresource.RowPitch = this->image_width * sizeof(Pixel);
	//mapped_subresource.DepthPitch = this->image_height*this->image_width*sizeof(Pixel);
	this->renderer->GetDeviceContext()->Unmap(this->texture, 0);

	this->renderer->BindShader(SHADER_TYPE_TEXTURE);
	this->renderer->SetTransform(TRANSFORM_WORLD, Matrix::Identity());
	this->renderer->SetTransform(TRANSFORM_VIEW, Matrix::Identity());
	this->renderer->SetTransform(TRANSFORM_PROJECTION, Matrix::Identity());
	
	//Matrix projection = Matrix::CreateOrthographicOffCenter(0.0f, 1.0f, 1.0f, 0.0f, 0.1f, 1.0f);
	//this->renderer->SetTransform(TRANSFORM_PROJECTION, projection);

	this->renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &vBuffs, strides, offsets);
	
	this->renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &textureSRVs);
	//this->renderer->GetDeviceContext()->PSSetSamplers(0, 1, &sampler_states);
	//this->renderer->SetDepthStencilEnabled(false);

	this->renderer->SetCullMode(D3D11_CULL_NONE);

	//TODO: Make drawing a textured 2D quad easier (add renderer->SetTexture())
	this->renderer->GetDeviceContext()->Draw(6, 0);
}

void A1::batch_vertex(A1_ColoredVertex vert)
{
	this->vertices.push_back(vert);
}

//use Cramer's Rule to get barycentric coordinates of a triangle in clockwise winding order
void __barycentric(XMINT2 P, XMINT2 A, XMINT2 B, XMINT2 C, float &u, float &v, float &w)
{
	Vector2 v0 = Vector2(B.x - A.x, B.y - A.y);
	Vector2 v1 = Vector2(C.x - A.x, C.y - A.y);
	Vector2 v2 = Vector2(P.x - A.x, P.y - A.y);

	float d00 = v0.Dot(v0);
	float d01 = v0.Dot(v1);
	float d11 = v1.Dot(v1);
	float d20 = v2.Dot(v0);
	float d21 = v2.Dot(v1);
	
	v = (d11*d20 - d01*d21) / (d00*d11 - d01*d01);
	w = (d00*d21 - d01*d20) / (d00*d11 - d01*d01);
	u = 1.0f - v - w;
}

bool A1::test_hit(XMINT2 pos, A1_ColoredVertex v1, A1_ColoredVertex v2, A1_ColoredVertex v3)
{
	float u, v, w;
	__barycentric(pos, v1.Position, v2.Position, v3.Position, u, v, w);

	if (v >= 0.0f && w >= 0.0f && u >= 0.0f)
		return true;
	else
		return false;
}

void A1::flush_verts(void)
{
	if (this->vertices.size() % 3 != 0)
	{
		log_str("Invalid number of vertices.  Results are undefined...\n");
		return;
	}

	for (int i = 0; i < this->vertices.size(); i += 3)
	{
		for (int j = 0; j < this->image_height; j++)
		{
			for (int k = 0; k < this->image_width; k++)
			{
				if (test_hit(XMINT2(k, j), vertices[i], vertices[i + 1], vertices[i + 2]))
				{
					this->batch_pixel(XMINT2(k, j), 255, 0, 0);
				}

			}
		}
	}
}