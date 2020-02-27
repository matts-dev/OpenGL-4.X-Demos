


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


#include "../VideoDemoHelperBase.h"
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
#include "../../new_utils/cpp_required/ClickableVisualVector.h"

using nho::VisualVector;
using nho::ClickableVisualVector;
using nho::VectorCollisionTriangleList;

namespace ray_tri_ns
{
	static bool bEnableDebugUI = true;


	class SlideBase : public IDemoInterface
	{

	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// demo class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct FrameRenderData
	{
		GLFWwindow* window = nullptr;
		glm::mat4 view{ 1.f };
		glm::mat4 projection{ 1.f };
		glm::mat4 projection_view{ 1.f };
		float fovY_rad = 0.f;
	};
	static FrameRenderData rd;

class RayTriDemo final : public VideoDemoHelperBase
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
private://visuals
	ClickableVisualVector aVec;
	ClickableVisualVector bVec;
	sp<ho::TextBlockSceneNode> dotProductValue;
private://state
	VectorCollisionTriangleList* activeClickTarget = nullptr;
	sp<nho::TriangleListDebugger> debugRenderer = nullptr;
	bool bRightCiickPressed = false;
	std::optional<Ray> previousRayCast;
private://ui
	bool bDrawDebugCubes = false;
	bool bDebugLastRay = false;
};

struct Slide_CrossProductReview : public SlideBase
{

};
struct Slide_PlaneEquation : public SlideBase
{

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Impl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VideoDemoHelperBase::WindowParameters RayTriDemo::defineWindow()
{
	VideoDemoHelperBase::WindowParameters params;
	params.windowResolution.x = 1000;
	params.windowResolution.y = 600;

	return params;
}

void RayTriDemo::init()
{
	lineRenderer = new_up<ho::LineRenderer>();

	quatCam = new_up<QuaternionCamera>();
	quatCam->pos = glm::vec3(0, 0, 5.f);

	rd.window = window;
	
	font = new_sp<ho::Montserrat_BMF>("./assets/textures/font/Montserrat_ss_alpha_1024x1024_wb.png");
	TestText3D = new_sp<ho::TextBlockSceneNode>(font, "Testing 3 2 1.");

	//visualVectors.emplace_back();
	//visualVectors[0].setVector(glm::vec3(1.f) * 3.f);

	slides.push_back(new_sp<Slide_DotProductReview>());

	for (sp<SlideBase>& slide : slides)
	{
		slide->init();
	}
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

	rd.view = quatCam->getView();
	rd.projection = glm::perspective(fovY_rad, aspect, near, far);
	rd.projection_view = rd.projection * rd.view;
	rd.fovY_rad = fovY_rad;

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (VisualVector& vector : visualVectors)
	{
		vector.render(rd.projection_view);
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
	{
		static float testFloat = 0.f;
		ImGui::SetNextWindowPos({ 0,0 });
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("OpenGL Tweaker (imgui library)", nullptr, flags);
		{
			ImGui::SliderFloat("center CP offset divisor", &testFloat, 0.0f, 1.0f);
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
// Dot product review
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Slide_DotProductReview::init()
{
	aVec.setVector(glm::vec3(-1, 0, 0));
	bVec.setVector(glm::vec3(0, 1, 0));

	dotProductValue = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");

	//same shape is used for all clickable collision, so we can just pick one of the vector's ends.
	debugRenderer = new_sp<nho::TriangleListDebugger>(aVec.startCollision);
}

void Slide_DotProductReview::inputPoll(float dt_sec)
{
	if (glfwGetMouseButton(rd.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !bRightCiickPressed)
	{
		bRightCiickPressed = true;
		if (glfwGetInputMode(rd.window, GLFW_CURSOR))
		{
			CameraRayCastData_Triangles raycastQuery;
			raycastQuery.camFront_n = RayTriDemo::quatCam->getFront();
			raycastQuery.camRight_n = RayTriDemo::quatCam->getRight();
			raycastQuery.camUp_n = RayTriDemo::quatCam->getUp();
			raycastQuery.camPos = RayTriDemo::quatCam->pos;
			raycastQuery.fovY_deg = glm::degrees<float>(rd.fovY_rad);
			raycastQuery.window = rd.window;

			raycastQuery.objectList.push_back(&aVec.endCollision);
			raycastQuery.objectList.push_back(&aVec.startCollision);
			raycastQuery.objectList.push_back(&bVec.endCollision);
			raycastQuery.objectList.push_back(&bVec.startCollision);

			std::optional<TriangleObjectRaycastResult> raycastResult = rayCast_TriangleObjects(raycastQuery);
			if (raycastResult)
			{
				activeClickTarget = static_cast<VectorCollisionTriangleList*>(raycastResult->hitObject);
				previousRayCast = raycastResult->castRay;
			}
			else
			{
				//regenerate the non-hitting ray for debugging purposes
				previousRayCast = rayCast(raycastQuery);
			}
		}
	}
	else
	{
		activeClickTarget = nullptr;
		bRightCiickPressed = false;
	}
}

void Slide_DotProductReview::tick(float dt_sec)
{
	using namespace glm;
	using std::optional;

	if (activeClickTarget) //&& camera
	{
		vec3 cameraPosition = RayTriDemo::quatCam->pos;
		vec3 cameraFront = RayTriDemo::quatCam->getFront();

		//CameraRayCastData raycastQuery;
		//raycastQuery.camFront_n = cameraFront;
		//raycastQuery.camRight_n = RayTriDemo::quatCam->getRight();
		//raycastQuery.camUp_n = RayTriDemo::quatCam->getUp();
		//raycastQuery.camPos = cameraPosition;
		//raycastQuery.fovY_deg = glm::degrees<float>(rd.fovY_rad);
		//raycastQuery.window = rd.window;
		//if (optional<Ray> ray = rayCast(raycastQuery))
		//{
		//	vec3 toPoint = activeClickTarget->position - cameraPosition;
		//	const float projection = glm::dot(toPoint, cameraFront);
		//	const float& frontPlaneDistance = projection;
		//	const vec3 planePosition = cameraFront * frontPlaneDistance + cameraPosition;
		//	const vec3 planeNormal = -cameraFront;

		//	if (optional<vec3> planePoint = RayTests::rayPlaneIntersection(*ray, planeNormal, planePosition))
		//	{
		//		activeClickTarget->setPosition(planePoint);
		//	}
		//}
	}
}

void Slide_DotProductReview::render_game(float dt_sec)
{
	aVec.render(rd.projection_view);
	bVec.render(rd.projection_view);

	//dotProductValue-> //set text
	float dotProduct = glm::dot(aVec.getVec(), bVec.getVec());

	char textBuffer[128];
	snprintf(textBuffer, sizeof(textBuffer), "%3.3f", dotProduct);
	dotProductValue->wrappedText->text = std::string(textBuffer);

	dotProductValue->setLocalScale(glm::vec3(10.f));
	dotProductValue->setLocalPosition(
		aVec.getStart()
		+ aVec.getVec() + 0.5f * glm::normalize(aVec.getVec())
		+ glm::vec3(0.f, 1.f, 0.f) * 0.5f);
	dotProductValue->render(rd.projection, rd.view);

	if (bDrawDebugCubes)
	{
		debugRenderer->render(rd.projection_view, aVec.startCollision.cachedPreviousXform);
		debugRenderer->render(rd.projection_view, aVec.endCollision.cachedPreviousXform);
		debugRenderer->render(rd.projection_view, bVec.startCollision.cachedPreviousXform);
		debugRenderer->render(rd.projection_view, bVec.endCollision.cachedPreviousXform);
	}
	if (bDebugLastRay && previousRayCast.has_value())
	{
		RayTriDemo::lineRenderer->renderLine(previousRayCast->start, previousRayCast->start + (previousRayCast->dir * 10.f), glm::vec3(1,0,0), rd.projection_view);
	}
}

void Slide_DotProductReview::render_UI(float dt_sec)
{
	static float testFloat = 0.f;
	ImGui::SetNextWindowPos({ 700, 0 });
	ImGuiWindowFlags flags = 0;
	ImGui::Begin("Dot product review", nullptr, flags);
	if(bEnableDebugUI){
		ImGui::Checkbox("draw debug cubes", &bDrawDebugCubes);
		ImGui::Checkbox("debug ray cast", &bDebugLastRay);
	}
	ImGui::End();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
static void true_main()
{
	ray_tri_ns::RayTriDemo anim;
	anim.start();
}


int main()
{
	true_main();
}