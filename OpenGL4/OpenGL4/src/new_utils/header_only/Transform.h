#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ho
{
	struct Transform
	{
		inline glm::mat4 getModelMatrix() const noexcept
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, position);
			model = model * glm::toMat4(rotQuat);
			model = glm::scale(model, scale);
			return model;
		}

		glm::vec3 position = { 0, 0, 0 };
		glm::quat rotQuat = glm::quat{ 1, 0, 0, 0 };
		glm::vec3 scale = { 1, 1, 1 };
	};
}
