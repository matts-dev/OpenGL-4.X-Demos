


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
#include "../../new_utils/header_only/ImmediateTriangleRenderer.h"
#include "../ClickableVisualPoint.h"
#include "../ClickableVisualRay.h"
#include "../../new_utils/cpp_required/VisualPoint.h"

using nho::VisualVector;
using nho::ClickableVisualVector;
using nho::SceneNode_VectorEnd;
using nho::VectorCollisionTriangleList;

namespace ray_tri_ns
{

	glm::vec3 yellow = glm::vec3(0xff / 255.f, 0xff / 255.f, 0x14 / 255.f);

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
		virtual void init() override;
		virtual void inputPoll(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		void updateRay();
	private:
		bool bRenderTriangleArea = true;
		bool bTriWireFrame = false;
		bool bRenderRay = true;
		bool bRenderEdgeA = false;
		bool bRenderEdgeB = false;
		bool bRenderTriangleNormals = false;
		bool bNormalizeTriXproduct = true;
		bool bRenderThreeNormals = false;
		bool bRenderPlanePnt = false;
		bool bRenderTriPlane = false;
		bool bTransparentPlane = false;

		bool bTestAPoint = false;
		bool bTestBPoint = false;
		bool bTestCPoint = false;
		bool binsideTest_RenderTriEdge = false;
		bool binsideTest_PntXproduct = false;
		bool bInsideTest_DotResult = false;
	
	private:
		sp<ho::ImmediateTriangle> triRender = nullptr;
		sp<nho::ClickableVisualPoint> pntA = nullptr;
		sp<nho::ClickableVisualPoint> pntB = nullptr;
		sp<nho::ClickableVisualPoint> pntC = nullptr;
		sp<nho::VisualVector> vectorRenderer = nullptr;
		sp<nho::VisualPoint> pointRenderer = nullptr;
		sp<ho::PlaneRenderer> planeRenderer = nullptr;
		sp<ho::TextBlockSceneNode> textRenderer = nullptr;
		sp<nho::ClickableVisualRay> ray;
		float yawRad = 0.f, pitchRad = 0.f;
		float targetRayT = 6.f;
	};

