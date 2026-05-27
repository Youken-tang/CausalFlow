
#include "typedefines.hpp"

using namespace mgsim;

double distance(const Vector3D& source, const Vector3D& target)
{
    auto x = source.x - target.x;
    auto y = source.y - target.y;
    auto z = source.z - target.z;
    return std::sqrt(x * x + y * y + z * z);  
}

const Vector3D operator+(const Vector3D& a, const Vector3D& b)
{
    return Vector3D( a.x + b.x, a.y + b.y, a.z + b.z );
}

const Vector3D operator-(const Vector3D& a, const Vector3D& b)
{
    return Vector3D( a.x - b.x, a.y - b.y, a.z - b.z );
}

const Vector3D operator*(const Vector3D& a, const double scalar)
{
    return Vector3D( a.x * scalar, a.y * scalar, a.z * scalar );
}
