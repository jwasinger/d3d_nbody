#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Core
{
	static Matrix CreateOrthographicOffCenterLH(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		Matrix m = Matrix::CreateOrthographicOffCenter(left, right, bottom, top, zNear, zFar);
		m.m[2][2] *= -1;
		return m;
	}

	static Matrix CreateOrthographicLH(float width, float height, float zNearPlane, float zFarPlane)
	{
		Matrix m = Matrix::CreateOrthographic(width, height, zNearPlane, zFarPlane);
		m.m[2][2] *= -1;
		return m;
	}
}