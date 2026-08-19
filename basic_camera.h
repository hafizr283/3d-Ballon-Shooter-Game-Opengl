//
//  basic_camera.h
//  test
//
//  Created by Nazirul Hasan on 10/9/23.
//  modified by Badiuzzaman on 3/11/24.
//

#ifndef basic_camera_h
#define basic_camera_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class BasicCamera {
public:
    // Camera Attributes
    glm::vec3 eye;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    // Euler Angles
    float Yaw;
    float Pitch;
    
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    // Constructor with vectors
    BasicCamera(float eyeX = 0.0, float eyeY = 1.0, float eyeZ = 3.0, float lookAtX = 0.0, float lookAtY = 0.0, float lookAtZ = 0.0, glm::vec3 viewUpVector = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        eye = glm::vec3(eyeX, eyeY, eyeZ);
        WorldUp = viewUpVector;
        Yaw = YAW;
        Pitch = PITCH;
        MovementSpeed = SPEED;
        MouseSensitivity = SENSITIVITY;
        Zoom = ZOOM;
        
        // Calculate initial front vector from lookAt
        glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
        Front = glm::normalize(lookAt - eye);
        
        // Also update Yaw/Pitch from Front? 
        // For simplicity, we just use default YAW/PITCH and update vectors, 
        // assuming standard start orientation (looking -Z). 
        // If lookAt input is important, we should calculate Yaw/Pitch from it.
        // Given the use case, we usually start looking at -Z anyway.
        
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 createViewMatrix()
    {
        return glm::lookAt(eye, eye + Front, Up);
    }

    // Processes input received from any keyboard-like input system
    void move(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            eye += Front * velocity;
        if (direction == BACKWARD)
            eye -= Front * velocity;
        if (direction == LEFT)
            eye -= Right * velocity;
        if (direction == RIGHT)
            eye += Right * velocity;
            
        // Keep user on the "ground" or fly? 
        // For now, free fly is fine, or we can constrain Y if needed.
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }
    
    // Accessor for lookAt point (compatibility)
    glm::vec3 getLookAt() {
        return eye + Front;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif /* basic_camera_h */
#pragma once

