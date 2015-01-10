#ifndef CAMERA_3D_H
#define CAMERA_3D_H

#include "Math.h"
#include "Error.h"
#include "common_include.h"

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace NBody
{
	class Camera3D
	{
	private: 
		Matrix view;
		Matrix invView;
		Vector3 translation;
		Matrix rotation;
		bool val_changed;

	public:
		Camera3D(void);
		~Camera3D(void) {}
		
		void RotateAxisAngle(Vector3 v, double angle);
		void SetRotationAxisAngle(Vector3 v, double angle);
		void SetPos(Vector3 v);

		Vector3 GetPos(void) const { return this->translation; }
		Matrix GetView(void);
		Matrix GetInvView(void);

	private:
		void buildView(void);
	};

	
};
#endif

