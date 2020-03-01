#include "InteractableDemo.h"
#include "../new_utils/header_only/ray_utils.h"
#include "../new_utils/header_only/cube_mesh_from_tris.h"


#include <imgui.1.69.gl/imgui.h>
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>

#include <cstdint>
#include "../new_utils/header_only/SceneNode.h"


void SceneNode_TriangleList::setLocalPosition(const glm::vec3& pos)
{
	SceneNode::setLocalPosition(pos);
	transformTriangleList(getWorldMat());
}

void SceneNode_TriangleList::setLocalRotation(const glm::quat newLocalRotQuat)
{
	SceneNode::setLocalRotation(newLocalRotQuat);
	transformTriangleList(getWorldMat());
}

void SceneNode_TriangleList::setLocalScale(const glm::vec3& newScale)
{
	SceneNode::setLocalScale(newScale);
	transformTriangleList(getWorldMat());
}

void SceneNode_TriangleList::setLocalTransform(const ho::Transform& newTransform)
{
	SceneNode::setLocalTransform(newTransform);
	transformTriangleList(getWorldMat());
}

void SceneNode_TriangleList::v_CleanComplete()
{
	SceneNode::v_CleanComplete();
	transformTriangleList(getWorldMat());
}

const TriangleList_SNO& SceneNode_TriangleList::getTriangleList()
{
	if (isDirty())
	{
		requestClean();

		//now that clean has happenedd, update the triangle before returning
		transformTriangleList(getWorldMat());
	}
	return *myTriangleList;
}

