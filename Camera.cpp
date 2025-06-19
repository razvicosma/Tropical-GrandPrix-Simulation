#include "Camera.hpp"

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, this->cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        rotate(pitch, yaw);
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    void Camera::setPosition(const glm::vec3& position) {
        cameraPosition = position;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    void Camera::setTarget(const glm::vec3& target) {
        cameraTarget = target;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    void Camera::move(MOVE_DIRECTION direction, float deltaTime) {
        const float speed = cameraSpeed * deltaTime;
        glm::vec3 frontOff = cameraFrontDirection * speed;
        glm::vec3 rightOff = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;

        if (direction == MOVE_FORWARD) {
            cameraPosition += frontOff;
        }
        else if (direction == MOVE_BACKWARD) {
            cameraPosition -= frontOff;
        }
        else if (direction == MOVE_RIGHT) {
            cameraPosition += rightOff;
        }
        else if (direction == MOVE_LEFT) {
            cameraPosition -= rightOff;
        }
    }

    void Camera::rotate(float pitch, float yaw) {
        cameraFrontDirection = glm::normalize(glm::vec3(
            cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
            sin(glm::radians(pitch)),
            cos(glm::radians(pitch)) * sin(glm::radians(yaw))
        ));

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    glm::mat4 Camera::getProjectionMatrix(float aspectRatio)
    {
        return glm::perspective(zoom, aspectRatio, nearPlane, farPlane);
    }
}