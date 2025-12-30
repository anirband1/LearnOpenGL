#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float CAMERA_SPEED = 3.0f;
const float MOUSE_SENSITIVITY = 0.1f;

const float INIT_YAW = -90.0f;
const float INIT_PITCH = 0.0f;
const float INIT_ZOOM = 45.0f;

class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 LookDir;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = INIT_YAW,
           float pitch = INIT_PITCH) : LookDir(glm::vec3(0.0f, 0.0f, -1.0f)),
                                       MovementSpeed(CAMERA_SPEED),
                                       MouseSensitivity(MOUSE_SENSITIVITY),
                                       Zoom(INIT_ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // constructor with scalars
    Camera(float posX, float posY,
           float posZ,
           float upX,
           float upY,
           float upZ,
           float yaw,
           float pitch) : LookDir(glm::vec3(0.0f, 0.0f, -1.0f)),
                          MovementSpeed(CAMERA_SPEED),
                          MouseSensitivity(MOUSE_SENSITIVITY),
                          Zoom(INIT_ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + LookDir, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 forwardHorizon = glm::normalize(glm::vec3(LookDir.x, 0.0f, LookDir.z));

        if (direction == FORWARD)
            Position += forwardHorizon * velocity;
        if (direction == BACKWARD)
            Position -= forwardHorizon * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
            Pitch = clamp(Pitch, -89.0f, 89.0f);

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;

        Zoom = clamp(Zoom, 1.0f, 179.0f);
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 direction;
        direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        direction.y = -sin(glm::radians(Pitch));
        direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        LookDir = glm::normalize(direction);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(LookDir, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.

        // Up = glm::normalize(glm::cross(Right, LookDir));
        Up = glm::vec3(0.0f, 1.0f, 0.0f); // for minecraft-like controls
    }

    float clamp(float val, float min, float max)
    {
        val = val < min ? min : val;
        val = val > max ? max : val;
        return val;
    }
};
#endif
