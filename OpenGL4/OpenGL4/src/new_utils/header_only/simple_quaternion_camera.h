#pragma once

#include<glad/glad.h> //glfw depends on this header so it must be included first
#include<GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assert.h>
#include "assimp/Compiler/pstdint.h"
#include "ICamera.h"

class QuaternionCamera final : public ICamera
{
public:
	enum class LookMode { Free, Orbit };
	QuaternionCamera()
	{	
		updateBasisVectors();
	}

public: //static helpers
	static inline bool anyValueNAN(glm::vec3 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z); }
	static inline bool anyValueNAN(glm::vec4 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z) || glm::isnan(vec.w); };
	static inline bool anyValueNAN(glm::quat quat)
	{
		glm::bvec4 vec = glm::isnan(quat);
		return vec.x || vec.y || vec.z || vec.w;
	}
public:
	void tick(float dt_sec, GLFWwindow* window)
	{
		//in real applications one should hook into the associated events with callbacks rather than polling every frame

		bool bCTRL = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
		bool bALT = glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
		bool bSHIFT = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

		////////////////////////////////////////////////////////
		// check mouse movement
		////////////////////////////////////////////////////////
		if (bEnableOrbitModeSwitch) { lookMode = bALT ? LookMode::Orbit : LookMode::Free; }

		int windowHeight, windowWidth;
		double x, y;
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		glfwGetCursorPos(window, &x, &y);

		x += double(windowWidth) / 0.5;
		y += double(windowHeight) / 0.5;

		glm::vec2 deltaMouse{ float(x - lastX), float(y - lastY) };
		lastX = x;
		lastY = y;
		handleMouseMoved(dt_sec, deltaMouse);

		////////////////////////////////////////////////////////
		// Cursor mode
		////////////////////////////////////////////////////////
		static bool bJustPressedCursorMode = false;
		if (glfwGetKey(window, cursorModeKey) == GLFW_PRESS)
		{
			if (!bJustPressedCursorMode)
			{
				bCursorMode = !bCursorMode;
				glfwSetInputMode(window, GLFW_CURSOR, bCursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

				//only flip once until they release
				bJustPressedCursorMode = true;
			}
		}
		else { bJustPressedCursorMode = false; }


		////////////////////////////////////////////////////////
		// handle movement
		////////////////////////////////////////////////////////
		glm::vec3 inputVector{ 0.f };
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			inputVector += -w_axis;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			inputVector -= -w_axis;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			inputVector += u_axis;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			inputVector += -u_axis;
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		{
			inputVector += v_axis;
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		{
			inputVector += -v_axis;
		}
		if (glm::length2(inputVector) != 0.f)
		{
			inputVector = glm::normalize(inputVector);
			pos += inputVector * cameraSpeed * dt_sec * (bCTRL?10.f:1.f) * (bSHIFT?0.1f:1.f);
		}

		////////////////////////////////////////////////////////
		// handle roll
		////////////////////////////////////////////////////////
		float rollDirection = 0.f;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			rollDirection += -1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			rollDirection += 1.0f;
		}
		if (rollDirection != 0.f)
		{
			glm::quat roll = glm::angleAxis(rollDirection * rollSpeed * dt_sec * (bSHIFT?0.25f:1.f), -w_axis);
			rotation = roll * rotation;
			updateBasisVectors();
		}
	}

	void handleMouseMoved(float dt_sec, const glm::vec2& deltaMouse)
	{
		if (!bCursorMode)
		{
			glm::vec3 orbitPoint{ 0.f };
			if(lookMode == LookMode::Orbit)
			{ 
				orbitPoint = (orbitDistance * getFront()) + getPosition();
			}

			glm::vec3 uvPlaneVec = u_axis * deltaMouse.x;
			uvPlaneVec += v_axis * -deltaMouse.y;

			float rotationMagnitude = glm::length(uvPlaneVec);
			if (rotationMagnitude == 0.0f) { return; }
			uvPlaneVec = glm::normalize(uvPlaneVec);

			glm::vec3 rotationAxis = glm::normalize(glm::cross(uvPlaneVec, w_axis));
			glm::quat deltaQuat = glm::angleAxis(dt_sec * mouseSensitivity * rotationMagnitude, rotationAxis);
			assert(!anyValueNAN(deltaQuat));

			rotation = deltaQuat * rotation;
			updateBasisVectors();

			if(lookMode == LookMode::Orbit) 
			{ 
				pos = orbitPoint + (orbitDistance * -getFront());
			}
		}
	}

	void updateBasisVectors()
	{
		u_axis = rotation * glm::vec3{ 1,0, 0}; //perhaps should normalize to be extra safe from fp imprecision ?
		v_axis = rotation * glm::vec3{ 0,1, 0};
		w_axis = rotation * glm::vec3{ 0,0,1};

		assert(!anyValueNAN(u_axis));
		assert(!anyValueNAN(v_axis));
		assert(!anyValueNAN(w_axis));
	}

	virtual glm::mat4 getView() override 
	{
		return glm::lookAt(pos, pos + -w_axis, v_axis);
	}
	virtual glm::vec3 getFront() override { return -w_axis; }
	virtual glm::vec3 getRight() override { return u_axis; }
	virtual glm::vec3 getUp() override { return v_axis; }
	virtual glm::vec3 getPosition() override { return pos; }
	virtual float getFOVy_rad() override { return fovY_rad; }
public: //public so demostrations can easily tweak; tick will correct
	float cameraSpeed = 10.0f; //NDCs per second
	float rollSpeed = glm::radians<float>(180.f);
	float fovY_rad = glm::radians<float>(45.f);
	glm::vec3 pos{ 0.f, 0.f, 0.f };
	glm::quat rotation{ 1.f,0,0,0 };
	uint32_t cursorModeKey = GLFW_KEY_ESCAPE;
private: //tick state
	double lastX = 0, lastY = 0;
private:
	glm::vec3 u_axis{ 1.f,0.f,0.f };
	glm::vec3 v_axis{ 0.f,1.f,0.f };
	glm::vec3 w_axis{ 0.f,0.f,1.f }; 
	float mouseSensitivity = 0.125f;
	float orbitDistance = 5.f;
	bool bCursorMode = true;
	bool bEnableOrbitModeSwitch = true;
	LookMode lookMode = LookMode::Free;
};