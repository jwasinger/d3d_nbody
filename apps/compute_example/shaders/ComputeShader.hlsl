#define HLSL_CODE
#include "BufType.h"

//#define NUM_OF_THREADS_X		32
//#define NUM_OF_THREADS_Y		32
//#define NUM_OF_THREADS_Z		1


// Global buffers
StructuredBuffer<PixelColor> ImageIn : register(t0);
RWStructuredBuffer<PixelColor> ImageOut : register(u0);
cbuffer dimImage : register(b0) { ImageDims dimImage; }
cbuffer matKernel : register(b1) { BlurKernel matKernel; };

// Group shared buffer
groupshared PixelColor sharedPixelData[NUM_OF_THREADS_X * NUM_OF_THREADS_Y];


//[numthreads(NUM_OF_THREADS_X, NUM_OF_THREADS_Y, NUM_OF_THREADS_Z)]
//void main(uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint GTidx : SV_GroupIndex)
//{
//	/*if (DTid.x >= dimImage.numCols || DTid.y >= dimImage.numRows)
//		return;*/
//
//	uint imgxy_flat = DTid.x + dimImage.numCols * DTid.y;
//	uint2 grouprange_flat =
//	{
//		(Gid.x + 80 * Gid.y) * NUM_OF_THREADS_X * NUM_OF_THREADS_Y * NUM_OF_THREADS_Z,
//		(Gid.x + 80 * Gid.y) * NUM_OF_THREADS_X * NUM_OF_THREADS_Y * NUM_OF_THREADS_Z + NUM_OF_THREADS_X * NUM_OF_THREADS_Y * NUM_OF_THREADS_Z - 1
//	};
//
//	sharedPixelData[GTidx] = ImageIn[imgxy_flat];
//
//	GroupMemoryBarrierWithGroupSync();
//
//	if (DTid.x >= dimImage.numCols ||
//		DTid.y >= dimImage.numRows)
//		return;
//
//	PixelColor pixBlurred;
//
//	/*pixBlurred.B = grouprange_flat.x;
//	pixBlurred.G = grouprange_flat.y;
//	pixBlurred.R = GTidx;*/
//
//	//For every value in the filter around the pixel (c, r)
//	for (int filter_r = -KERNEL_SIZE / 2; filter_r <= KERNEL_SIZE / 2; filter_r++)
//		for (int filter_c = -KERNEL_SIZE / 2; filter_c <= KERNEL_SIZE / 2; filter_c++)
//		{
//			//Find the global image position for this filter position
//			//clamp to boundary of the image
//			uint2 clamped_xy =
//			{
//				clamp(GTid.x + filter_c, 0, dimImage.numCols - 1),
//				clamp(GTid.y + filter_r, 0, dimImage.numRows - 1)
//			};
//			uint clamped_flat = clamped_xy.x + clamped_xy.y * dimImage.numCols;
//			uint filter_flat = (filter_r + KERNEL_SIZE / 2) * KERNEL_SIZE + filter_c + KERNEL_SIZE / 2;
//			float filter_value = matKernel.d[filter_flat].x;
//
//			if (clamped_flat >= grouprange_flat.x && clamped_flat <= grouprange_flat.y)
//			{
//				pixBlurred.B += sharedPixelData[clamped_flat - grouprange_flat.x].B * matKernel.d[filter_flat].x;
//				pixBlurred.G += sharedPixelData[clamped_flat - grouprange_flat.x].G * matKernel.d[filter_flat].y;
//				pixBlurred.R += sharedPixelData[clamped_flat - grouprange_flat.x].R * matKernel.d[filter_flat].z;
//			}
//			else
//			{
//				pixBlurred.B += ImageIn[clamped_flat].B * matKernel.d[filter_flat].x;
//				pixBlurred.G += ImageIn[clamped_flat].G * matKernel.d[filter_flat].y;
//				pixBlurred.R += ImageIn[clamped_flat].R * matKernel.d[filter_flat].z;
//			}
//		}
//
//	ImageOut[imgxy_flat] = pixBlurred;
//}

[numthreads(NUM_OF_THREADS_X, NUM_OF_THREADS_Y, NUM_OF_THREADS_Z)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	if (DTid.x >= dimImage.numCols || DTid.y >= dimImage.numRows)
		return;

	int imgxy_flat = DTid.x + dimImage.numCols * DTid.y;

	PixelColor pixBlurred;
	
	//For every value in the filter around the pixel (c, r)
	for (int filter_r = -KERNEL_SIZE / 2; filter_r <= KERNEL_SIZE / 2; filter_r++)
		[unroll]
		for (int filter_c = -KERNEL_SIZE / 2; filter_c <= KERNEL_SIZE / 2; filter_c++)
		{
			//Find the global image position for this filter position
			//clamp to boundary of the image
			uint2 clamped_xy = 
			{
				clamp(DTid.x + filter_c, 0, dimImage.numCols - 1),
				clamp(DTid.y + filter_r, 0, dimImage.numRows - 1)
			};
			uint clamped_flat = clamped_xy.x + clamped_xy.y * dimImage.numCols;
			uint filter_flat = (filter_r + KERNEL_SIZE / 2) * KERNEL_SIZE + filter_c + KERNEL_SIZE / 2;

			pixBlurred.B += ImageIn[clamped_flat].B * matKernel.d[filter_flat].x;
			pixBlurred.G += ImageIn[clamped_flat].G * matKernel.d[filter_flat].y;
			pixBlurred.R += ImageIn[clamped_flat].R * matKernel.d[filter_flat].z;
		}
	
	ImageOut[imgxy_flat] = pixBlurred;
}
#undef HLSL_CODE