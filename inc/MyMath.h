#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace NBody
{
	static Matrix CreateOrthographicLH(float width, float height, float zNearPlane, float zFarPlane)
	{
		Matrix m = Matrix::CreateOrthographic(width, height, zNearPlane, zFarPlane);
		m.m[2][2] *= -1;
		return m;
	}
}