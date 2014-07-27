//#include "Camera3D.h"
//#include <SimpleMath.h>
//
//using namespace DirectX::SimpleMath;
//
//namespace NBody
//{
//	Camera3D::Camera3D(void)
//	{
//		this->view = Matrix::Identity();
//		this->rotation = Matrix::Identity();
//		this->translation = Vector3(0.0f, 0.0f, 0.0f);
//	}
//
//	void Camera3D::RotateInView(const Matrix &m)
//	{
//		Matrix m_world = m * this->invView;
//		this->rotation *= m_world;
//
//		this->buildView();
//	}
//
//	void buildViewFromMat(const Matrix &m)
//	{
//			
//	}
//
//	void Camera3D::SetView(const Matrix &m)
//	{
//		this->buildViewFromMat(m);
//	}
//
//	void Camera3D::RotateInWorld(const Matrix &m)
//	{
//		this->rotation *= m;
//
//		this->buildView();
//	}
//
//	void Camera3D::TranslateInView(const Vector3 &translation)
//	{
//		Vector3 translation_world = Vector3::TransformNormal(translation, this->invView);
//		this->translation += translation_world;
//
//		this->buildView();
//	}
//
//	void Camera3D::TranslateInWorld(const Vector3 &translation)
//	{
//		this->translation += translation;
//
//		this->buildView();
//	}
//
//	void Camera3D::LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
//	{
//		this->view = Matrix::CreateLookAt(eye, target, up);
//		this->translation.x = this->view.m[3][0];
//		this->translation.y = this->view.m[3][1];
//		this->translation.z = this->view.m[3][2];
//
//		this->rotation = this->view;
//		this->rotation.m[3][0] = 0.0f;
//		this->rotation.m[3][1] = 0.0f;
//		this->rotation.m[3][2] = 0.0f;
//	}
//
//	void Camera3D::buildViewFromMat(const Matrix &m)
//	{
//		this->view = m;
//		this->translation = Vector3(this->view.m[3][0], this->view.m[3][1], this->view.m[3][2]);
//
//		this->rotation = this->view;
//		this->rotation.m[3][0] = 0.0f;
//		this->rotation.m[3][1] = 0.0f;
//		this->rotation.m[3][2] = 0.0f;
//
//		this->invView = this->view.Invert();
//	}
//
//	void Camera3D::buildView(void)
//	{
//		this->view = Matrix::Identity();
//		this->view *= this->rotation;
//		this->view.m[3][0] = -this->translation.x;
//		this->view.m[3][1] = -this->translation.y;
//		this->view.m[3][2] = -this->translation.z;
//
//		this->invView = this->view.Invert();
//	}
//};		