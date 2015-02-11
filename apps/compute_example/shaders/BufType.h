#ifndef HLSL_CODE
#include <DirectXMath.h>
#endif

struct PixelColor
{
	float B;
	float G;
	float R;
};

#ifndef HLSL_CODE
__declspec(align(16))
#endif
struct ImageDims
{
	unsigned int numRows;
	unsigned int numCols;
};

#define KERNEL_SIZE		9

#ifndef HLSL_CODE
__declspec(align(16)) struct XMFLOAT1A : public DirectX::XMFLOAT3
{
	XMFLOAT1A() : XMFLOAT3() {}
	XMFLOAT1A(float _x) : XMFLOAT3(_x, _x, _x) {}
	XMFLOAT1A(float _x, float _y, float _z) : XMFLOAT3(_x, _y, _z) {}
	explicit XMFLOAT1A(_In_reads_(3) const float *pArray) : XMFLOAT3(pArray) {}

	XMFLOAT1A& operator= (const XMFLOAT1A& Float1) { x = Float1.x; y = Float1.y; z = Float1.z; return *this; }
};
#endif

#ifndef HLSL_CODE
__declspec(align(16))
#endif
struct BlurKernel
{
#ifdef HLSL_CODE
	float4 d[KERNEL_SIZE * KERNEL_SIZE];
#else
	XMFLOAT1A d[KERNEL_SIZE * KERNEL_SIZE];
#endif
};