void SceneNode_TriangleList::transformTriangleList(const glm::mat4& worldMat)
{
	myTriangleList->transform(worldMat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
//InteractableDemo
// 
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InteractableDemo::InteractableDemo()
{
	++instanceCount;

	//renaming for convenience
	rd = &WindowManager::frameRenderData;
}

InteractableDemo::~InteractableDemo()
{
	--instanceCount;

	if (instanceCount == 0)
	{
		debugCubeRenderer = nullptr;
		lineRenderer = nullptr;
	}
}

void InteractableDemo::render_game(float dt_sec)
{
	if (rd)
	{
		if (bDrawDebugCubes)
		{
			std::vector<const TriangleList_SNO*> objectList;
			gatherInteractableCubeObjects(objectList);
		
			for (const TriangleList_SNO* triList_SNO : objectList)
			{
				if (triList_SNO->owner)
				{
					debugCubeRenderer->render(rd->projection_view, triList_SNO->owner->getWorldMat());
				}
			}
		}

		if (bDebugLastRay && previousRayCast.has_value())
		{
			lineRenderer->renderLine(previousRayCast->start, previousRayCast->start + (previousRayCast->dir * 10.f), glm::vec3(1, 0, 0), rd->projection_view);
		}
	}
}

void InteractableDemo::render_UI(float dt_sec)
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGuiWindowFlags flags = 0;
	ImGui::Begin("Interactable Demo Debug Window", nullptr, flags);
	if (WindowManager::bEnableDebugUI)
	{
		ImGui::Checkbox("draw debug cubes", &bDrawDebugCubes);
		ImGui::Checkbox("debug ray cast", &bDebugLastRay);
	}
	ImGui::End();
}

void InteractableDemo::init()
{


	//same shape is used for all clickable collision, so we can just pick one of the vector's ends.
	debugCubeRenderer = debugCubeRenderer ? debugCubeRenderer : new_sp<nho::TriangleListDebugger>(ho::TriangleCube{}.triangles);
	lineRenderer = lineRenderer ? lineRenderer : new_sp<ho::LineRenderer>();

	instanceCount++;
}

void InteractableDemo::inputPoll(float dt_sec)
{
	if (rd && rd->camera && rd->window)
	{
		if (glfwGetMouseButton(rd->window, selectMouseButton) == GLFW_PRESS )
		{
			if (!bSelectButtonPressed) //must be in a separate branch from above to prevent releasing hold
			{
				bSelectButtonPressed = true;
				if (glfwGetInputMode(rd->window, GLFW_CURSOR))
				{
					CameraRayCastData_Triangles raycastQuery;
					raycastQuery.camFront_n = rd->camera->getFront();
					raycastQuery.camRight_n = rd->camera->getRight();
					raycastQuery.camUp_n = rd->camera->getUp();
					raycastQuery.camPos = rd->camera->getPosition();
					raycastQuery.fovY_deg = glm::degrees<float>(rd->camera->getFOVy_rad());
					raycastQuery.window = rd->window;

					static std::vector<const TriangleList_SNO*> typedObjects;
					static int oneTimeInit = [](std::vector<const TriangleList_SNO*>& typedObjects) {typedObjects.reserve(20); return 0; }(typedObjects);
					typedObjects.clear();

					//gather objects from subclass in type safe way then convert them into form raycast can work with. This means casting will be safe.
					gatherInteractableCubeObjects(typedObjects);
					raycastQuery.objectList.reserve(typedObjects.size());
					for (const TriangleList_SNO* typedTriList : typedObjects)
					{
						raycastQuery.objectList.push_back(typedTriList);
					}

					std::optional<TriangleObjectRaycastResult> raycastResult = rayCast_TriangleObjects(raycastQuery);
					if (raycastResult)
					{
						//fast safe cast because we gathered objets in a typed manner.
						activeClickTarget = static_cast<const TriangleList_SNO*>(raycastResult->hitObject);
						previousRayCast = raycastResult->castRay;
					}
					else
					{
						//regenerate the non-hitting ray for debugging purposes
						previousRayCast = rayCast(raycastQuery);
					}

					glfwGetCursorPos(rd->window, &lastMousePos.x, &lastMousePos.y);
				}
			}
		}
		else
		{
			activeClickTarget = nullptr;
			bSelectButtonPressed = false;
		}
	}
}

void InteractableDemo::tick(float dt_sec)
{
	using namespace glm;
	using std::optional;

	if (rd && rd->camera && rd->window)
	{
		//this will be useful for creating vectors... so not deleting just yet

		//MOVE ALONG PLANE FROM CAMERA
		if (activeClickTarget && activeClickTarget->owner) 
		{
			SceneNode_TriangleList& targetNode = *activeClickTarget->owner;

			vec3 cameraPosition = rd->camera->getPosition();
			vec3 cameraFront = rd->camera->getFront();

			CameraRayCastData raycastQuery;
			raycastQuery.camFront_n = cameraFront;
			raycastQuery.camRight_n = rd->camera->getRight();
			raycastQuery.camUp_n = rd->camera->getUp();
			raycastQuery.camPos = cameraPosition;
			raycastQuery.fovY_deg = glm::degrees<float>(rd->camera->getFOVy_rad());
			raycastQuery.window = rd->window;
			if (optional<Ray> ray = rayCast(raycastQuery))
			{
				vec3 pointOnPlane = targetNode.getWorldPosition();
				const vec3 planeNormal = -cameraFront;

				if (optional<vec3> newPoint = RayTests::rayPlaneIntersection(*ray, planeNormal, pointOnPlane))
				{
					const sp<ho::SceneNode>& parent = targetNode.getParent();
					glm::mat4 localFromWorld = parent ? parent->getInverseWorldMat() : mat4(1.f);

					targetNode.setLocalPosition(localFromWorld * glm::vec4(*newPoint, 1.f));
				}

				previousRayCast = ray;
			}
		}

		//MOVE USING CAMERA BASIS
		//if (activeClickTarget)
		//{
		//	glm::dvec2 newMouse;
		//	glfwGetCursorPos(rd->window, &newMouse.x, &newMouse.y);

		//	glm::dvec2 mouseDelta = newMouse - lastMousePos;
		//	lastMousePos = newMouse; //record last mouse position now that we have created the delta

		//	glm::vec4 moveDelta_ws = glm::vec4(rd->camera->getUp() * float(mouseDelta.y) + rd->camera->getRight() * float(mouseDelta.x), 0);

		//	if (activeClickTarget->owner)
		//	{
		//		SceneNode_TriangleList* node = activeClickTarget->owner;
		//		glm::mat4 localFromWorld = node->getInverseWorldMat();

		//		//convert the world vector into a vector of the object's local space so that translation looks correct with parent node 
		//		//note that I am not set up to test this at time of writing, hopefully it works but expect it to break :) -- #TODO please remove this comment after testing
		//		glm::vec3 moveDelta_ls = glm::vec3(localFromWorld * moveDelta_ws);
		//		node->setLocalPosition(node->getLocalPosition() + moveDelta_ls);
		//	}
		//	else
		//	{
		//		std::cerr << "attempting to move an triangle list object that has no owning scene node" << std::endl;
		//	}
		//}
	}


}

/*static*/ sp<nho::TriangleListDebugger> InteractableDemo::debugCubeRenderer = nullptr;
/*static*/sp<ho::LineRenderer> InteractableDemo::lineRenderer = nullptr;
/*static*/ int32_t InteractableDemo::instanceCount = 0;

