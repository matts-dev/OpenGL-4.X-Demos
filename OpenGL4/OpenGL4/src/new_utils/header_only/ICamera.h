#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

struct ICamera
{
	virtual ~ICamera() {}

	virtual glm::mat4 getView() = 0;
	virtual glm::vec3 getFront() = 0;
	virtual glm::vec3 getRight() = 0;
	virtual glm::vec3 getUp() = 0;
	virtual glm::vec3 getPosition() = 0;
	virtual float getFOVy_rad() = 0;
};
