#include "DotBlue/GLPlatform.h"

namespace DotBlue {
namespace Math {

mat4 perspective(float fov, float aspect, float near, float far) {
    return glm::perspective(fov, aspect, near, far);
}

mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    return glm::ortho(left, right, bottom, top, near, far);
}

mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up) {
    return glm::lookAt(eye, center, up);
}

mat4 translate(const mat4& matrix, const vec3& translation) {
    return glm::translate(matrix, translation);
}

mat4 rotate(const mat4& matrix, float angle, const vec3& axis) {
    return glm::rotate(matrix, angle, axis);
}

mat4 scale(const mat4& matrix, const vec3& scaling) {
    return glm::scale(matrix, scaling);
}

float radians(float degrees) {
    return glm::radians(degrees);
}

float degrees(float radians) {
    return glm::degrees(radians);
}

} // namespace Math
} // namespace DotBlue
