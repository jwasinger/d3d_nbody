#include "Camera3D.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace NBody
{
	Camera3D::Camera3D(void)
	{
		this->view = Matrix::Identity();
		this->rotation = Matrix::Identity();
		this->translation = Vector3(0.0f, 0.0f, 0.0f);
		this->val_changed = false;
	}

	void Camera3D::RotateAxisAngle(Vector3 v, double angle)
	{
		Matrix new_rot = Matrix::CreateFromAxisAngle(v, angle);
		this->rotation *= new_rot;
		this->val_changed = true;
	}

	void Camera3D::SetRotationAxisAngle(Vector3 v, double angle)
	{
		Matrix new_rot = Matrix::CreateFromAxisAngle(v, angle);
		this->rotation = new_rot;
		this->val_changed = true;
	}

	Matrix Camera3D::GetView(void)
	{
		if (this->val_changed)
		{
			this->buildView();
			this->val_changed = false;
		}
		
		return this->view;
	}

	void Camera3D::Translate(Vector3 v)
	{
		this->translation += v;
		this->val_changed = true;
	}

	Matrix Camera3D::GetInvView(void)
	{
		if (this->val_changed)
		{
			this->buildView();
			this->val_changed = false;
		}

		return this->invView;
	}

	void Camera3D::buildView(void)
	{
		this->view = Matrix::Identity();
		this->view *= this->rotation;
		this->view.m[3][0] = -this->translation.x;
		this->view.m[3][1] = -this->translation.y;
		this->view.m[3][2] = -this->translation.z;

		this->invView = this->view.Invert();
	}
};		