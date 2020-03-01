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
		ho::Transform xform;
		xform.scale = glm::vec3(0.25f);
		xform.position = values.startPos;
		startCollision->setLocalTransform(xform);

		xform.position = values.startPos + values.dir;
		endCollision->setLocalTransform(xform);
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
