


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 
// DISCLAIMER:
// 
// 
// 
// 
// 
// 
// 
// 
// 
// This demo is to show a concept, not necessarily demonstrate the best practices. A lot of shortcuts
// have been taken in terms of encapsulation and efficiency. Please consider these things  using this 
// as a reference for designing production quality code.
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 


#include "../WindowManager.h"
#include "../../new_utils/header_only/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <imgui.1.69.gl/imgui.h>
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>

#include "../../new_utils/header_only/share_ptr_typedefs.h"
#include "../../new_utils/header_only/static_mesh.h"
#include "../../new_utils/header_only/line_renderer.h"
#include "../../new_utils/header_only/simple_quaternion_camera.h"
#include "../../new_utils/header_only/bitmap_font/Montserrat_BitmapFont.h"
#include "../../new_utils/cpp_required/VisualVector.h"
#include "../../new_utils/header_only/ray_utils.h"
#include "../InteractableDemo.h"
#include "../ClickableVisualVector.h"
#include "../../new_utils/header_only/cube_mesh_from_tris.h"
#include "../DrawVectorDemo.h"

using nho::VisualVector;
using nho::ClickableVisualVector;
using nho::SceneNode_VectorEnd;
using nho::VectorCollisionTriangleList;

namespace ray_tri_ns
{
	//class SlideBase : public InteractableDemo
	class SlideBase : public DrawVectorDemo
	{

	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// demo class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//struct FrameRenderData
	//{
	//	GLFWwindow* window = nullptr;
	//	glm::mat4 view{ 1.f };
	//	glm::mat4 projection{ 1.f };
	//	glm::mat4 projection_view{ 1.f };
	//	float fovY_rad = 0.f;
	//};
	//static FrameRenderData rd;

	class RayTriDemo final : public WindowManager
	{
	public:
		static sp<ho::Montserrat_BMF> font;
		static up<QuaternionCamera> quatCam;
		static up<ho::LineRenderer> lineRenderer;
	public:
		virtual WindowParameters defineWindow();
		virtual void init();
		virtual void inputPoll(float dt_sec);
		virtual void tick(float dt_sec);
		virtual void render_game(float dt_sec);
		virtual void render_UI(float dt_sec);
	private:
		up<ho::Shader> coneTipShader = nullptr;
		sp<ho::TextBlockSceneNode> TestText3D = nullptr;
		float fovY_rad = glm::radians<float>(45.f);
		float near = 0.1f;
		float far = 100.f;

		size_t slideIdx = 0;
		std::vector<sp<SlideBase>> slides;
		std::vector<VisualVector> visualVectors;
	};
	up<ho::LineRenderer> RayTriDemo::lineRenderer = nullptr;
	sp<ho::Montserrat_BMF> RayTriDemo::font = nullptr;
	up<QuaternionCamera> RayTriDemo::quatCam = nullptr;


	/////////////////////////////////////////////////////////////////////////////////////
	// slides
	/////////////////////////////////////////////////////////////////////////////////////

	struct Slide_HighlevelOverview : public SlideBase
	{

	};
	struct Slide_VectorReview : public SlideBase
	{

	};
	struct Slide_RayReview : public SlideBase
	{

	};
	struct Slide_ViewVsRay : public SlideBase
	{

	};

	/////////////////////////////////////////////////////////////////////////////////////
	// dot product review
	/////////////////////////////////////////////////////////////////////////////////////
	struct Slide_DotProductReview : public SlideBase
	{
	public:
		virtual void init() override;
		virtual void inputPoll(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
	private://visuals
		sp<ClickableVisualVector> aVec;
		sp<ClickableVisualVector> bVec;
		sp<ho::TextBlockSceneNode> dotProductValue;
	private://state
		bool bNormalizeVectors = false;
		//const VectorCollisionTriangleList* activeClickTarget = nullptr;
		//sp<nho::TriangleListDebugger> debugRenderer = nullptr;
		//bool bRightCiickPressed = false;
		//std::optional<Ray> previousRayCast;
	};

