#pragma once
#include <cstdint>
#include <glm/gtc/type_precision.hpp>

#include"WindowManager.h"

#include "../new_utils/header_only/ICamera.h"
#include "../new_utils/header_only/share_ptr_typedefs.h"
#include "../new_utils/header_only/ray_utils.h"
#include "../new_utils/header_only/SceneNode.h"
#include "../new_utils/cpp_required/TriangleListDebugger.h"
#include "../new_utils/header_only/line_renderer.h"
#include "../new_utils/header_only/plane_renderer.h"

class SceneNode_TriangleList;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TriangleList_SNO - Triangle list managed by a scene node for the benefits a scene node provides
//	SNO = scene node owned
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct TriangleList_SNO : public TriangleList
{
	TriangleList_SNO(const std::vector<Triangle>& inLocalTriangles) : TriangleList(inLocalTriangles) {}
	virtual ~TriangleList_SNO() {}

	SceneNode_TriangleList* owner = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TriangleListSceneNode  - a scene node that managed a triangle list that can be detected with ray casting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SceneNode_TriangleList : public ho::SceneNode
{
public:
	SceneNode_TriangleList(const sp<TriangleList_SNO>& inTriangleList)
		: ho::SceneNode(), myTriangleList(inTriangleList)
	{	
		assert(myTriangleList);
		myTriangleList->owner = this;
	}
	virtual void setLocalPosition(const glm::vec3& pos) override;
	virtual void setLocalRotation(const glm::quat newLocalRotQuat) override;
	virtual void setLocalScale(const glm::vec3& newScale) override;
	virtual void setLocalTransform(const ho::Transform& newTransform) override;
	virtual void v_CleanComplete() override;
	const TriangleList_SNO& getTriangleList();
private:
	void transformTriangleList(const glm::mat4& worldMat);
protected:
	sp<TriangleList_SNO> myTriangleList = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InteractableDemo - A demo that has built in support for mouse interaction of moving scene node based objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InteractableDemo : public IDemoInterface
{
public:
	InteractableDemo();
	InteractableDemo(const InteractableDemo& copy) = delete;
	InteractableDemo(InteractableDemo&& move) = delete;
	InteractableDemo& operator=(const InteractableDemo& copy) = delete;
	InteractableDemo& operator=(InteractableDemo&& move) = delete;
	virtual ~InteractableDemo();
public: //api
	virtual void init() override;
	virtual void render_game(float dt_sec) override;
	virtual void render_UI(float dt_sec) override;
	virtual void inputPoll(float dt_sec)override;
	virtual void tick(float dt_sec)override;
public:
	std::optional<glm::vec3> getDrawLineStart();
	std::optional<glm::vec3> getDrawLineEnd();
	std::optional<glm::vec3> getDrawPoint();
protected:
	/** override this to provide the list of objects you want to be tested when ray casting*/
	virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) = 0;
public:
	uint32_t selectMouseButton = GLFW_MOUSE_BUTTON_RIGHT;
protected://state
	const TriangleList_SNO* activeClickTarget = nullptr; //#TODO perhaps this would be cleaner to just cache the scene node?
	std::optional<Ray> previousRayCast;
	WindowManager::FrameRenderData* rd = nullptr; 
public://debug
	bool bDrawDebugCubes = false;
	bool bDebugLastRay = false;
	bool bDrawInteractionPlane = false;
protected:
	bool bRenderLineGeneration = true;
	float lineCreationDistFromCamera = 5.0f;
	float lineCreationDistAdjustSpeedSec = 5.0f;
private: //statics
	static sp<nho::TriangleListDebugger> debugCubeRenderer;
	static sp<ho::LineRenderer> lineRenderer;
	static sp<ho::PlaneRenderer> planeRenderer;
	static int32_t instanceCount;
private:
	bool bSelectButtonPressed = false;
	glm::dvec2 lastMousePos{ 0.f,0.f };
private: //implementation details
	std::vector<sp<ho::SceneNode>> nodePool;
	ho::SceneNode* start_linePnt= nullptr;
	ho::SceneNode* end_linePnt = nullptr;
	ho::SceneNode* customPoint = nullptr;
};