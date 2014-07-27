//#ifndef CAMERA_3D_H
//#define CAMERA_3D_H
//
//#include "Math.h"
//#include "Error.h"
//#include "common_include.h"
//
//#include <SimpleMath.h>
//
//using namespace DirectX::SimpleMath;
//
//namespace NBody
//{
//	class Camera3D
//	{
//	private: 
//		Matrix view;
//		Matrix invView;
//
//		Matrix rotation;
//		Vector3 translation;
//
//	public:
//		Camera3D(void);
//		~Camera3D(void) { }
//		
//		Matrix GetView(void) const { return this->view; }
//		Matrix GetInvView(void) const { return this->invView; }
//		
//		void SetView(const Matrix &m);
//
//		void LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);
//
//		void RotateInView(const Matrix &mat);
//
//		void RotateInWorld(const Matrix &mat);
//
//		void TranslateInWorld(const Vector3 &translation);
//
//		void TranslateInView(const Vector3 &translation);
//
//	private:
//		void buildView(void);
//		void buildViewFromMat(const Matrix &m);
//	};
//
//	
//};
//#endif
//
