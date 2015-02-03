#ifndef PROC_SPHERE_H
#define PROC_SPHERE_H

#include "Renderer.h"
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>

namespace Core
{
	class ProcSphere
	{
	private: 
		Renderer *renderer;
		ID3D11Buffer *vBuf;
		ID3D11Buffer *iBuf;
		int perms;
		int num_tris;

	public:
		ProcSphere(void);
		bool Init(Renderer *renderer);
		bool Generate(int permutations);
		void Render();
		~ProcSphere();
	private:
		void __generate_faces(Vector4 verts[3], std::vector<Vector4> &output, int permutations);
		void __adjust_vert(Vector4 &v);
	};

}
#endif

