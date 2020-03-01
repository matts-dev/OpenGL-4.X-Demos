#include "ClickableVisualVector.h"
#include "../new_utils/header_only/cube_mesh_from_tris.h"
#include "../new_utils/header_only/opengl_debug_utils.h"
#include "../new_utils/header_only/shader.h"

namespace nho
{

	VectorCollisionTriangleList::VectorCollisionTriangleList()
		: TriangleList_SNO(ho::TriangleCube{}.triangles)
	{
	}

	SceneNode_VectorEnd::SceneNode_VectorEnd() 
		: SceneNode_TriangleList(new_sp<VectorCollisionTriangleList>())
	{	
		myTriangleBox = static_cast<VectorCollisionTriangleList*>(myTriangleList.get());
		assert(myTriangleBox);
	}

	void ClickableVisualVector::sharedInit()
	{
		startCollision = new_sp<SceneNode_VectorEnd>();
		endCollision = new_sp<SceneNode_VectorEnd>();
	}

	void ClickableVisualVector::onValuesUpdated(const VisualVector::POD& values)
	{
		//prevent recursion since scene node responds to vector updates. Don't have scene nodes update when we're retro-applying an update from the scene node itself (eg clicking and dragging a vector)
		if (!bUpdatingFromSceneNode)
		{
			ho::Transform xform;
			xform.scale = glm::vec3(0.25f);
			xform.position = values.startPos;
			startCollision->setLocalTransform(xform);

			xform.position = values.startPos + values.dir;
			endCollision->setLocalTransform(xform);
		}
	}


	void ClickableVisualVector::postConstruct()
	{
		startCollision->dirtyEvent.addWeakObj(sp_this(), &ClickableVisualVector::handleStartDirty);
		endCollision->dirtyEvent.addWeakObj(sp_this(), &ClickableVisualVector::handleEndDirty);

		startCollision->updatedEvent.addWeakObj(sp_this(), &ClickableVisualVector::handleStartCollisionUpdated);
		endCollision->updatedEvent.addWeakObj(sp_this(), &ClickableVisualVector::handleEndCollisionUpdated);
	}


	void ClickableVisualVector::handleStartDirty()
	{
		startCollision->requestClean();
	}

	void ClickableVisualVector::handleEndDirty()
	{
		endCollision->requestClean();
	}

	void ClickableVisualVector::handleStartCollisionUpdated()
	{
		bUpdatingFromSceneNode = true;
		setStart(startCollision->getWorldPosition());
		endCollision->setLocalPosition(getStart() + getVec());
		bUpdatingFromSceneNode = false;
	}

	void ClickableVisualVector::handleEndCollisionUpdated()
	{
		bUpdatingFromSceneNode = true;
		setEnd(endCollision->getWorldPosition());
		bUpdatingFromSceneNode = false;
	}

	ClickableVisualVector::ClickableVisualVector()
	{
		sharedInit();
	}

	ClickableVisualVector::ClickableVisualVector(const ClickableVisualVector& copy)
	{
		if (&copy != this)
		{
			sharedInit();
			startCollision->setLocalTransform(copy.startCollision->getLocalTransform());
			endCollision->setLocalTransform(copy.endCollision->getLocalTransform());
		}
	}

	ClickableVisualVector& ClickableVisualVector::operator=(const ClickableVisualVector& copy)
	{
		if (&copy != this)
		{
			sharedInit();
			startCollision->setLocalTransform(copy.startCollision->getLocalTransform());
			endCollision->setLocalTransform(copy.endCollision->getLocalTransform());
		}
		return *this;
	}

	//ClickableVisualVector::ClickableVisualVector(ClickableVisualVector&& move)
	//{
	//	sharedInit();
	//}

	//ClickableVisualVector& ClickableVisualVector::operator=(ClickableVisualVector&& move)
	//{
	//	sharedInit();
	//}



}
