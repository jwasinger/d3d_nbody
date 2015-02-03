#include "a2.h"

double cot(double val)
{
    return 1.0 / (tan(val));
}

Matrix Matrix::CreateRotationAxisAngle(Vector3 axis, double angle)
{
    Matrix axis_angle;
}

Matrix Matrix::CreateOrthographic(double left, double right, double bottom, double top, double near, double far)
{
    Matrix m;
    m.m00 = 2.0 / (right - left);
    m.m10 = 0.0;
    m.m20 = 0.0;
    m.m30 = -((right + left)/(right-left));
  
    m.m01 = 0.0;
    m.m11 = 2.0 / (top - bottom);
    m.m21 = 0.0;
    m.m31 = - ((top + bottom) / (top - bottom));
    
    m.m02 = 0.0;
    m.m12 = 0.0;
    m.m22 = 1.0  / (far - near);
    m.m32 = - (near) / (far - near);

    m.m03 = 0.0;
    m.m13 = 0.0;
    m.m23 = 0.0;
    m.m33 = 1.0;
}

Matrix Matrix::CreatePerspectiveFOV(double fov, double aspect, double near, double far)
{
    m.m00 = (1.0/aspect)*cot(fov/2.0);
    m.m10 = 0.0;
    m.m20 = 0.0;
    m.m30 = 0.0;

    m.m01 = 0.0;
    m.m11 = cot(fov/2.0);
    m.m21 = 0.0;
    m.m31 = 0.0;

    m.m02 = 0.0;
    m.m12 = 0.0;
    m.m22 = far / (far - near);
    m.m32 = - (far * near)/(far - near);
    
    m.m03 = 0.0;
    m.m13 = 0.0;
    m.m23 = 1.0;
    m.m33 = 0.0;
}
