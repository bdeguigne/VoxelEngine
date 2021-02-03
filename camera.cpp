#include "camera.hpp"

namespace vxe
{
    Camera::Camera()
    {
        _viewMatrix = glm::mat4(1.f);

        _movementSpeed = 5.0f;
        _sensitivity = 5.0f;

        _worldUp = glm::vec3(0.f, 1.f, 0.f);
        _direction = glm::vec3(0.f, 0.f, 90.f);
        _position = glm::vec3(0.f, -6.f, -10.f);
        _right = glm::vec3(0.f);
        _up = _worldUp;

        _pitch = 50.f;
        _yaw = -90.f;
        _roll = 0.f;
    }

    const glm::mat4 Camera::getViewMatrix()
    {
        _updateCameraVectors();

        _viewMatrix = glm::lookAt(_position, _position + _front, _worldUp);

        return _viewMatrix;
    }

    void Camera::setMousePos(const float &dt, const double &mouseX, const double &mouseY)
    {
        double mouseOffsetX;
        double mouseOffsetY;

        if (_firstMouse)
        {
            _lastMouseX = mouseX;
            _lastMouseY = mouseY;
            _firstMouse = false;
        }

        //Calc offset
        mouseOffsetX = mouseX - _lastMouseX;
        mouseOffsetY = _lastMouseY - mouseY;

        //Set last X and Y
        _lastMouseX = mouseX;
        _lastMouseY = mouseY;

        _updateMouseInput(dt, mouseOffsetX, mouseOffsetY);
    }

    void Camera::move(float speed, const int direction, float dt)
    {
        switch (direction)
        {
        // case UP:
        //     _position = moveUp(_position, speed, dt);
        //     break;
        // case DOWN:
        //     _position = moveDown(_position, speed, dt);
        //     break;
        case FORWARD:
            _position += _front * speed * dt;
            break;
        case BACKWARD:
            _position -= _front * speed * dt;
            break;
        case LEFT:
            _position -= _right * speed * dt;
            break;
        case RIGHT:
            _position += _right * speed * dt;
            break;
        default:
            break;
        }
    }

    void Camera::_updateCameraVectors()
    {
        _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front.y = sin(glm::radians(_pitch));
        _front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

        _front = glm::normalize(_front);
        _right = glm::normalize(glm::cross(_front, _worldUp));
        _up = glm::normalize(glm::cross(_right, _front));
    }

    void Camera::_updateMouseInput(const float &dt, const double &offsetX, const double &offsetY)
    {
        _pitch += static_cast<float>(offsetY) * _sensitivity * dt;
        _yaw += static_cast<float>(offsetX) * _sensitivity * dt;

        if (_pitch > 80.f)
            _pitch = 80.f;
        else if (_pitch < -80.f)
            _pitch = -80.f;

        if (_yaw > 360.f || _yaw < -360.f)
        {
            _yaw = 0.f;
        }
    }
} // namespace vxe