	struct Slide_VectorAndPointReview : public SlideBase
	{

	};
	struct Slide_RayReview : public SlideBase
	{
		virtual void init() override;
		virtual void inputPoll(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
	private:
		sp<nho::ClickableVisualRay> ray;
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
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// cross product review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Slide_CrossProductReview : public SlideBase
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
		sp<ClickableVisualVector> crossVecVisual;
		sp<ho::TextBlockSceneNode> crossProductText;
	private://state
		bool bNormalizeVectors = false;
		bool bShowCrossProduct = false;
		bool bShowLength = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Plane Review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Slide_PlaneEquation : public SlideBase
	{
	protected:
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		sp<nho::ClickableVisualVector> planeNormal;
		sp<nho::ClickableVisualPoint> testPoint;
		sp<nho::ClickableVisualVector> vecToPoint;
		sp<ho::PlaneRenderer> planeRenderer;
		sp<ho::TextBlockSceneNode> text_dotProductValue;

		sp<nho::VisualVector> genericVector;
		sp<nho::VisualPoint> genericPoint;
	private:
		bool bRenderPlaneNormal = true;
		bool bRenderPlanePoint = false;
		bool bRenderTestPoint = false;
		bool bRenderVecToPoint = false;
		bool bRenderDotProduct = false;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Live Coding
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct Slide_LiveCoding : public SlideBase
	{
	protected:
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;

		void reviewCode_template();
		void reviewCode_recorded();

		bool codeGroundTruth();
		bool liveCodingIntersection();

		sp<ho::ImmediateTriangle> triRender = nullptr;
		sp<nho::VisualVector> genericVector = nullptr;
		sp<nho::VisualPoint> genericPoint = nullptr;
	private:
		glm::vec3 rayStart{ -100.f };
		glm::vec3 rayEnd{ -100.1f };

	private: //live coding



		//glm::vec3 triPoint_A = glm::vec3(-1, -1, -1);
		//glm::vec3 triPoint_B = glm::vec3(1, -1, -1);
		//glm::vec3 triPoint_C = glm::vec3(0, 1, -1);

		

		glm::vec3 triPoint_A = glm::vec3(-1, -1, -1); //left
		glm::vec3 triPoint_B = glm::vec3( 1, -1, -1); //right
		glm::vec3 triPoint_C = glm::vec3( 0,  1, -1); //top























	};
















	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// generic base slide
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct Slide_AllRenderables : public SlideBase
	{
	public:
		bool bRenderXYZ = false;
	protected:
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
	protected:
		sp<ho::ImmediateTriangle> genericTriangle = nullptr;
		sp<nho::VisualVector> genericVector = nullptr;
		sp<nho::VisualPoint> genericPoint = nullptr;
		sp<ho::TextBlockSceneNode> genericText = nullptr;
	};

	void Slide_AllRenderables::init()
	{
		SlideBase::init();

		genericTriangle = new_sp<ho::ImmediateTriangle>();
		genericVector = new_sp<nho::VisualVector>();
		genericPoint = new_sp<nho::VisualPoint>();
		genericText = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
	}

	////////////////////////////////////////////////////////
	// vector vs ray
	////////////////////////////////////////////////////////
	struct Slide_VectorVsRay : public Slide_AllRenderables
	{
	protected:
		virtual void init() override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
	private:
		bool bLerpToOrigin = true;
		float lerpSpeedSec = 8.f;
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
		params.windowResolution.x = 1400;
		params.windowResolution.y = 900;

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


		slides.push_back(new_sp<Slide_HighlevelOverview>());
		slides.push_back(new_sp<Slide_VectorAndPointReview>());
		slides.push_back(new_sp<Slide_RayReview>());
		slides.push_back(new_sp<Slide_DotProductReview>());
		slides.push_back(new_sp<Slide_CrossProductReview>());
		slides.push_back(new_sp<Slide_PlaneEquation>());
		slides.push_back(new_sp<Slide_VectorVsRay>());
		slides.push_back(new_sp<Slide_LiveCoding>());

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
		{
			static bool bFirstDraw = true;
			if (bFirstDraw)
			{
				bFirstDraw = false;
				ImGui::SetNextWindowPos(ImVec2{ 0, (frameRenderData.fbHeight * 0.9f) });
			}

			ImGuiWindowFlags flags = 0;
			ImGui::Begin("Slides", nullptr, flags);
			{
				if (ImGui::Button("Previous"))
				{
					--slideIdx;
					slideIdx = glm::clamp<size_t>(slideIdx, 0, slides.size() -1);
				}
				ImGui::SameLine();
				if (ImGui::Button("Next"))
				{
					++slideIdx;
					slideIdx = glm::clamp<size_t>(slideIdx, 0, slides.size() -1);
				}
			}
			ImGui::End();
		}

		if (slideIdx < slides.size())
		{
			slides[slideIdx]->render_UI(dt_sec);
		}

		ImGui::EndFrame(); //added this to test decoupling rendering from frame; seems okay
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// High level overview
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Slide_HighlevelOverview::init()
	{
		SlideBase::init();

		triRender = new_sp<ho::ImmediateTriangle>();
		pntA = new_sp<nho::ClickableVisualPoint>();
		pntB = new_sp<nho::ClickableVisualPoint>();
		pntC = new_sp<nho::ClickableVisualPoint>();
		ray = new_sp<nho::ClickableVisualRay>();

		ray->setUseOffsetTipMesh(true);

		pntA->setPosition(glm::vec3(-2, -2, -2));
		pntB->setPosition(glm::vec3(2, -2, -2));
		pntC->setPosition(glm::vec3(0, 2, -2));

		pntA->setUserScale(glm::vec3(3.0f));
		pntB->setUserScale(glm::vec3(3.0f));
		pntC->setUserScale(glm::vec3(3.0f));

		ray->setStartPnt(glm::vec3(1.f, 0, 0.5f));
		yawRad = glm::radians(33.f);
		pitchRad = glm::radians(0.f);

		vectorRenderer = new_sp<VisualVector>();
		vectorRenderer->bUseCenteredMesh = false;
		pointRenderer = new_sp<nho::VisualPoint>();
		planeRenderer = new_sp<ho::PlaneRenderer>();

		textRenderer = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "text.");
		
		updateRay();
	}

	void Slide_HighlevelOverview::inputPoll(float dt_sec)
	{
		SlideBase::inputPoll(dt_sec);
	}

	void Slide_HighlevelOverview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);

		Ray mathRay;
		mathRay.dir = ray->getDirVec();
		mathRay.T = targetRayT;
		mathRay.start = ray->getStartPnt();

		Triangle tri;
		tri.pntA = pntA->getPosition();
		tri.pntB = pntB->getPosition();
		tri.pntC = pntC->getPosition();

		glm::vec3 intersectPnt{ 0.f };
		float adjustedT = std::numeric_limits<float>::infinity();
		RayTests::triangleIntersect(mathRay, tri, intersectPnt, adjustedT);

		adjustedT = glm::clamp<float>(adjustedT, 0.01f, targetRayT);

		ray->setT(adjustedT);

	}

	void Slide_HighlevelOverview::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		std::optional<glm::vec3> camPos;
		if (rd->camera)
		{
			camPos = rd->camera->getPosition();
		}

