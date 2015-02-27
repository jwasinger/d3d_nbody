#include "Camera3D.h"
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Core
{
	Camera3D::Camera3D(void)
	{
		this->view = Matrix::Identity();
		this->right = Vector3(1.0f, 0.0f, 0.0f);
		this->up = Vector3(0.0f, 1.0f, 0.0f);
		this->target = Vector3(0.0f, 0.0f, 1.0f);
		this->translation = Vector3(0.0f, 0.0f, 0.0f);
		this->val_changed = false;
	}

	void Camera3D::Walk(float dist)
	{
		this->translation += this->target * -dist;
		this->val_changed = true;
	}

	void Camera3D::Strafe(float dist)
	{
		this->translation += this->right * dist;
		this->val_changed = true;
	}

	void Camera3D::buildView(void)
	{
		this->view = Matrix::Identity();

		//re-orthogonalize vectors
		this->target.Normalize();
		this->up = this->target.Cross(this->right);
		this->up.Normalize();
		this->right = this->up.Cross(this->target);
		this->right.Normalize();

		Vector3 pos = Vector3(-this->translation.Dot(this->right), -this->translation.Dot(this->up), -this->translation.Dot(this->target));
		this->view = Matrix(
			right.x, up.x, target.x, 0.0f,
			right.y, up.y, target.y, 0.0f,
			right.z, up.z, target.z, 0.0f,
			pos.x, pos.y, pos.z, 1.0f);
		
		this->invView = this->view.Invert();
		this->val_changed = false;
	}

	void Camera3D::Yaw(double angle)
	{
		Matrix rot = Matrix::CreateFromAxisAngle(this->right, angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
		this->val_changed = true;
	}

	void Camera3D::Pitch(double angle)
	{
		Matrix rot = Matrix::CreateFromAxisAngle(this->up, angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
		this->val_changed = true;
	}

	void Camera3D::Roll(double angle)
	{
		Matrix rot = Matrix::CreateFromAxisAngle(this->target, angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
		this->val_changed = true;
	}

	void Camera3D::RotateX(double angle)
	{
		Matrix rot = Matrix::CreateRotationX(angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
		this->val_changed = true;
	}

	void Camera3D::RotateY(double angle)
	{
		Matrix rot = Matrix::CreateRotationY(angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
		this->val_changed = true;
	}

	void Camera3D::RotateZ(double angle)
	{
		Matrix rot = Matrix::CreateRotationZ(angle);
		this->up = Vector3::TransformNormal(this->up, rot);
		this->target = Vector3::TransformNormal(this->target, rot);
		this->right = Vector3::TransformNormal(this->right, rot);
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
};		