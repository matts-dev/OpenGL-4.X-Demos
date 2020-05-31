#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace MathUtils
{
	inline bool anyValueNAN(glm::vec3 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z); }
	inline bool anyValueNAN(glm::vec4 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z) || glm::isnan(vec.w); };
	inline bool anyValueNAN(glm::quat quat)
	{
		glm::bvec4 vec = glm::isnan(quat);
		return vec.x || vec.y || vec.z || vec.w;
	};
#if _DEBUG && _WIN32
#include <intrin.h>
#define NAN_BREAK(value)\
if(anyValueNAN(value))\
{\
	__debugbreak();\
}
#else
#define NAN_BREAK(value) 
#endif //_DEBUG

	inline bool floatEquals(float value, float compareValue, float epsilon = 0.001f)
	{
		//equal if value is under epsilon range
		float delta = std::abs(value - compareValue);
		return delta < epsilon;
	}
	inline bool vectorsAreSame(glm::vec3 first, glm::vec3 second, float epsilon = 0.001)
	{
		return floatEquals(first.x, second.x, epsilon)
			&& floatEquals(first.y, second.y, epsilon)
			&& floatEquals(first.z, second.z, epsilon);
	}

	static glm::vec3 getDifferentVector(glm::vec3 vec)
	{
		if (vec.x < vec.y && vec.x < vec.z) { vec.x = 1.f; }
		else if (vec.y < vec.x && vec.y < vec.z) { vec.y = 1.f; }
		else if (vec.z < vec.x && vec.z < vec.y) { vec.z = 1.f; }
		else //all are equal
		{
			if (vec.x > 0.f) { vec.x = -1.f; }
			else { vec.x = 1.f; }
		}
		return vec;
	}

	static glm::quat getRotationBetween(const glm::vec3& from_n, const glm::vec3& to_n)
	{
		glm::quat rot = glm::quat{ 1, 0, 0, 0 };

		float cosTheta = glm::clamp(glm::dot(from_n, to_n), -1.f, 1.f);

		bool bVectorsAre180 = MathUtils::floatEquals(cosTheta, -1.0f);
		bool bVectorsAreSame = MathUtils::floatEquals(cosTheta, 1.0f);
		
		if (!bVectorsAreSame && !bVectorsAre180)
		{
			glm::vec3 rotAxis = glm::normalize(glm::cross(from_n, to_n)); //theoretically, I don't think I need to normalize if both normal; but generally I normalize the result of xproduct
			float rotDegreesRadians = glm::acos(cosTheta);
			rot = glm::angleAxis(rotDegreesRadians, rotAxis);
		}
		else if (bVectorsAre180)
		{
			//if tail end and front of projectile are not the same, we need a 180 rotation around ?any? axis
			glm::vec3 temp = MathUtils::getDifferentVector(from_n);
			glm::vec3 rotAxisFor180 = glm::normalize(cross(from_n, temp));

			rot = glm::angleAxis(glm::pi<float>(), rotAxisFor180);
		}

		return rot;
	}

	static glm::vec3 projectAontoB(const glm::vec3& a, const glm::vec3& b)
	{
		using namespace glm;

		//scalar projection is doting a vector onto the normalized vector of b
		//		vec3 bUnitVec = b / ||b||;	//ie normali
		//		scalarProj = a dot (b/||b||);	//ie dot with bUnitVec
		// you then then need to scale up the bUnitVector to get a projection vector
		//		vec3 projection = bUnitVector * scalarProjection
		// this can be simplified so we never have to do a square root for normalization (this looks better when written in 
		//		vec3 projection = bUnitVector * scalarProjection
		//		vec3 projection = b / ||b||	  * a dot (b/||b||)
		//		vec3 projection = b / ||b||	  * (a dot b) / ||b||		//factor around dot product
		//		vec3 projection =     b * (a dot b) / ||b||*||b||		//multiply these two terms
		//		vec3 projection =     b * ((a dot b) / ||b||*||b||)		//recale dot product will product scalar, lump scalars in parenthesis
		//		vec3 projection =     ((a dot b) / ||b||*||b||) * b;	//here b is a vector, so we have scalar * vector;
		//		vec3 projection =     ((a dot b) / (b dot b) * b;	//recall that dot(b,b) == ||b||^2 == ||b||*||b||
		vec3 projection = (glm::dot(a, b) / glm::dot(b, b)) * b;
		return projection;
	}


}