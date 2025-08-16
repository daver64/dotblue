#include "DotBlue/GLPlatform.h"
#include <glm/gtc/matrix_transform.hpp>

namespace DotBlue
{
    GLCamera::GLCamera()
        : position(0.0, 0.0, 0.0),
          target(0.0, 0.0, -1.0),
          up(0.0, 1.0, 0.0),
          fov(60.0),
          aspect(16.0 / 9.0),
          nearPlane(0.1),
          farPlane(1e8) {}

    void GLCamera::setPosition(const glm::dvec3 &pos) { position = pos; }
    void GLCamera::setTarget(const glm::dvec3 &t) { target = t; }
    void GLCamera::setUp(const glm::dvec3 &u) { up = u; }
    void GLCamera::setFOV(double fovDegrees) { fov = fovDegrees; }
    void GLCamera::setAspect(double a) { aspect = a; }
    void GLCamera::setNearFar(double n, double f)
    {
        nearPlane = n;
        farPlane = f;
    }

    const glm::dvec3 &GLCamera::getPosition() const { return position; }
    const glm::dvec3 &GLCamera::getTarget() const { return target; }
    const glm::dvec3 &GLCamera::getUp() const { return up; }
    double GLCamera::getFOV() const { return fov; }
    double GLCamera::getAspect() const { return aspect; }
    double GLCamera::getNear() const { return nearPlane; }
    double GLCamera::getFar() const { return farPlane; }

    glm::dmat4 GLCamera::getViewMatrix() const
    {
        return glm::lookAt(position, target, up);
    }

    glm::dmat4 GLCamera::getProjectionMatrix() const
    {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    void GLCamera::move(const glm::dvec3 &delta)
    {
        position += delta;
        target += delta;
    }

    void GLCamera::rotate(double yaw, double pitch)
    {
        // Simple yaw/pitch rotation around the camera's position
        glm::dvec3 dir = glm::normalize(target - position);
        double yawRad = glm::radians(yaw);
        double pitchRad = glm::radians(pitch);
        // Yaw (around up)
        glm::dmat4 yawMat = glm::rotate(glm::dmat4(1.0), yawRad, up);
        dir = glm::dvec3(yawMat * glm::dvec4(dir, 0.0));
        // Pitch (around right)
        glm::dvec3 right = glm::normalize(glm::cross(dir, up));
        glm::dmat4 pitchMat = glm::rotate(glm::dmat4(1.0), pitchRad, right);
        dir = glm::dvec3(pitchMat * glm::dvec4(dir, 0.0));
        target = position + dir;
    }

} // namespace DotBlue