	struct Slide_CrossProductReview : public SlideBase
	{

	};
	struct Slide_PlaneEquation : public SlideBase
	{

	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// tests
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Slide_TestParentChild : public SlideBase
	{
		sp<SceneNode_TriangleList> grandparent;
		sp<SceneNode_TriangleList> parent;
		sp<SceneNode_TriangleList> child;
		virtual void init() override
		{
			using namespace glm;
			SlideBase::init();

			grandparent = new_sp<SceneNode_TriangleList>(new_sp<TriangleList_SNO>(ho::TriangleCube{}.triangles));
			parent = new_sp<SceneNode_TriangleList>(new_sp<TriangleList_SNO>(ho::TriangleCube{}.triangles));
			child = new_sp<SceneNode_TriangleList>(new_sp<TriangleList_SNO>(ho::TriangleCube{}.triangles));

			grandparent->setLocalPosition(glm::vec3(1, 0, 0));
			grandparent->setLocalScale(glm::vec3(2.0));
			grandparent->setLocalRotation(glm::angleAxis(glm::radians<float>(45.f), glm::vec3(0, 0, 1)));

			parent->setLocalPosition(vec3(0, 1, 0));
			parent->setLocalScale(vec3(0.5f));
			parent->setLocalRotation(glm::angleAxis(glm::radians<float>(45.f), glm::vec3(0, 1, 0)));

			child->setLocalPosition(vec3(0, 0, -1));

			child->setParent(parent);
			parent->setParent(grandparent);
		}
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override
		{
			objectList.push_back(&grandparent->getTriangleList());
			objectList.push_back(&parent->getTriangleList());
			objectList.push_back(&child->getTriangleList());
		}

	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Impl
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	WindowManager::WindowParameters RayTriDemo::defineWindow()
	{
		WindowManager::WindowParameters params;
		params.windowResolution.x = 1000;
		params.windowResolution.y = 600;

		return params;
	}

	void RayTriDemo::init()
	{
		lineRenderer = new_up<ho::LineRenderer>();

		quatCam = new_up<QuaternionCamera>();
		quatCam->pos = glm::vec3(0, 0, 5.f);

		frameRenderData.window = window;
	
		font = new_sp<ho::Montserrat_BMF>("./assets/textures/font/Montserrat_ss_alpha_1024x1024_wb.png");
		TestText3D = new_sp<ho::TextBlockSceneNode>(font, "Testing 3 2 1.");

		//visualVectors.emplace_back();
		//visualVectors[0].setVector(glm::vec3(1.f) * 3.f);

		slides.push_back(new_sp<Slide_DotProductReview>());

		//debug testing
		//slides.push_back(new_sp<Slide_TestParentChild>());

		for (sp<SlideBase>& slide : slides)
		{
			slide->init();
		}
		glEnable(GL_DEPTH_TEST);
	}

	void RayTriDemo::inputPoll(float dt_sec)
	{
		if (slideIdx < slides.size())
		{
			slides[slideIdx]->inputPoll(dt_sec);
		}
	}

	void RayTriDemo::tick(float dt_sec)
	{
		quatCam->tick(dt_sec, window);

		if (slideIdx < slides.size())
		{
			slides[slideIdx]->tick(dt_sec);
		}
	}

	void RayTriDemo::render_game(float dt_sec)
	{
		using namespace glm;

		frameRenderData.view = quatCam->getView();
		frameRenderData.projection = glm::perspective(quatCam->getFOVy_rad(), aspect, near, far);
		frameRenderData.projection_view = frameRenderData.projection * frameRenderData.view;
		frameRenderData.camera = quatCam.get();

		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		vec3 camPos = quatCam->getPosition();

		for (VisualVector& vector : visualVectors)
		{
			vector.render(frameRenderData.projection_view, camPos);
		}

		if (slideIdx < slides.size())
		{
			slides[slideIdx]->render_game(dt_sec);
		}
	}

	void RayTriDemo::render_UI(float dt_sec)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/////////////////////////////////////////////////////////////////////////////////////
		// Overall demo ui
		/////////////////////////////////////////////////////////////////////////////////////
		//{
		//	static float testFloat = 0.f;
		//	ImGui::SetNextWindowPos({ 0,0 });
		//	ImGuiWindowFlags flags = 0;
		//	ImGui::Begin("OpenGL Tweaker (imgui library)", nullptr, flags);
		//	{
		//		//ImGui::SliderFloat("center CP offset divisor", &testFloat, 0.0f, 1.0f);
		//	}
		//	ImGui::End();
		//}

		if (slideIdx < slides.size())
		{
			slides[slideIdx]->render_UI(dt_sec);
		}

		ImGui::EndFrame(); //added this to test decoupling rendering from frame; seems okay
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dot product review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Slide_DotProductReview::init()
	{
		SlideBase::init();

		aVec = new_sp<ClickableVisualVector>();
		bVec = new_sp<ClickableVisualVector>();

		aVec->setVector(glm::vec3(1, 0.f, 0));
		bVec->setVector(glm::vec3(0, 1, 0));

		dotProductValue = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
	}

	void Slide_DotProductReview::inputPoll(float dt_sec)
	{
		SlideBase::inputPoll(dt_sec);
	}

	void Slide_DotProductReview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);

		using namespace glm;
		

		if (bNormalizeVectors)
		{
			aVec->setVector(glm::normalize(aVec->getVec()));
			bVec->setVector(glm::normalize(bVec->getVec()));
		}

	}

	void Slide_DotProductReview::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		glm::vec3 camPos = rd->camera->getPosition();

		aVec->render(rd->projection_view, camPos);
		bVec->render(rd->projection_view, camPos);

		//dotProductValue-> //set text
		float dotProduct = glm::dot(aVec->getVec(), bVec->getVec());

		char textBuffer[128];
		snprintf(textBuffer, sizeof(textBuffer), "%3.3f", dotProduct);
		dotProductValue->wrappedText->text = std::string(textBuffer);

		dotProductValue->setLocalScale(glm::vec3(10.f));
		dotProductValue->setLocalPosition(
			aVec->getStart()
			+ aVec->getVec() + 0.5f * glm::normalize(aVec->getVec())
			+ glm::vec3(0.f, 1.f, 0.f) * 0.5f);
		dotProductValue->render(rd->projection, rd->view);

	}

	void Slide_DotProductReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		ImGui::SetNextWindowPos({ 700, 0 });
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Dot Product Review", nullptr, flags);
		ImGui::Checkbox("force normalization", &bNormalizeVectors);
		ImGui::End();
	}

	void Slide_DotProductReview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&aVec->startCollision->getTriangleList());
		objectList.push_back(&aVec->endCollision->getTriangleList());
		objectList.push_back(&bVec->startCollision->getTriangleList());
		objectList.push_back(&bVec->endCollision->getTriangleList());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void true_main()
{
	ray_tri_ns::RayTriDemo anim;
	anim.start();
}


int main()
{
	true_main();
}