		if (bRenderTriangleArea)
		{
			if (bTriWireFrame) { ec(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)); }
			triRender->renderTriangle(pntA->getPosition(), pntB->getPosition(), pntC->getPosition(), glm::vec3(0x38/255.f, 0x02/255.f, 0x82/255.f), rd->projection_view);
			if (bTriWireFrame) { ec(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)); }
		}

		if (bRenderRay)
		{
			ray->render(rd->projection_view, camPos);
		}

		pntA->render(rd->projection_view, camPos);
		pntB->render(rd->projection_view, camPos);
		pntC->render(rd->projection_view, camPos);

		const glm::vec3& pA = pntA->getPosition();
		const glm::vec3& pB = pntB->getPosition();
		const glm::vec3& pC = pntC->getPosition();

		glm::vec3 toB = pB - pA;
		glm::vec3 toC = pC - pA;
		glm::vec3 triNormal = glm::cross(toB, toC);
		glm::vec3 triNormal_n = glm::normalize(triNormal);
		if (bNormalizeTriXproduct)
		{
			triNormal = triNormal_n;
		}

		if (bRenderEdgeA)
		{
			vectorRenderer->setStart(pA);
			vectorRenderer->setVector(toB);
			vectorRenderer->color = glm::vec3(1, 0, 0);
			vectorRenderer->render(rd->projection_view, camPos);
		}
		if (bRenderEdgeB)
		{
			vectorRenderer->setStart(pA);
			vectorRenderer->color = glm::vec3(0, 1, 0);
			vectorRenderer->setVector(toC);
			vectorRenderer->render(rd->projection_view, camPos);
		}

		if (bRenderTriangleNormals)
		{
			vectorRenderer->color = glm::vec3(0, 0, 1);
			vectorRenderer->setVector(triNormal);
			vectorRenderer->setStart(pA);
			vectorRenderer->render(rd->projection_view, camPos);
			
			if (bRenderThreeNormals)
			{
				vectorRenderer->setStart(pB);
				vectorRenderer->render(rd->projection_view, camPos);
				vectorRenderer->setStart(pC);
				vectorRenderer->render(rd->projection_view, camPos);
			}
		}

		if (bRenderTriPlane)
		{
			glm::vec3 planePnt = 0.33f*pA + 0.33f*pB + 0.33f*pC;
			planeRenderer->bScreenDoorEffect = bTransparentPlane;
			planeRenderer->renderPlane(planePnt + (-0.05f * triNormal_n), triNormal_n, glm::vec3(10.f), glm::vec4(0.5f, 0.f, 0.f, 1.f), rd->projection_view);
		}

		Ray rayData;
		rayData.start = ray->getStartPnt();
		rayData.dir = ray->getDirVec();
		rayData.T = std::numeric_limits<float>::infinity();

		if (std::optional<glm::vec3> planePnt = RayTests::rayPlaneIntersection(rayData, triNormal_n, pA))
		{
			if (bRenderPlanePnt)
			{
				pointRenderer->setPosition(*planePnt);
				pointRenderer->color = glm::vec3(0.5f, 0.5f, 0.5f);
				pointRenderer->setUserScale(glm::vec3(2.0f));
				pointRenderer->render(rd->projection_view, camPos);

				pointRenderer->setUserScale(glm::vec3(1.0f));
			}

			auto renderPtrTest = [&](const glm::vec3& triPnt, const glm::vec3& triEdge, const glm::vec3& color)
			{
				glm::vec3 triangleToPlanePoint = *planePnt - triPnt;
				vectorRenderer->color = color;
				vectorRenderer->setStart(triPnt);
				vectorRenderer->setVector(triangleToPlanePoint);

				vectorRenderer->render(rd->projection_view, camPos);

				if (binsideTest_RenderTriEdge)
				{
					vectorRenderer->color = glm::vec3(0, 1, 0);
					vectorRenderer->setStart(triPnt);
					vectorRenderer->setVector(triEdge);
					vectorRenderer->render(rd->projection_view, camPos);
				}
				glm::vec3 xProduct = glm::cross(triEdge, triangleToPlanePoint);
				if (bNormalizeTriXproduct) //note: this is shared with normals so becareful before refactoring, perhaps best to split if change
				{
					xProduct = glm::normalize(xProduct);
				}
				if (binsideTest_PntXproduct)
				{

					vectorRenderer->color = glm::vec3(0,0.5f,1.f);
					vectorRenderer->setStart(triPnt + triNormal_n * 0.05f); //add a little of normal so vector can be seen
					vectorRenderer->setVector(xProduct);
					vectorRenderer->render(rd->projection_view, camPos);
				}
				if (bInsideTest_DotResult)
				{
					float comparisonTest = glm::dot(xProduct, triNormal_n);
					textRenderer->wrappedText->text = comparisonTest >= 0.f ? "+" : "-";
					textRenderer->setLocalPosition(triPnt + xProduct + 0.5f*glm::normalize(xProduct));
					textRenderer->setLocalScale(glm::vec3(10.f));
					if (QuaternionCamera* qCam = dynamic_cast<QuaternionCamera*>(rd->camera))
					{
						textRenderer->setLocalRotation(qCam->rotation);
					}
					textRenderer->render(rd->projection, rd->view);
				}
			};

			glm::vec3 color = yellow;
			if (bTestAPoint)
			{
				renderPtrTest(pA, pB - pA, color);
			}
			if (bTestBPoint)
			{
				renderPtrTest(pB, pC - pB, color);
			}
			if (bTestCPoint)
			{
				renderPtrTest(pC, pA - pC, color);
			}
		}


	}

	void Slide_HighlevelOverview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstWindow = true;
		if (bFirstWindow)
		{
			bFirstWindow = !bFirstWindow;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Overview", nullptr, flags);
		{
			float tProxy = ray->getT();
			ImGui::Text("t: %3.3f", tProxy);
			//if (ImGui::SliderFloat("t", &tProxy, 0.1f, 10.f))
			//{
			//	ray->setT(tProxy);
			//}

			if (ImGui::SliderAngle("ray yaw", &yawRad, -90.f, 90.f))
			{
				updateRay();
			}
			if (ImGui::SliderAngle("ray pitch", &pitchRad, -90.f, 90.f))
			{
				updateRay();
			}
			ImGui::Checkbox("Triangle Area", &bRenderTriangleArea);
			ImGui::SameLine();
			ImGui::Checkbox("WireFrame", &bTriWireFrame);
			ImGui::SameLine();
			ImGui::Checkbox("Ray", &bRenderRay);


			ImGui::Checkbox("Triangle Edge A", &bRenderEdgeA);
			ImGui::Checkbox("Triangle Edge B", &bRenderEdgeB);

			ImGui::Checkbox("Triangle Normals", &bRenderTriangleNormals);
			ImGui::SameLine();
			ImGui::Checkbox("normalize", &bNormalizeTriXproduct);
			ImGui::SameLine();
			ImGui::Checkbox("3Normals", &bRenderThreeNormals);

			ImGui::Checkbox("tri plane", &bRenderTriPlane);
			ImGui::SameLine();
			ImGui::Checkbox("transparent", &bTransparentPlane);

			ImGui::Checkbox("ray-plane point", &bRenderPlanePnt);

			ImGui::Checkbox("test A", &bTestAPoint);
			ImGui::SameLine(); ImGui::Checkbox("test B", &bTestBPoint);
			ImGui::SameLine(); ImGui::Checkbox("test C", &bTestCPoint);

			ImGui::Checkbox("inside: tri edge", &binsideTest_RenderTriEdge);
			ImGui::Checkbox("inside: xproduct", &binsideTest_PntXproduct);
			ImGui::Checkbox("inside: dot reuslt", &bInsideTest_DotResult);
				
		}
		ImGui::End();

	}

	void Slide_HighlevelOverview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&pntA->pointCollision->getTriangleList());
		objectList.push_back(&pntB->pointCollision->getTriangleList());
		objectList.push_back(&pntC->pointCollision->getTriangleList());
	}

	void Slide_HighlevelOverview::updateRay()
	{
		glm::vec3 newDir(0, 0, -1);

		glm::quat yawQ = yawRad != 0.f ? glm::angleAxis(yawRad, glm::vec3(0, 1, 0)) : glm::quat(1, 0, 0, 0);
		glm::quat pitchQ = pitchRad != 0.f ? glm::angleAxis(pitchRad, glm::vec3(1, 0, 0)) : glm::quat(1, 0, 0, 0);
		newDir = normalize(yawQ * pitchQ * newDir);

		ray->setDirVec(newDir);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Ray Review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Slide_RayReview::init()
	{
		SlideBase::init();

		ray = new_sp<nho::ClickableVisualRay>();
		ray->setStartPnt(glm::vec3(0, 0, -3));
		ray->setDirVec(glm::normalize(glm::vec3(1, 1, 0)));
	}

	void Slide_RayReview::inputPoll(float dt_sec)
	{
		SlideBase::inputPoll(dt_sec);
	}

	void Slide_RayReview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);

		ray->tick(dt_sec);
	}

	void Slide_RayReview::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		if (rd->camera)
		{
			ray->render(rd->projection_view, rd->camera->getPosition());
		}
	}

	void Slide_RayReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);
		static bool bFirstWindow = true;
		if (bFirstWindow)
		{
			bFirstWindow = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Ray Review", nullptr, flags);
		{
			float tProxy = ray->getT();
			if (ImGui::SliderFloat("t", &tProxy, 0.1f, 10.f))
			{
				ray->setT(tProxy);
			}
		}
		ImGui::End();

	}

	void Slide_RayReview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);
		ray->getCollision(objectList);
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

		if (QuaternionCamera* qCam = dynamic_cast<QuaternionCamera*>(rd->camera))
		{
			dotProductValue->setLocalRotation(qCam->rotation);
		}
		dotProductValue->render(rd->projection, rd->view);

	}

	void Slide_DotProductReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// cross product review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Slide_CrossProductReview::init()
	{
		SlideBase::init();

		aVec = new_sp<ClickableVisualVector>();
		bVec = new_sp<ClickableVisualVector>();
		crossVecVisual = new_sp<ClickableVisualVector>();

		aVec->setVector(glm::vec3(1, 0.f, 0));
		bVec->setVector(glm::vec3(0, 1, 0));
		crossVecVisual->setVector(glm::cross(aVec->getVec(), bVec->getVec()));
		crossVecVisual->color = glm::vec3(0, 0, 1);

		crossProductText = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
	}

	void Slide_CrossProductReview::inputPoll(float dt_sec)
	{
		SlideBase::inputPoll(dt_sec);
	}

	void Slide_CrossProductReview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);

		using namespace glm;

		//just ignore any color setting by selection so that it is always red x green = blue
		aVec->color = glm::vec3(1, 0, 0);
		bVec->color = glm::vec3(0, 1, 0);
		if (bNormalizeVectors)
		{
			aVec->setVector(glm::normalize(aVec->getVec()));
			bVec->setVector(glm::normalize(bVec->getVec()));
		}
	}

	void Slide_CrossProductReview::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		glm::vec3 camPos = rd->camera->getPosition();

		aVec->render(rd->projection_view, camPos);
		bVec->render(rd->projection_view, camPos);

		if (bShowCrossProduct)
		{
			glm::vec3 crossVec = glm::cross(aVec->getVec(), bVec->getVec());
			crossVecVisual->setVector(crossVec);
			float crossLength = glm::length(crossVec);

			char textBuffer[128];
			snprintf(textBuffer, sizeof(textBuffer), "length %3.3f", crossLength);
			crossProductText->wrappedText->text = std::string(textBuffer);

			crossProductText->setLocalScale(glm::vec3(10.f));
			crossProductText->setLocalPosition(
				crossVecVisual->getStart()
				+ crossVecVisual->getVec() + 0.5f * glm::normalize(crossVecVisual->getVec())
			);
			if (QuaternionCamera* qCam = dynamic_cast<QuaternionCamera*>(rd->camera))
			{
				crossProductText->setLocalRotation(qCam->rotation);
			}

			if (bShowLength)
			{
				crossProductText->render(rd->projection, rd->view);
			}

			crossVecVisual->render(rd->projection_view, camPos);
		}
	}

	void Slide_CrossProductReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Cross Product Review", nullptr, flags);
		ImGui::Checkbox("force normalization", &bNormalizeVectors);
		ImGui::Checkbox("show crossproduct", &bShowCrossProduct);
		ImGui::Checkbox("show value", &bShowLength);
		ImGui::End();
	}

	void Slide_CrossProductReview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&aVec->startCollision->getTriangleList());
		objectList.push_back(&aVec->endCollision->getTriangleList());
		objectList.push_back(&bVec->startCollision->getTriangleList());
		objectList.push_back(&bVec->endCollision->getTriangleList());
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plane review
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Slide_PlaneEquation::init()
	{
		SlideBase::init();

		planeNormal = new_sp<nho::ClickableVisualVector>();
		planeNormal->color = glm::vec3(0.2f, 0.2f, 1.f);
		planeNormal->setStart(glm::vec3(0, 0, 0));
		planeNormal->setVector(glm::normalize(glm::vec3(1, 1, 0.2)));

		testPoint = new_sp<nho::ClickableVisualPoint>();
		testPoint->color = glm::vec3(0, 1, 0);
		testPoint->setPosition(glm::vec3(2, 1, -1));
		testPoint->setUserScale(glm::vec3(3));

		vecToPoint = new_sp<nho::ClickableVisualVector>();
		vecToPoint->setStart(planeNormal->getStart());
		vecToPoint->setVector(testPoint->getPosition() - planeNormal->getStart());
		vecToPoint->color = yellow;
		vecToPoint->bUseCenteredMesh = false;

		genericVector = new_sp<VisualVector>();
		genericPoint = new_sp<nho::VisualPoint>();

		planeRenderer = new_sp<ho::PlaneRenderer>();

		text_dotProductValue = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
	}

	void Slide_PlaneEquation::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		objectList.push_back(&planeNormal->endCollision->getTriangleList());
		objectList.push_back(&planeNormal->startCollision->getTriangleList());

		objectList.push_back(&testPoint->pointCollision->getTriangleList());
	}

	void Slide_PlaneEquation::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);
		if (rd->camera)
		{
			glm::vec3 camPos = rd->camera->getPosition();

			planeRenderer->renderPlane(planeNormal->getStart(), planeNormal->getVec(), glm::vec3(5.f), glm::vec4(0.5f, 0, 0, 1.f), rd->projection_view);

			if (bRenderPlaneNormal)
			{
				planeNormal->render(rd->projection_view, camPos);
			}

			if (bRenderPlanePoint)
			{
				genericPoint->setPosition(planeNormal->getStart());
				genericPoint->setUserScale(glm::vec3(3.0f));
				genericPoint->color = planeNormal->color;
				genericPoint->render(rd->projection_view, camPos);
			}

			if (bRenderTestPoint)
			{
				testPoint->render(rd->projection_view, camPos);
			}

			if (bRenderVecToPoint)
			{
				vecToPoint->render(rd->projection_view, camPos);
			}

			if (bRenderDotProduct)
			{
				text_dotProductValue->setLocalPosition(
					vecToPoint->getStart()
					+ vecToPoint->getVec() + 0.5f * glm::normalize(vecToPoint->getVec())
					//+ glm::vec3(0.f, 1.f, 0.f) * 0.5f
				);

				if (QuaternionCamera* qCam = dynamic_cast<QuaternionCamera*>(rd->camera))
				{
					text_dotProductValue->setLocalRotation(qCam->rotation);
				}

				text_dotProductValue->render(rd->projection, rd->view);
			}
		}
	}

	void Slide_PlaneEquation::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Plane Review", nullptr, flags);
		{
			ImGui::Checkbox("plane normal", &bRenderPlaneNormal);
			ImGui::Checkbox("plane point", &bRenderPlanePoint);

			ImGui::Checkbox("test point", &bRenderTestPoint);
			ImGui::Checkbox("vector to test point", &bRenderVecToPoint);
			ImGui::Checkbox("dot product", &bRenderDotProduct);
		}
		ImGui::End();

	}


	void Slide_PlaneEquation::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);

		planeNormal->color = glm::vec3(0, 0, 1);

		vecToPoint->setStart(planeNormal->getStart());
		vecToPoint->setVector(testPoint->getPosition() - planeNormal->getStart());

		//must come afte we've set vecToPoint for it to be latest values
		float dotProduct = glm::dot(glm::normalize(planeNormal->getVec()), vecToPoint->getVec());
		char textBuffer[128];
		snprintf(textBuffer, sizeof(textBuffer), "%3.3f", dotProduct);
		text_dotProductValue->wrappedText->text = std::string(textBuffer);
		text_dotProductValue->setLocalScale(glm::vec3(5.f));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// live coding
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Slide_LiveCoding::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

	}

	void Slide_LiveCoding::init()
	{
		SlideBase::init();

		triRender = new_sp<ho::ImmediateTriangle>();
		genericVector = new_sp<nho::VisualVector>();
		genericVector->bUseCenteredMesh = false;
		genericPoint = new_sp<nho::VisualPoint>();

	}

	void Slide_LiveCoding::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		triRender->renderTriangle(triPoint_A, triPoint_B, triPoint_C, glm::vec3(1.f,0,0) , rd->projection_view);

		if (rd->camera)
		{
			genericVector->setStart(rayStart);
			genericVector->setEnd(rayEnd);
			genericVector->render(rd->projection_view, rd->camera->getPosition());
		}
	}

	void Slide_LiveCoding::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Live Coding", nullptr, flags);
		{
			if (ImGui::Button("run ray tri intersection"))
			{
				//bool bGroundTruthHit = codeGroundTruth();
				//if (bGroundTruthHit)
				//{
				//	std::cout << "hit the triangle" << std::endl;
				//}
				//else
				//{
				//	std::cout << "missed" << std::endl;
				//}










				if (liveCodingIntersection())
				{
					std::cout << "hit the triangle" << std::endl;
				}
				else
				{
					std::cout << "missed" << std::endl;
				}







			}
		}
		ImGui::End();

	}

	void Slide_LiveCoding::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);
	}

	void Slide_LiveCoding::reviewCode_template()
	{
		using namespace glm;

		////////////////////////////////////////////////////////
		// points review
		////////////////////////////////////////////////////////

		vec3 pointA;
		pointA.x = 1;
		pointA.y = -2;
		pointA.z = 3;

		vec3 pointB{ -1, 2, 3 };

		////////////////////////////////////////////////////////
		// vector review
		////////////////////////////////////////////////////////

		const vec3 xDir(1, 0, 0);
		const vec3 yDir = vec3(0, 1, 0);
		const vec3 zDir{ 0,0,1 };

		//you can create vectors/direction from points
		vec3 A_to_B = pointA - pointB;

		float LengthAB = glm::length(A_to_B);

		vec3 ab_normalized = A_to_B / LengthAB;

		vec3 ab_normalized_2 = glm::normalize(A_to_B);

		////////////////////////////////////////////////////////
		// dot product review
		////////////////////////////////////////////////////////

		float dotProduct = glm::dot(A_to_B, xDir);

		////////////////////////////////////////////////////////
		// cross product review
		////////////////////////////////////////////////////////

		vec3 x_cross_y = glm::cross(xDir, yDir);

		////////////////////////////////////////////////////////
		// ray review
		////////////////////////////////////////////////////////
		struct Ray
		{
			vec3 startPoint;
			vec3 rayDirection;
			float t;
		};

		Ray myRay;
		myRay.startPoint = vec3(1, 3, 0);
		myRay.rayDirection = zDir;
		myRay.t = 5;

		////////////////////////////////////////////////////////
		// plane review
		////////////////////////////////////////////////////////
		struct Plane
		{
			vec3 aPointOnPlane;
			vec3 normal_n;
		};
		Plane myPlane;
		myPlane.aPointOnPlane = vec3(3, 3, -3);
		myPlane.aPointOnPlane = yDir;

		////////////////////////////////////////////////////////
		// plane test
		////////////////////////////////////////////////////////

		vec3 testPoint = vec3(5, 5, 5);

		vec3 planeToPoint = testPoint - myPlane.aPointOnPlane;

		float planeTestVal = glm::dot(planeToPoint, myPlane.normal_n);
		//if (planeTestVal == 0.f)
		if(glm::abs(planeTestVal) < 0.001f)
		{
			//on the plane!
		}

	}



	void Slide_LiveCoding::reviewCode_recorded()
	{
		using namespace glm;

		////////////////////////////////////////////////////////
		// point review
		////////////////////////////////////////////////////////

		vec3 pointA;
		pointA.x = 1;
		pointA.y = -2;
		pointA.z = 3;

		vec3 pointB{ -1,2,3.f };

		////////////////////////////////////////////////////////
		// vector review
		////////////////////////////////////////////////////////

		const vec3 xDir = vec3(1, 0, 0);
		const vec3 yDir(0, 1, 0);
		const vec3 zDir{ 0,0,1 };


		vec3 A_FROM_B = pointA - pointB;

		float abLength = glm::length(A_FROM_B);

		vec3 abNormalized = A_FROM_B / abLength;

		vec3 abNormalized2 = glm::normalize(A_FROM_B);


		////////////////////////////////////////////////////////
		// dot product review
		////////////////////////////////////////////////////////















		////////////////////////////////////////////////////////
		// dot product review
		////////////////////////////////////////////////////////

		float dotProductResult = glm::dot(A_FROM_B, xDir);


		////////////////////////////////////////////////////////
		// cross product review
		////////////////////////////////////////////////////////
		
		vec3 x_cross_y = glm::cross(xDir, yDir);



		////////////////////////////////////////////////////////
		// Ray review
		////////////////////////////////////////////////////////

		struct Ray
		{
			vec3 startPoint;
			vec3 rayDirection;
			float t;
		};

		Ray myRay;
		myRay.startPoint = vec3(1, 3, 0);
		myRay.rayDirection = zDir;
		myRay.t = 5;

		vec3 pointFromRay = myRay.startPoint + (myRay.rayDirection * myRay.t);



		////////////////////////////////////////////////////////
		// plane review
		////////////////////////////////////////////////////////

		struct Plane
		{
			vec3 pointOnPlane;
			vec3 normal;
		};

		Plane myPlane;
		myPlane.pointOnPlane = vec3(1, 2, 0);
		myPlane.normal = yDir;


		////////////////////////////////////////////////////////
		// plane test
		////////////////////////////////////////////////////////

		vec3 testPoint = vec3(2, 2, 0);

		vec3 vecToTestPoint = testPoint - myPlane.pointOnPlane;

		float testResult = glm::dot(vecToTestPoint, myPlane.normal);

		if(glm::abs(testResult) < 0.0001f)
		{
			//on the plane!
		}


	}






















	bool Slide_LiveCoding::codeGroundTruth()
	{
		using namespace glm;
		struct Ray
		{
			vec3 startPoint;
			vec3 rayDirection;
			float t;
		};
		struct Plane
		{
			vec3 pointOnPlane;
			vec3 normal;
		};

		vec3 camPos = rd->camera->getPosition();
		vec3 camDir = rd->camera->getFront();

		Ray camRay;
		camRay.startPoint = camPos;
		camRay.rayDirection = camDir;
		camRay.t = -1;

		vec3 triEdge1 = triPoint_B - triPoint_A;
		vec3 triEdge2 = triPoint_C - triPoint_A;
		vec3 triFlatNormal = glm::cross(triEdge1, triEdge2);

		Plane triPlane;
		triPlane.pointOnPlane = triPoint_A;
		triPlane.normal = triFlatNormal;

		//calculate t value where ray hits plane
		float n_dot_d = glm::dot(triPlane.normal, camRay.rayDirection);

		if (glm::abs(n_dot_d) < 0.00001f)
		{
			//avoid divide by zero, ray must be running along the plane
			return false;
		}
		float n_dot_ps = glm::dot(triPlane.normal, (triPlane.pointOnPlane - camRay.startPoint));
		camRay.t = n_dot_ps / n_dot_d;

		//generate a point from the t value we found
		vec3 planePoint = camRay.startPoint + camRay.t * camRay.rayDirection;
		
		//test point a
		vec3 AtoB_Edge = triPoint_B - triPoint_A;
		vec3 BtoC_Edge = triPoint_C - triPoint_B;
		vec3 CtoA_edge = triPoint_A - triPoint_C;

		vec3 AtoPnt = planePoint - triPoint_A;
		vec3 BtoPnt = planePoint - triPoint_B;
		vec3 CtoPnt = planePoint - triPoint_C;

		vec3 ATestVec = glm::cross(AtoB_Edge, AtoPnt);
		vec3 BTestVec = glm::cross(BtoC_Edge, BtoPnt);
		vec3 CTestVec = glm::cross(CtoA_edge, CtoPnt);

		bool ATestVec_MatchesNormal = glm::dot(ATestVec, triFlatNormal) > 0.f;
		bool BTestVec_MatchesNormal = glm::dot(BTestVec, triFlatNormal) > 0.f;
		bool CTestVec_MatchesNormal = glm::dot(CTestVec, triFlatNormal) > 0.f;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// don't show this part in video
		rayStart = camRay.startPoint;
		rayEnd = planePoint;
		if (ATestVec_MatchesNormal && BTestVec_MatchesNormal && CTestVec_MatchesNormal)
		{
		}
		else
		{
			rayEnd = camRay.startPoint + camRay.rayDirection * 10.f;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		return ATestVec_MatchesNormal && BTestVec_MatchesNormal && CTestVec_MatchesNormal;
	}









	bool Slide_LiveCoding::liveCodingIntersection()
	{
		using namespace glm;
		struct Ray
		{
			vec3 startPoint;
			vec3 rayDirection;
			float t;
		};
		struct Plane
		{
			vec3 pointOnPlane;
			vec3 normal;
		};

		vec3 cameraPosition = rd->camera->getPosition();
		vec3 cameraDirection = rd->camera->getFront();





		Ray camRay;
		camRay.startPoint = cameraPosition;
		camRay.rayDirection = cameraDirection;
		camRay.t = -1;

		vec3 triEdge1 = triPoint_B - triPoint_A;
		vec3 triEdge2 = triPoint_C - triPoint_A;
		vec3 triFlatNormal = glm::cross(triEdge1, triEdge2);

		Plane triPlane;
		triPlane.normal = triFlatNormal;
		triPlane.pointOnPlane = triPoint_A;

		//calculate t value when ray hits plane
		float n_dot_d = glm::dot(triPlane.normal, camRay.rayDirection);
		if (glm::abs(n_dot_d) < 0.0001f)
		{
			return false;
		}

		float n_dot_ps = glm::dot(triPlane.normal, triPlane.pointOnPlane - camRay.startPoint);
		camRay.t = n_dot_ps / n_dot_d;

		vec3 planePoint = camRay.startPoint + camRay.t * camRay.rayDirection;
		
		//test if plane point is within the triangle
		vec3 AtoB_edge = triPoint_B - triPoint_A;
		vec3 BtoC_edge = triPoint_C - triPoint_B;
		vec3 CtoA_edge = triPoint_A - triPoint_C;

		vec3 AtoPoint = planePoint - triPoint_A;
		vec3 BtoPoint = planePoint - triPoint_B;
		vec3 CtoPoint = planePoint - triPoint_C;

		vec3 ATestVec = glm::cross(AtoB_edge, AtoPoint);
		vec3 BTestVec = glm::cross(BtoC_edge, BtoPoint);
		vec3 CTestVec = glm::cross(CtoA_edge, CtoPoint);

		bool ATestVec_MatchesNormal = glm::dot(ATestVec, triFlatNormal) > 0.f;
		bool BTestVec_MatchesNormal = glm::dot(BTestVec, triFlatNormal) > 0.f;
		bool CTestVec_MatchesNormal = glm::dot(CTestVec, triFlatNormal) > 0.f;

		bool hitTriangle = ATestVec_MatchesNormal && BTestVec_MatchesNormal && CTestVec_MatchesNormal;
		//return hitTriangle;





		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// don't show this part in video, hooking up graphics!
		rayStart = camRay.startPoint;
		rayEnd = planePoint;
		if (ATestVec_MatchesNormal && BTestVec_MatchesNormal && CTestVec_MatchesNormal)
		{
		}
		else
		{
			rayEnd = camRay.startPoint + camRay.rayDirection * 10.f;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////










		return hitTriangle;










	}























	////////////////////////////////////////////////////////
	// all renderables base
	////////////////////////////////////////////////////////
	void Slide_AllRenderables::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		if (bRenderXYZ && rd)
		{
			glm::vec3 cachedColor = genericText->wrappedText->bitMapFont->getFontColor();
			glm::quat cachedRotation = genericText->getLocalRotation();
			glm::vec3 cachedScale = genericText->getLocalScale();
			std::optional<glm::vec3> camPos = rd->camera ? rd->camera->getPosition() : glm::vec3(0.f);
			std::optional<glm::quat> rotQuat = std::nullopt;
			if (QuaternionCamera* qCam = dynamic_cast<QuaternionCamera*>(rd->camera))
			{
				rotQuat = qCam->rotation;
			}
			float textOffsetScalar = 1.3f;
			float brightenTextScalar = 0.25f;
			genericVector->setStart(glm::vec3(0.f));
			genericText->setLocalScale(glm::vec3(8.f));
			if (rotQuat) { genericText->setLocalRotation(*rotQuat); }

			genericVector->color = glm::vec3(1, 0, 0);
			genericVector->setVector(genericVector->color);
			genericVector->render(rd->projection_view, camPos);
			genericText->wrappedText->text = "x";
			genericText->setLocalPosition(genericVector->getVec()*textOffsetScalar);
			genericText->wrappedText->bitMapFont->setFontColor(genericVector->color + glm::vec3(brightenTextScalar));
			genericText->render(rd->projection, rd->view);

			genericVector->color = glm::vec3(0, 1, 0);
			genericVector->setVector(genericVector->color);
			genericVector->render(rd->projection_view, camPos);
			genericText->wrappedText->text = "y";
			genericText->setLocalPosition(genericVector->getVec()*textOffsetScalar);
			genericText->wrappedText->bitMapFont->setFontColor(genericVector->color + glm::vec3(brightenTextScalar));
			genericText->render(rd->projection, rd->view);

			genericVector->color = glm::vec3(0, 0, 1);
			genericVector->setVector(genericVector->color);
			genericVector->render(rd->projection_view, camPos);
			genericText->wrappedText->text = "z";
			genericText->setLocalPosition(genericVector->getVec() * textOffsetScalar);
			genericText->wrappedText->bitMapFont->setFontColor(genericVector->color + glm::vec3(brightenTextScalar));
			genericText->render(rd->projection, rd->view);

			genericText->wrappedText->bitMapFont->setFontColor(cachedColor);
			genericText->setLocalScale(cachedScale);
			if (rotQuat) { genericText->setLocalRotation(cachedRotation); }
		}
	}


	////////////////////////////////////////////////////////
	// vector vs ray
	////////////////////////////////////////////////////////

	void Slide_VectorVsRay::init()
	{
		Slide_AllRenderables::init();
		bRenderXYZ = true;
	}

	void Slide_VectorVsRay::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		Slide_AllRenderables::gatherInteractableCubeObjects(objectList);
	}

	void Slide_VectorVsRay::render_game(float dt_sec)
	{
		Slide_AllRenderables::render_game(dt_sec);
	}

	void Slide_VectorVsRay::render_UI(float dt_sec)
	{
		Slide_AllRenderables::render_UI(dt_sec);


		SlideBase::render_UI(dt_sec);
		static bool bFirstWindow = true;
		if (bFirstWindow)
		{
			bFirstWindow = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Vector vs Ray Review", nullptr, flags);
		{
			ImGui::Checkbox("Interpolate To Origin", &bLerpToOrigin);
		}
		ImGui::End();

	}

	void Slide_VectorVsRay::tick(float dt_sec)
	{
		using namespace glm;

		Slide_AllRenderables::tick(dt_sec);

		if (bLerpToOrigin)
		{
			for (sp<nho::ClickableVisualVector>& vec : customVectors)
			{
				if (vec != newVector)
				{
					vec3 start = vec->getStart();
					vec3 toOri = vec3(0.f) - start;

					float len = glm::length(toOri);

					vec3 newStart = start;
					if (len > 0.01f)
					{
						toOri = toOri / len; //normalize

						float lerpSpeedThisTick = glm::min(lerpSpeedSec * dt_sec, len);
						newStart = start + (toOri * lerpSpeedThisTick);
					}
					else
					{
						newStart = vec3(0.f);
					}
					vec->setStart(newStart);
				}
			}
		}
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