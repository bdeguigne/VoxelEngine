#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace vxe
{
    enum direction
    {
        FORWARD = 0,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    class Camera
    {
    public:
        Camera();
        const glm::mat4 getViewMatrix();
        const glm::vec3 getPosition() { return _position; }

        void setMousePos(const float &dt, const double &mouseX, const double &mouseY);
        void move(float speed, const int direction, float dt);

    private:
        void _updateCameraVectors();
        void _updateMouseInput(const float &dt, const double &offsetX, const double &offsetY);

        glm::mat4 _viewMatrix;

        float _movementSpeed;
        float _sensitivity;

        glm::vec3 _worldUp;
        glm::vec3 _position;
        glm::vec3 _direction;
        glm::vec3 _front;
        glm::vec3 _right;
        glm::vec3 _up;

        float _pitch;
        float _yaw;
        float _roll;

        bool _firstMouse = false;
        double _lastMouseX;
        double _lastMouseY;
        double _mouseOffsetX;
        double _mouseOffsetY;
    };
} // namespace vxe
