#include "a1.h"

struct Pixel
{
	short a, r, g, b;
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

bool A1::Init(void)
{
	HRESULT res;
	D3D11_TEXTURE2D_DESC texture_desc;
	D3D11_SUBRESOURCE_DATA initial_data;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_desc;
	Vector4 verts[6];
	D3D11_SUBRESOURCE_DATA verts_data;
	Pixel default_color;

	short *rawData = (short *)malloc(this->image_width*this->image_height*sizeof(Pixel));
	default_color.r = 0;
	default_color.g = 0;
	default_color.b = 0;
	default_color.a = 255;

	texture_desc.ArraySize = 1;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = DXGI_CPU_ACCESS_READ_WRITE;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.Width = this->image_width;
	texture_desc.Height = this->image_height;
	texture_desc.MipLevels = 1;
	texture_desc.MiscFlags = 0;
	texture_desc.SampleDesc.Count = 1; 
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;

	initial_data.pSysMem = &this->raw_data;
	initial_data.SysMemPitch = 0;
	initial_data.SysMemSlicePitch = 0;

	SRV_desc.Texture2D.MipLevels = 1;
	SRV_desc.Texture2D.MostDetailedMip = 0;

	for (int i = 0; i < this->image_height; i++)
	{
		for (int j = 0; j < this->image_width; j++)
		{
			memcpy((void *)this->raw_data[i*this->image_width*sizeof(Pixel)+j*sizeof(Pixel)], &default_color, sizeof(Pixel));
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
	v_buffer_desc.ByteWidth = sizeof(Vector4)* 6;
	v_buffer_desc.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	v_buffer_desc.MiscFlags = 0;
	v_buffer_desc.StructureByteStride = 0;
	v_buffer_desc.Usage = D3D11_USAGE_DEFAULT;

	verts[0] = Vector4(0.0f, 0.0f, 0.2f, 1.0f);
	verts[1] = Vector4(1.0f, 1.0f, 0.2f, 1.0f);
	verts[2] = Vector4(0.0f, 1.0f, 0.2f, 1.0f);
	verts[3] = Vector4(1.0f, 0.0f, 0.2f, 1.0f);
	verts[4] = Vector4(1.0f, 1.0f, 0.2f, 1.0f);
	verts[5] = Vector4(0.0f, 1.0f, 0.2f, 1.0f);
	
	verts_data.pSysMem = &verts;
	verts_data.SysMemPitch = 0;
	verts_data.SysMemSlicePitch = 0;

	if (FAILED(res = this->renderer->GetDevice()->CreateBuffer(&v_buffer_desc, &verts_data, &this->quad_v_buffer)))
		return false;

	return true;
}

void A1::batch_pixel(XMINT2 position, short r, short g, short b)
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

	memcpy((void *)this->raw_data[position.y*this->image_width*sizeof(Pixel)+position.x*sizeof(Pixel)], &pix, sizeof(Pixel));
}

void A1::Render(void)
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	ID3D11Buffer *vBuffs = {this->quad_v_buffer};
	UINT *strides = {0};
	UINT *offsets = {0};

	//write data to the texture
	this->renderer->GetDeviceContext()->Map(this->texture, 0, D3D11_MAP_WRITE, 0, &mapped_subresource);
	memcpy(mapped_subresource.pData, this->raw_data, this->image_width*this->image_height*sizeof(Pixel));
	this->renderer->GetDeviceContext()->Unmap(this->texture, 0);
	
	this->renderer->SetTransform(TRANSFORM_WORLD, Matrix::Identity());
	this->renderer->SetTransform(TRANSFORM_VIEW, Matrix::Identity());
	
	Matrix projection = Matrix::CreateOrthographicOffCenter(0.0f, 1.0f, 1.0f, 0.0f, 0.1f, 1.0f);
	this->renderer->SetTransform(TRANSFORM_PROJECTION, projection);

	this->renderer->BindShader(SHADER_TYPE_TEXTURE);
	this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &vBuffs, strides, offsets);
	this->renderer->GetDeviceContext()->VSSetShaderResources()
	//TODO: Make drawing a textured 2D quad easier (add renderer->SetTexture())

}