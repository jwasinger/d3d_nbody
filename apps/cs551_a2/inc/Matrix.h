#ifndef MATRIX_H
#define MATRIX_H

#include <DirectXMath.h>

namespace A2Math
{


  class Matrix
  {
  private:
      double m00, m10, m20, m30;
      double m01, m11, m21, m31;
      double m02, m12, m22, m32;
      double m03, m13, m23, m33;

  public:
      Matrix operator* (Matrix &mat1, Matrix &mat2);
      operator DirectXMath::Matrix const ();

      static Matrix CreateRotationAxisAngle(Vector3 axis, double angle); 
      static Matrix CreateTranslation(Vector3 vec);
      static Matrix CreateScale(Vector3 vec);
      static Matrix CreateOrthographicLH(double left, double right, double bottom, double top, double near, double far);

      static Matrix CreatePerspectiveFOV(double fov, double near, double far);

      static Matrix CreateLookAt(Vector3 target, Vector3 eye, Vector3 up);
  };

}

#endif
