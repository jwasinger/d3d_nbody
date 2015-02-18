namespace A2Math
{
    class Vector3
    {
    private:
        double v_x, v_y, v_z;

    public: 
        Vector3 operator* (Vector3 &v1, Vector3 &v2);
        Vector3 Dot(Vector3 &v);
        Vector3 Cross(const Vector3 &v);

        double X() const { return v_x; }
        double Y() const { return v_y; }
        double Z() const { return v_z; } 
    };
}
