#pragma once

#include <glad/glad.h> //note: sets up OpenGL headers, so should be before anything that uses those headers (such as GLFW)
#include <GLFW/glfw3.h>

#include "math_utils.h"
#include <vector>
#include <optional>
#include "share_ptr_typedefs.h"
#include <iostream>

struct Triangle
{
	glm::vec3 pntA;
	glm::vec3 pntB;
	glm::vec3 pntC;
};

struct Ray
{
	float T = 1.0f;
	glm::vec3 start = glm::vec3(0, 0, 0);
	glm::vec3 dir = glm::vec3(1, 0, 0);
};

namespace RayTests
{
	static float initForIntersectionTests()
	{
		return std::numeric_limits<float>::infinity();
	}

	static bool triangleIntersect(const Ray& ray, const Triangle& tri, glm::vec3& intersectPnt, float& outT)
	{
		//Invariant: this should be a valid triangle such that the cross of its edges does not produce NAN/zerovector results

		using glm::vec3; using glm::vec4; using glm::normalize; using glm::cross; using glm::dot;

		//PARAMETRIC EQ FOR PLANE: F(u,v) : (u*edgeA_n + v*edgeB_n) 
		//IMPLICIT EQ FOR PLANE:   F(pnt) : dot( (pnt - tri.pntA) , triNormal) = 0
		//create parametric equation for plane of triangle
		const vec3 edgeA = vec3(tri.pntA - tri.pntB);
		const vec3 edgeB = vec3(tri.pntA - tri.pntC);
		const vec3 planeNormal_n = normalize(cross(edgeA, edgeB));

		//choose a point in plane that is not the same as the start of the ray
		vec3 planePnt = tri.pntA;
		planePnt = length(planePnt - ray.start) > 0.0001 ? tri.pntB : planePnt;


		// calculate ray T value when it intersects plane triangle is within
		//		notation: @ == dot product; capital_letters == vector; S = rayStart, D = rayDirection, B = pointInPlane,
		//		ray-plane intersection:
		//			N @ ((S - tD) - B) = 0
		//			N@S - N@tD - N@b = 0			//distribute dot
		//			N@tD = -N@S + N@B
		//			t(N@D) = -N@S + N@B				//factor out t
		//			t = (-N@S + N@B) / (N@D)		//divide over dot-product coefficient
		//			t = (N@(-S+B) / N@D)			//factor out N from numerator
		//			t = (N@(B-S) / N@D)				//rearrange 

		//check that ray is not in the plane of the triangle
		vec3 dir_n = normalize(ray.dir);
		vec3 s_to_b = vec3(planePnt) - ray.start;
		bool bRayInPlane = glm::abs(dot(planeNormal_n, dir_n)) < 0.001;
		if (bRayInPlane || s_to_b == vec3(0.f)) return false; //do not consider this a collision

		//calculate t where ray passes through triangle's plane
		float newT = dot(planeNormal_n, s_to_b) / dot(planeNormal_n, dir_n);
		//do not consider larger t values, or negative t values (negative t values will intersect with objects behind camera)
		if (newT > ray.T || newT < 0.0f) return false;
		const vec3 rayPnt = ray.start + newT * dir_n;

		//check if point is within the triangle using crossproduct comparisons with normal
		// X =crossproduct, @=dotproduct
		//   C
		//  /  \      p_out
		// / p   \    
		//A-------B
		//
		//planeNormal = (B-A) X(C-A)
		//
		// verify the below by using right hand rule for crossproducts
		// p_out should return a vector opposite to the normal when compared with edge CB 
		// dot is positive(+) when they're the same, dot is negative(-) when they're opposite
		//
		//edge AB test = ((B-A) X (p-A)) @ (planeNormal) > 0
		//edge BC test = ((C-B) X (p-B)) @ (planeNormal) > 0
		//edge CA test = ((A-C) X (p-C)) @ (planeNormal) > 0
		bool bBA_Correct = dot(cross(tri.pntB - tri.pntA, rayPnt - tri.pntA), planeNormal_n) > 0;
		bool bCB_Correct = dot(cross(tri.pntC - tri.pntB, rayPnt - tri.pntB), planeNormal_n) > 0;
		bool bAC_Correct = dot(cross(tri.pntA - tri.pntC, rayPnt - tri.pntC), planeNormal_n) > 0;
		if (!bBA_Correct || !bCB_Correct || !bAC_Correct)
		{
			return false;
		}

		outT = newT;
		intersectPnt = rayPnt;
		return true;
	}
	
