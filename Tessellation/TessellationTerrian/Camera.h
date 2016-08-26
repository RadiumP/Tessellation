
//http://learnopengl.com/#!Getting-started/Camera
#pragma once

#include <vector>

#include <GL\glew.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>

using namespace glm;


enum Camera_Movement {
	FWD,
	BWD,
	LFT,
	RGT
};


const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.25f;
const GLfloat ZOOM = 45.0f;


class Camera
{
public:
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	GLfloat Yaw;
	GLfloat Pitch;
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;
	
	//vector constructor
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : 
		Front(vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED), 
		MouseSensitivity(SENSITIVITY), 
		Zoom(ZOOM)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	//scalar constructor
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) :
		Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		this->Position = vec3(posX, posY, posZ);
		this->WorldUp = vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	mat4 GetViewMatrix()
	{
		return lookAt(this->Position, this->Position + this->Front, this->Up);
	}

	void ProcessKeyboard(Camera_Movement dir, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		if (dir == FWD)
			this->Position += this->Front * velocity;
		if (dir == BWD)
			this->Position -= this->Front * velocity;
		if (dir == LFT)
			this->Position -= this->Right * velocity;
		if (dir == RGT)
			this->Position += this->Right * velocity;
	}

	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}
		this->updateCameraVectors();

	}

	void ProcessMouseScroll(GLfloat yoffset)
	{
		if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
			this->Zoom -= yoffset;
		if (this->Zoom <= 1.0f)
			this->Zoom = 1.0f;
		if (this->Zoom >= 45.0f)
			this->Zoom = 45.0f;
	}

private:
	void updateCameraVectors()
	{
		vec3 front;
		front.x = cos(radians(this->Yaw)) * cos(radians(this->Pitch));
		front.y = sin(radians(this->Pitch));
		front.z = sin(radians(this->Yaw)) * cos(radians(this->Pitch));
		this->Front = normalize(front);

		this->Right = normalize(cross(this->Front, this->WorldUp));
		this->Up = normalize(cross(this->Right, this->Front));

	}
};
