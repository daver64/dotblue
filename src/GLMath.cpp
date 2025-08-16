#include "DotBlue/GLPlatform.h"

namespace DotBlue
{
    namespace Math
    {

        glm::dvec3 sphericalToCartesian(const SphericalCoord &sph)
        {
            double latRad = glm::radians(sph.latitude);
            double lonRad = glm::radians(sph.longitude);
            double x = sph.radius * std::cos(latRad) * std::cos(lonRad);
            double y = sph.radius * std::sin(latRad);
            double z = sph.radius * std::cos(latRad) * std::sin(lonRad);
            return glm::dvec3(x, y, z);
        }

        SphericalCoord cartesianToSpherical(const glm::dvec3 &v)
        {
            double r = glm::length(v);
            double lat = glm::degrees(std::asin(v.y / r));
            double lon = glm::degrees(std::atan2(v.z, v.x));
            return SphericalCoord{lat, lon, r};
        }

        Mat4 perspective(float fov, float aspect, float near, float far)
        {
            return glm::perspective(fov, aspect, near, far);
        }

        Mat4 ortho(float left, float right, float bottom, float top, float near, float far)
        {
            return glm::ortho(left, right, bottom, top, near, far);
        }

        Mat4 lookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
        {
            return glm::lookAt(eye, center, up);
        }

        Mat4 translate(const Mat4 &matrix, const Vec3 &translation)
        {
            return glm::translate(matrix, translation);
        }

        Mat4 rotate(const Mat4 &matrix, float angle, const Vec3 &axis)
        {
            return glm::rotate(matrix, angle, axis);
        }

        Mat4 scale(const Mat4 &matrix, const Vec3 &scaling)
        {
            return glm::scale(matrix, scaling);
        }

        float radians(float degrees)
        {
            return glm::radians(degrees);
        }

        float degrees(float radians)
        {
            return glm::degrees(radians);
        }

    } // namespace Math
} // namespace DotBlue