	static std::optional<glm::vec3> rayPlaneIntersection(const Ray& ray, const glm::vec3& planeNormal_n, const glm::vec3& planePoint)
	{
		//this should probably be called from ray triangle intersection test, but wanted to keep logic separate as that function is well documented.
		//to understand the intersection with a plane, look at the first part of that function and read comments to understand. This is doing the same work.
		using glm::vec3; using glm::vec4; using glm::normalize; using glm::cross; using glm::dot;

		//check that ray is not in the plane of the triangle
		vec3 dir_n = normalize(ray.dir);
		vec3 s_to_b = vec3(planePoint) - ray.start;
		bool bRayInPlane = glm::abs(dot(planeNormal_n, dir_n)) < 0.001;
		if (bRayInPlane || s_to_b == vec3(0.f))
		{
			return std::nullopt;
		}

		//calculate t where ray passes through triangle's plane (see notes from ray triangle intersection to understand math)
		float newT = dot(planeNormal_n, s_to_b) / dot(planeNormal_n, dir_n);
		//do not consider larger t values, or negative t values (negative t values will intersect with objects behind camera)
		if (newT > ray.T || newT < 0.0f) return std::nullopt;
		const vec3 rayPnt = ray.start + newT * dir_n;
		return rayPnt;
	}
};

class ObjectPicker
{
public:
	static Ray generateRayFromCamera(
		const glm::vec2& screenResolution, const glm::vec2& clickPoint,
		const glm::vec3& cameraPos_w, const glm::vec3& cameraUp_n, const glm::vec3& cameraRight_n, const glm::vec3& cameraFront_n,
		float FOVy_deg, float aspectRatio
	)
	{
		//convert click point to NDC
		glm::vec2 ndcClick = glm::vec2{
			clickPoint.x / screenResolution.x,
			clickPoint.y / screenResolution.y
		};					//range [0, 1]
		//make this relative to center of screen, not top corner
		//-0.5 is an offset that transforms left-top points to center points (math works out)
		//0.1 - 0.5f = -0.4; 0.6-0.5 = 0.1; 1.0 - 0.5f 0.5f; so this limits the range to [-0.5, 0.5]
		ndcClick += glm::vec2(-0.5f);	//range [-0.5, 0.5]
		ndcClick *= 2.0f;				//range [-1,1]
		ndcClick.y *= -1.0f;
		//       x
		//______________   RENDER PLANE 
		//   |       /
		//   |     /
		// z |FOVx/
		//   |  /
		//   |/
		//   C---------->
		//		(r_v)
		// Z = depth from camera to render plane; known
		// r_v = camera's right vector; known
		// x = scalar for r_v to reach maximum visble area pased on FOV; unknown
		//
		// tan(FOVx) = x / z
		//  z * tan(FOVx) = x
		float hFOVy = glm::radians(FOVy_deg / 2.0f);
		float tan_hFOVy = glm::tan(hFOVy);

		//assuming dist to plane (z) is 1
		float upScalar = tan_hFOVy * ndcClick.y;
		glm::vec3 up = cameraUp_n * upScalar;

		//again, assuming z is 1;
		float rightScalar = aspectRatio * tan_hFOVy * ndcClick.x;
		glm::vec3 right = cameraRight_n * rightScalar;

		//this is relative to the origin, but we can just add cameraPos to get this point in world space
		glm::vec3 click_dir = (up + right + cameraFront_n);

		Ray rayFromCamera;
		rayFromCamera.start = cameraPos_w;
		rayFromCamera.T = std::numeric_limits<float>::infinity();
		rayFromCamera.dir = glm::normalize(click_dir); 
		return rayFromCamera;
	}
};

struct TriangleList
{
	TriangleList(const std::vector<Triangle>& inLocalTriangles)
		: localTriangles(inLocalTriangles),
		worldTriangles(worldTriangles)
	{}
	virtual ~TriangleList(){}
	virtual void transform(const glm::mat4& model)
	{
		if (worldTriangles.size() != localTriangles.size())
		{
			worldTriangles.resize(localTriangles.size());
		}

		for (size_t triIdx = 0; triIdx < localTriangles.size(); ++triIdx)
		{
			worldTriangles[triIdx].pntA = glm::vec3(model * glm::vec4(localTriangles[triIdx].pntA, 1.f));
			worldTriangles[triIdx].pntB = glm::vec3(model * glm::vec4(localTriangles[triIdx].pntB, 1.f));
			worldTriangles[triIdx].pntC = glm::vec3(model * glm::vec4(localTriangles[triIdx].pntC, 1.f));
		}
	}
	std::vector<Triangle> worldTriangles;
	const std::vector<Triangle>& getLocalTriangles() const { return localTriangles; }
private: //private because const will prevent default copy
	std::vector<Triangle> localTriangles;
};

