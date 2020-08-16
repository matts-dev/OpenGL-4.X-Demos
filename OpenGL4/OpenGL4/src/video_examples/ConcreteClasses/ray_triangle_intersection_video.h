#pragma once
#include "../DrawVectorDemo.h"
#include <optional>

namespace ho
{
	struct ImmediateTriangle;
	struct TextBlockSceneNode;
}

namespace nho
{
	class VisualPoint;
	class VectorGridLines;
	class VectorProjectionAnimation;
	class ClickableVisualRay;
	struct ClickableVisualPoint;
	struct ClickableVisualVector;
	class Matrix_3D;
}

struct TriangleList_SNO;
class SceneNode_TriangleList;


namespace ray_tri_ns
{
	//using nho::VisualVector;
	//using nho::ClickableVisualVector;
	//using nho::SceneNode_VectorEnd;
	//using nho::VectorCollisionTriangleList;
	//using nho::VectorProjectionAnimation;
	//using nho::VectorGridLines;

	//TODO move things over to header as this has grown large
	class SlideBase : public DrawVectorDemo
	{

	};

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
		//nothing, just let user draw vectors and points
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
		sp<nho::ClickableVisualVector> aVec;
		sp<nho::ClickableVisualVector> bVec;
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
		sp<nho::ClickableVisualVector> aVec;
		sp<nho::ClickableVisualVector> bVec;
		sp<nho::ClickableVisualVector> crossVecVisual;
		sp<ho::TextBlockSceneNode> crossProductText;
		sp<ho::ImmediateTriangle> triRender;
	private://state
		bool bNormalizeVectors = false;
		bool bShowCrossProduct = false;
		bool bShowLength = false;
		bool bRenderArea = false;
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
		glm::vec3 triPoint_A = glm::vec3(-1, -1, -1); //left
		glm::vec3 triPoint_B = glm::vec3(1, -1, -1); //right
		glm::vec3 triPoint_C = glm::vec3(0, 1, -1); //top

	};

	////////////////////////////////////////////////////////
	// simple vector projection
	////////////////////////////////////////////////////////
	struct Slide_VectorProjectionExplanation : public SlideBase
	{
		using Parent = SlideBase;
	protected:
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		void handleVisualVectorUpdated(const nho::ClickableVisualVector& updatedVec);

		//sp<ho::ImmediateTriangle> triRender = nullptr;
		//sp<nho::VisualVector> genericVector = nullptr;
		//sp<nho::VisualPoint> genericPoint = nullptr;

		sp<nho::ClickableVisualVector> aVec;
		sp<nho::ClickableVisualVector> bVec;

		sp<nho::VectorProjectionAnimation> projectionAnim;

		sp<ho::TextBlockSceneNode> text;
	};


	////////////////////////////////////////////////////////
	// vector projection on unit axis
	////////////////////////////////////////////////////////
	struct Slide_ProjectionOnAxesGrid : public SlideBase
	{
		using Parent = SlideBase;
	protected:
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		void handleVisualVectorUpdated(const nho::ClickableVisualVector& updatedVec);
		void clearProjections();
	public:
		bool bRenderZ = false;
		bool bProject = false;

	private:
		sp<nho::ClickableVisualVector> vectorToProject;

		sp<nho::ClickableVisualVector> x_basis;
		sp<nho::ClickableVisualVector> y_basis;
		sp<nho::ClickableVisualVector> z_basis;

		sp<nho::VectorProjectionAnimation> projectionAnim_x;
		sp<nho::VectorProjectionAnimation> projectionAnim_y;
		sp<nho::VectorProjectionAnimation> projectionAnim_z;
		sp<nho::VectorGridLines> gridX;
		sp<nho::VectorGridLines> gridY;
		sp<nho::VectorGridLines> gridZ;

		sp<ho::TextBlockSceneNode> text;
	};

	////////////////////////////////////////////////////////
	// Scalar triple product review
	////////////////////////////////////////////////////////

	struct Slide_ScalarTripleProductReview : public SlideBase
	{
	public:
		virtual void init() override;
		virtual void inputPoll(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
	private://visuals
		sp<nho::ClickableVisualVector> aVec;
		sp<nho::ClickableVisualVector> bVec;
		sp<nho::ClickableVisualVector> cVec;
		sp<nho::ClickableVisualVector> crossVecVisual;
		sp<ho::TextBlockSceneNode> crossProductText;
		sp<ho::ImmediateTriangle> triRender;
		sp<ho::LineRenderer> lineRenderer;
		sp<nho::VectorProjectionAnimation> projAnim;
	private://state
		bool bNormalizeSourceVecs = false;
		bool bShowCrossProduct = false;
		bool bShowLength = false;
		bool bRenderArea = false;
		bool bNormalizeCrossResult = false;
		bool bProjectC = false;
		bool bShowParallelpiped = false;
		bool bParallelepiped = false;
		bool bRenderCrossAreaSweeping = false;
	private://impl
		bool bResetAnimProjection = false;
	private:
		float timePassedSec = 0.f;
		float timestamp_showArea = 0.f; 
		float timestamp_projection = 0.f;
		float timestamp_showPiped = 0.f;
		float timestamp_areaSweep = 0.f;
	};

	////////////////////////////////////////////////////////
	// Determinant Review
	////////////////////////////////////////////////////////

	struct Slide_DeterminantReview : public SlideBase
	{
	protected:
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
	private:
		sp<nho::Matrix_3D> matrix;
		sp<ho::ImmediateTriangle> triRender = nullptr;
		sp<nho::VisualVector> genericVector = nullptr;
		sp<nho::VisualPoint> genericPoint = nullptr;
	};


	////////////////////////////////////////////////////////
	// Barycentric reivew
	////////////////////////////////////////////////////////

	namespace EBarycentricMode
	{
		enum Type {
			MY_METHOD = 0,
			OPTIMIZED_PROJECTION,
			AREA_METHOD,
			LINEAR_SYSTEMS_METHOD
		};
	}

	struct TickData
	{
		float tickedTime = 0.f;
		float timestamp_start = 0.f;
		float dt_sec = 0.0001f;
		//float animDurationSecs = 1.f; //make this separate from tick data so we can one-line function calls
	};

	struct AnimationHelperFunctions
	{
		static void renderVector(
			bool bShouldRender,
			const TickData& td,
			glm::vec3 start,
			glm::vec3 dir,
			glm::vec3 color,
			nho::VisualVector& genericVector,
			const WindowManager::FrameRenderData& rd
		);
		static void renderVector(
			bool bShouldRender,
			const TickData& td,
			glm::vec3 start,
			glm::vec3 dir,
			glm::vec3 color,
			float timestampSecs,
			float animDurSecs,
			nho::VisualVector& genericVector,
			const WindowManager::FrameRenderData& rd,
			bool bDriftToOrigin = false
		);
		static void renderProjection(
			bool bShouldRender,
			const TickData& td,
			nho::VectorProjectionAnimation& projAnim, 
			glm::vec3 aVec,
			glm::vec3 bVec,
			glm::vec3 aStart,
			glm::vec3 bStart, 
			float animDurationSecs,
			bool bResetAnim,
			bool bForceUpdate,
			const WindowManager::FrameRenderData& rd, 
			std::optional<glm::vec3> color = std::nullopt
		);
		static void renderCrossArea(
			bool bShouldRender,
			const TickData& td,
			glm::vec3 first,
			glm::vec3 second,
			glm::vec3 start,
			glm::vec3 color,
			float timestamp_start,
			float animDurSecs,
			ho::ImmediateTriangle& triRender,
			const WindowManager::FrameRenderData& rd,
			bool bRenderHalfAreas = false
		);
	};

	struct Slide_BaryCentricsExplanation : public SlideBase
	{
	public:
		static glm::vec3 calcBarycentrics_myMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC);
		static glm::vec3 calcBarycentrics_optimizedProjection(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC);
		static glm::vec3 calcBarycentrics_AreaMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC);
		static glm::vec3 calcBarycentrics_LinearSystemMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC);
	protected:
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		void renderGame_Barycentric_myMethod(float dt_sec);
		void renderGame_Barycentric_OptimizedProjectionMethod(float dt_sec);
		void renderGame_Barycentric_AreaMethod(float dt_sec);
		void renderGame_Barycentric_SolvingLinearSystem(float dt_sec);
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override;
		void handleTestPointUpdated(const nho::VisualPoint& pnt);
	private:
		//avoiding using anim helpers for these atm because helpers cannot be done in single line, after refactoring them will use those instead of having explicit methods below
		//also avoiding because reworked some logic with projection updates to be non-negative language (reset anim instead of talkinga bout test opint updated)
		void helper_renderVector(bool bShouldRender, glm::vec3 start, glm::vec3 dir, glm::vec3 color);
		void helper_renderVector(bool bShouldRender, glm::vec3 start, glm::vec3 dir, glm::vec3 color, float timestampSecs, bool bDriftToOrigin = false);
		void helper_renderProjection(bool bShouldRender, nho::VectorProjectionAnimation& projAnim, glm::vec3 aVec, glm::vec3 bVec, glm::vec3 aStart, glm::vec3 bStart, float dt_sec, std::optional<glm::vec3> color = std::nullopt);
		void helper_renderCrossArea(bool bShouldRender, glm::vec3 first, glm::vec3 second, glm::vec3 start, glm::vec3 color, float timestamp_start);
	private:
		sp<ho::ImmediateTriangle> triRender = nullptr;
		sp<nho::VisualVector> genericVector = nullptr;
		sp<nho::VisualPoint> genericPoint = nullptr;
		sp<ho::TextBlockSceneNode> text;
	private:
		bool bTestPointUpdated = false;
	private:
		EBarycentricMode::Type barymode = EBarycentricMode::MY_METHOD;


		//ground truths
		bool bRenderRealTimeCollisionBook = false;
		bool bRenderShirleyVersion = false;
	private:
		////////////////////////////////////////////////////////
		// shared between all methods
		////////////////////////////////////////////////////////
		bool bWireframe = false;
		bool bRenderBarycentricA = true;
		bool bRenderBarycentricB = false;
		bool bRenderBarycentricC = false;
		////////////////////////////////////////////////////////
		// slow intuitive method flags
		////////////////////////////////////////////////////////
		bool bRenderBToA = false;
		bool bRenderBToC = false;
		bool bRenderBCProj = false;
		bool bRender_EdgeProjectPointToTestPoint = false;
		bool bRender_PerpendicularToEdge = false;
		bool bRenderTestPointProjectionOntoPerpendicular = false;

		float timestamp_RenderBToA = 0.f;
		float timestamp_RenderBToC = 0.f;
		float timestamp_RenderBCProj = 0.f;
		float timestamp_Render_EdgeProjectPointToTestPoint = 0.f;
		float timestamp_Render_PerpendicularToEdge = 0.f;
		float timestamp_RenderTestPointProjectionOntoPerpendicular = 0.f;
		float timestamp_RenderBarycentricA = 0.f;
		float timestamp_RenderBarycentricB = 0.f;
		float timestamp_RenderBarycentricC = 0.f;

		////////////////////////////////////////////////////////
		// optimized projection method flags
		////////////////////////////////////////////////////////
		bool bRenderAB = false;
		bool bRenderCB = false;
		bool bRender_AtoTestPnt = false;
		bool bRender_ProjToCB = false;
		bool bRender_VectorFromFirstProjection = false;
		bool bRender_projTestPointOntoPerpendicular = false;
		bool bRender_projABontoPerpendicular = false;

		float timestamp_renderbRenderAB									= 0.f;
		float timestamp_renderbRenderCB									= 0.f;
		float timestamp_renderbRender_AtoTestPnt						= 0.f;
		float timestamp_renderbRender_Perpendicular						= 0.f;
		float timestamp_renderbRender_VectorFromFirstProjection	  		= 0.f;
		float timestamp_renderbRender_projTestPointOntoPerpendicular	= 0.f;
		float timestamp_renderbRender_projABontoPerpendicular			= 0.f;

		////////////////////////////////////////////////////////
		// area method
		////////////////////////////////////////////////////////
		bool bRenderHalfAreas = false; 
		bool bRenderCrossVec_first = false;
		bool bRenderCrossVec_second = false;

		bool bAreaMethod_RenderTriNormals = false;
		bool bRenderCrossProductVectors = false;
		bool bAreaMethod_normalizeNormals = false;
		bool bAreaMethod_renderFullArea = false;
		bool bAreaMethod_renderPBC_Area = false;
		bool bAreaMethod_renderPCA_Area = false;
		bool bAreaMethod_renderPAB_Area = false;

		
		float timestamp_area_normals = 0.f;
		float timestamp_area_fullarea = 0.f;
		float timestamp_area_PBC_area = 0.f;
		float timestamp_area_PCA_area = 0.f;
		float timestamp_area_PAB_area = 0.f;
		float timestamp_crossvecfirst = 0.f;
		float timestamp_crossvecsecond = 0.f;
		float timestamp_crossproductVecs = 0.f;

	private:
		float tickedTime = 0.f;
		float vectorAnimSecs = 1.0f;
	private:
		sp<nho::ClickableVisualPoint> pntA = nullptr;
		sp<nho::ClickableVisualPoint> pntB = nullptr;
		sp<nho::ClickableVisualPoint> pntC = nullptr;

		sp<nho::ClickableVisualPoint> testPoint = nullptr;

		/////////////////////////////////////////////////////////////////////////////////////
		// intuitive method
		/////////////////////////////////////////////////////////////////////////////////////
		sp<nho::VectorProjectionAnimation> projAnim_BC;
		sp<nho::VectorProjectionAnimation> projAnim_PointOnPerpendicular;

		////////////////////////////////////////////////////////
		// optimized projection method
		////////////////////////////////////////////////////////
		sp<nho::VectorProjectionAnimation> projAnim_ab_onto_cb;
		sp<nho::VectorProjectionAnimation> projAnim_testPointOnPerpendicular;
		sp<nho::VectorProjectionAnimation> projAnim_aBOnPerpendicular;
	};


	struct Slide_MollerAndTrumbore : public SlideBase
	{
	protected:
		virtual void init() override;
		virtual void render_game(float dt_sec) override;
		virtual void render_UI(float dt_sec) override;
		virtual void tick(float dt_sec) override;
		virtual void gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList) override { SlideBase::gatherInteractableCubeObjects(objectList); }

		virtual void doTrace();

		void liveCodingTemplate()
		{
			using namespace glm;

			////////////////////////////////////////////////////////
			// point, vector, and ray review
			////////////////////////////////////////////////////////
			vec3 startPoint(1.f, 2.f, 3.f);
			startPoint.x = 7.f;
			startPoint.y = 8.f;
			startPoint.z = 9.f;

			vec3 endPoint(-3.f, -4.f, 5.f);

			vec3 vectorBetweenPoints = endPoint - startPoint;

			struct Ray
			{
				vec3 startPoint;
				vec3 direction;
				float t;
			};

			Ray myRay;
			myRay.startPoint = startPoint;
			myRay.direction = vectorBetweenPoints;
			myRay.t = 3.99f;

			vec3 rayTracedPoint = myRay.startPoint + myRay.direction * myRay.t;

			////////////////////////////////////////////////////////
			// barycentrics review
			////////////////////////////////////////////////////////
			vec3 triPointA(-1, 0, 0);
			vec3 triPointB(1, 0, 0);
			vec3 triPointC(0, 1, 0);

			//note barycentrics are not exactly like weights,
			//but behave like weights when point is within tri
			vec3 barycentrics(0.33f, 0.33f, 0.33f);

			vec3 baryTestPoitn = barycentrics.r * triPoint_A 
				+ barycentrics.g * triPoint_B 
				+ barycentrics.b* triPoint_C;
			
		}

		void liveCodingRecorded();

		sp<ho::ImmediateTriangle> triRender = nullptr;
		sp<nho::VisualVector> genericVector = nullptr;
		sp<nho::VisualPoint> genericPoint = nullptr;
	private:
		glm::vec3 rayStart{ -100.f };
		glm::vec3 rayEnd{ -100.1f };
	private:
		glm::vec3 triPoint_A = glm::vec3(-1, -1, -1); //left
		glm::vec3 triPoint_B = glm::vec3(1, -1, -1); //right
		glm::vec3 triPoint_C = glm::vec3(0, 1, -1); //top
	};

}
