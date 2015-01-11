#include "ProcSphere.h"
#include "log.h"

namespace NBody
{
	ProcSphere::ProcSphere(void) {}

	bool ProcSphere::Init(Renderer *renderer)
	{
		if (!renderer)
			return false;

		this->renderer = renderer;
	}

	ProcSphere::~ProcSphere()
	{
	}

	Vector4 __find_middle(Vector4 &v1, Vector4 &v2)
	{
		return Vector4((v1.x + v2.x) / 2.0f, (v1.y + v2.y) / 2.0f, (v1.z + v2.z) / 2.0f, 1.0f);
	}

	void __push_back_arr(std::vector<Vector4> &vec, Vector4 *arr, int size)
	{
		if (!arr || size == 0)
			return;

		for (int i = 0; i < size; i++)
		{
			vec.push_back(arr[i]);
		}
	}

	void ProcSphere::__adjust_vert(Vector4 &v)
	{
		//calculate a unit vector in the direction origin->v, and replace v with that value

		Vector3 unit = Vector3(v.x,v.y,v.z) - Vector3(0.0f, 0.0f, 0.0f);
		unit.Normalize();
		v = Vector4(unit.x, unit.y, unit.z, 1.0f);
	}

	/*
		A recursive function to generate a sphere recursively from a tetrahedron.
		Each triangle is broken down into a serpinski triangle and molded around
		the center of the sphere.
		*/
	void ProcSphere::__generate_faces(Vector4 verts[3], std::vector<Vector4> &output, int permutations)
	{
		/*
			Visual represenation:
			0						0
			***					   ***
			*****			-->		20*****01
			*******				 *******
			2*******1				2**12***1

			where:

			^							^
			/ \						   /0\
			/	  \				-->		  -----
			/	   \				     /\ 3 /\
			/		\					/ 1\ /2 \
			---------				   -----------
			*/
		std::vector<Vector4> output_0;
		std::vector<Vector4> output_1;
		std::vector<Vector4> output_2;
		std::vector<Vector4> output_3;

		Vector4 v01 = __find_middle(verts[0], verts[1]);
		Vector4 v12 = __find_middle(verts[1], verts[2]);
		Vector4 v20 = __find_middle(verts[2], verts[0]);

		__adjust_vert(verts[0]);
		__adjust_vert(verts[1]);
		__adjust_vert(verts[2]);
		__adjust_vert(v01);
		__adjust_vert(v12);
		__adjust_vert(v20);

		if (permutations == 0)
		{

			Vector4 output_arr[] =
			{
				verts[0], v01, v20,
				v01, verts[1], v12,
				v20, v12, verts[2],
				v20, v01, v12,
			};

			__push_back_arr(output, output_arr, 12);
			return;
		}
		else
		{
			Vector4 input_0[3] = { verts[0], v01, v20 };
			Vector4 input_1[3] = { v01, verts[1], v12 };
			Vector4 input_2[3] = { v20, v12, verts[2] };
			Vector4 input_3[3] = { v01, v12, v20 };

			__generate_faces(input_0, output_0, permutations - 1);
			__generate_faces(input_1, output_1, permutations - 1);
			__generate_faces(input_2, output_2, permutations - 1);
			__generate_faces(input_3, output_3, permutations - 1);

			__push_back_arr(output, output_0.data(), output_0.size());
			__push_back_arr(output, output_1.data(), output_1.size());
			__push_back_arr(output, output_2.data(), output_2.size());
			__push_back_arr(output, output_3.data(), output_3.size());
		}

	}

	bool ProcSphere::Generate(int permutations)
	{
		std::vector<Vector4> output_left;
		std::vector<Vector4> output_bottom;
		std::vector<Vector4> output_right;
		std::vector<Vector4> output_front;
		std::vector<Vector4> output_all;
		HRESULT res;

		Vector4 tet[] =
		{
			//left
			Vector4(0, 1, 0, 1),
			Vector4(-1, -1, -1, 1),
			Vector4(0, -1, 1, 1),

			//bottom
			Vector4(-1, -1, -1, 1),
			Vector4(1, -1, -1, 1),
			Vector4(0, -1, 1, 1),

			//right
			Vector4(1, -1, -1, 1),
			Vector4(0, 1, 0, 1),
			Vector4(1, -1, 1, 1),

			//front
			Vector4(0, 1, 0, 1),
			Vector4(1, -1, -1, 1),
			Vector4(-1, -1, -1, 1),
		};

		this->__generate_faces(tet, output_left, permutations);
		this->__generate_faces(tet, output_bottom, permutations);
		this->__generate_faces(tet, output_right, permutations);
		this->__generate_faces(tet, output_front, permutations);

		//compile the vertices into one array 
		__push_back_arr(output_all, output_left.data(), output_left.size());
		__push_back_arr(output_all, output_bottom.data(), output_bottom.size());
		__push_back_arr(output_all, output_right.data(), output_right.size());
		__push_back_arr(output_all, output_front.data(), output_front.size());

		this->num_tris = output_all.size() / 3;

		D3D11_BUFFER_DESC v_desc;
		v_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		v_desc.ByteWidth = output_all.size() * sizeof(Vector4);
		v_desc.CPUAccessFlags = 0;
		v_desc.MiscFlags = 0;
		v_desc.StructureByteStride = 0;
		v_desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA initial_data;
		initial_data.SysMemPitch = 0;
		initial_data.SysMemSlicePitch = 0;
		initial_data.pSysMem = output_all.data();

		res = this->renderer->GetDevice()->CreateBuffer(
			&v_desc,
			&initial_data,
			&this->vBuf);

		if (FAILED(res))
		{
			log_str("CreateBufer failed: %s\n", get_err_str(res));
			return false;
		}

		this->iBuf = NULL;
		return true;
	}

	void ProcSphere::Render(void)
	{
		this->renderer->GetDeviceContext()->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		UINT stride[] = { sizeof(Vector4) };
		UINT offset[] = { 0 };

		this->renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &this->vBuf, stride, offset);

		//this->renderer->SetTransform(TRANSFORM_WORLD, Matrix::CreateTranslation(Vector3(1.0f, 1.0f, -10.0f)));
		this->renderer->SetTransform(TRANSFORM_WORLD, Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 0.0f)));
		this->renderer->SetTransform(TRANSFORM_PROJECTION, Matrix::CreatePerspectiveFieldOfView(
			3.14f/2.0f, 
			this->renderer->GetBBWidth()/this->renderer->GetBBHeight(),
			0.1f, 100.0f));
		
		//render as a wireframe mesh
		ID3D11RasterizerState *new_rs = NULL;
		ID3D11RasterizerState *old_rs = NULL;

		D3D11_RASTERIZER_DESC rs_desc;
		this->renderer->GetDeviceContext()->RSGetState(&old_rs);
		old_rs->GetDesc(&rs_desc);
		rs_desc.CullMode = D3D11_CULL_NONE;
		rs_desc.FillMode = D3D11_FILL_WIREFRAME;
		
		this->renderer->GetDevice()->CreateRasterizerState(&rs_desc, &new_rs);
		this->renderer->GetDeviceContext()->RSSetState(new_rs);

		//do rendering 
		this->renderer->SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		this->renderer->GetDeviceContext()->Draw(this->num_tris, 0);

		this->renderer->GetDeviceContext()->RSSetState(old_rs);
	}
}