//#TODO remove this if possible
template<typename Owner>
struct OwnedTriangleList : public TriangleList
{
	OwnedTriangleList(const std::vector<Triangle>& inLocalTriangles) : TriangleList(inLocalTriangles) {}
	virtual ~OwnedTriangleList(){}
	virtual void onUpdated() {}
	Owner* owner = nullptr;
};

/** 
	Note this is not necessarily intended to be efficient. It is more of a debug utility. 
	For example, creating/destorying std::vectors on fly has overhead. 
	You can optimize this by reserving the vector and reusing the query struct.
*/
struct CameraRayCastData
{
	std::optional<glm::vec3> camPos = std::nullopt;
	std::optional<glm::vec3> camRight_n = std::nullopt;
	std::optional<glm::vec3> camUp_n = std::nullopt;
	std::optional<glm::vec3> camFront_n = std::nullopt;
	std::optional<float> fovY_deg;
	GLFWwindow* window = nullptr;

	bool validate() const
	{
		if (camPos.has_value() && window && camRight_n && camUp_n && camFront_n && fovY_deg)
		{
			return true;
		}
		else
		{
			std::cerr << "incomplete raycast data" << std::endl;
			return false;
		}
	}
};

struct CameraRayCastData_Triangles : public CameraRayCastData
{
	std::vector<const TriangleList*> objectList;
	bool validate() const
	{
		if (CameraRayCastData::validate())
		{
			if (objectList.size() > 0)
			{
				return true;
			}
			else
			{
				std::cerr << "incomplete raycast data" << std::endl;
				return false;
			}
		}
		return false;
	}
};

struct TriangleObjectRaycastResult
{
	float hitT;
	glm::vec3 hitPoint;
	const TriangleList* hitObject;
	Ray castRay;
};

static std::optional<Ray> rayCast(const CameraRayCastData& rd)
{
	std::optional<Ray> clickRay;
	if (rd.validate())
	{
		int screenWidth = 0, screenHeight = 0;
		double cursorPosX = 0.0, cursorPosY = 0.0;
		glfwGetWindowSize(rd.window, &screenWidth, &screenHeight);
		//glfwGetFramebufferSize(rd.window, &screenWidth, &screenHeight);
		glfwGetCursorPos(rd.window, &cursorPosX, &cursorPosY);
		float windowAspect = float(screenWidth) / screenHeight;

		glm::vec2 mousePos_TopLeft(static_cast<float>(cursorPosX), static_cast<float>(cursorPosY));
		glm::vec2 screenResolution(static_cast<float>(screenWidth), static_cast<float>(screenHeight));

		glm::vec3 rayNudge = (*rd.camRight_n) * 0.0001f; //nude ray to side so visible when debugging

		glm::vec3 camPos = (*rd.camPos) + rayNudge;
		clickRay = ObjectPicker::generateRayFromCamera(
			glm::ivec2(screenWidth, screenHeight), mousePos_TopLeft,
			camPos, *rd.camUp_n, *rd.camRight_n, *rd.camFront_n, *rd.fovY_deg,
			float(windowAspect));
	}
	return clickRay;
}

static std::optional<TriangleObjectRaycastResult> rayCast_TriangleObjects(const CameraRayCastData_Triangles& rd)
{
	if (rd.validate())
	{
		if (std::optional<Ray> clickRay = rayCast(rd))
		{
			float closestDistance2SoFar = std::numeric_limits<float>::infinity();
			const TriangleList* closestPick = nullptr;

			std::optional<float> closestObjT;
			std::optional<glm::vec3> closestObjHitPoint;
			for(const TriangleList* object : rd.objectList)
			{
				std::optional<float> hitT;
				std::optional<glm::vec3> hitPoint;
				for (const Triangle& tri : object->worldTriangles)
				{
					glm::vec3 iterHitPnt;
					float iterT;
					if (RayTests::triangleIntersect(*clickRay, tri, iterHitPnt, iterT))
					{
						if (iterT > 0 && (!hitT || *hitT < iterT))
						{
							hitT = iterT;
							hitPoint = iterHitPnt;
						}
					}
				}

				if (hitT && (!closestObjT || *closestObjT < *hitT))
				{
					closestObjT = hitT;
					closestObjHitPoint = hitPoint;
					closestPick = object;
				}
			}

			//above could use some optimization
			if (closestPick)
			{
				TriangleObjectRaycastResult hitData;
				hitData.hitObject = closestPick;
				hitData.hitPoint = *closestObjHitPoint;
				hitData.hitT = *closestObjT;
				hitData.castRay = *clickRay;
				return hitData;
			}
		}
	}

	return std::nullopt;
}