#include "Camera.hpp"
#include <iostream>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::cross(this->cameraFrontDirection, cameraUp);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction)
        {
            case MOVE_FORWARD:
                this->cameraPosition = this->cameraPosition + this->cameraFrontDirection * speed;
                this->cameraTarget = this->cameraTarget + this->cameraFrontDirection * speed;
                break;

            case MOVE_BACKWARD:
                this->cameraPosition = this->cameraPosition - this->cameraFrontDirection * speed;
                this->cameraTarget = this->cameraTarget - this->cameraFrontDirection * speed;
                break;

            case MOVE_RIGHT:
                this->cameraPosition = this->cameraPosition + this->cameraRightDirection * speed;
                this->cameraTarget = this->cameraTarget + this->cameraRightDirection * speed;
                break;

            case MOVE_LEFT:
                this->cameraPosition = this->cameraPosition - this->cameraRightDirection * speed;
                this->cameraTarget = this->cameraTarget - this->cameraRightDirection * speed;
                break;

            default:
                break;
        }
        //std::cout << "Position:" << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << "\n";
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        if (glm::abs(pitch) > 0.0001f || glm::abs(yaw) >= 0.0001f)
        {
            glm::mat4 rotations = glm::rotate(glm::mat4(1.0f), pitch, this->cameraRightDirection);
            rotations = glm::rotate(rotations, yaw, this->cameraUpDirection);
            
            glm::vec3 newCameraFrontDirection = rotations * glm::vec4(this->cameraFrontDirection, 0.0f);
            glm::vec3 newCameraRightDirection = glm::cross(newCameraFrontDirection, this->cameraUpDirection);

            //a vector that is straigth forward from camera position
            glm::vec3 straigthForward = glm::cross(this->cameraUpDirection, newCameraRightDirection);
            float dotVal = glm::dot(newCameraFrontDirection, straigthForward);
            if (dotVal > 0.1f) // angle < 90 => valid rotation on x axis(clamp rotation)
            {
                this->cameraTarget = this->cameraPosition + newCameraFrontDirection;
                this->cameraFrontDirection = newCameraFrontDirection;
                this->cameraRightDirection = newCameraRightDirection;
            }
            //std::cout << "dotVal = " << dotVal << "\n";
        }
    }

    glm::vec3 Camera::getCameraPosition()
    {
        return this->cameraPosition;
    }
}