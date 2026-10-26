#pragma once
#include "Common.h"

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float fov;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 60.0f, 0.0f));

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void updateCameraVectors();
};
