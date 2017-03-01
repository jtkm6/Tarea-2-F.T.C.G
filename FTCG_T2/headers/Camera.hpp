#pragma once

#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

class Camera{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;
	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			GLfloat yaw = YAW,
			GLfloat pitch = PITCH) :
			Front(glm::vec3(0.0f, 0.0f, -1.0f)),
			MovementSpeed(SPEED),
			MouseSensitivity(SENSITIVTY),
			Zoom(ZOOM){

		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
			GLfloat upX, GLfloat upY, GLfloat upZ,
			GLfloat yaw, GLfloat pitch) :
			Front(glm::vec3(0.0f, 0.0f, -1.0f)),
			MovementSpeed(SPEED),
			MouseSensitivity(SENSITIVTY),
			Zoom(ZOOM){

		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix(){
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input
	// parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime){
		GLfloat velocity = MovementSpeed * deltaTime;
		if(direction == FORWARD)
			Position += Front * velocity;
		if(direction == BACKWARD)
			Position -= Front * velocity;
		if(direction == LEFT)
			Position -= Right * velocity;
		if(direction == RIGHT)
			Position += Right * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both
	// the x and y direction.
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true){
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;
		Yaw += xoffset;
		Pitch += yoffset;
		if(constrainPitch){
			if(Pitch > 89.0f)
				Pitch = 89.0f;
			if(Pitch < -89.0f)
				Pitch = -89.0f;
		}
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yoffset){
		if(Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if(Zoom <= 1.0f)
			Zoom = 1.0f;
		if(Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors(){
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
};