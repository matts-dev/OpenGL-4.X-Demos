


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


#include "ray_triangle_intersection_video.h"
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
#include "../../new_utils/header_only/cube_mesh_from_tris.h"
#include "../../new_utils/header_only/ImmediateTriangleRenderer.h"
#include "../InteractableDemo.h"
#include "../ClickableVisualVector.h"
#include "../DrawVectorDemo.h"
#include "../ClickableVisualPoint.h"
#include "../ClickableVisualRay.h"
#include "../../new_utils/cpp_required/VisualPoint.h"
#include "../../new_utils/cpp_required/ProjectionAnimation.h"
#include "../../new_utils/cpp_required/VectorVisualizationTools/VectorGridLines.h"
#include "../../new_utils/cpp_required/InteractiveMatrix.h"

using nho::VisualVector;
using nho::ClickableVisualVector;
using nho::SceneNode_VectorEnd;
using nho::VectorCollisionTriangleList;
using nho::VectorProjectionAnimation;
using nho::VectorGridLines;

namespace ray_tri_ns
{

	glm::vec3 yellow = glm::vec3(0xff / 255.f, 0xff / 255.f, 0x14 / 255.f);


	inline bool anyValueNAN(float a) { return glm::isnan(a); }
	inline bool anyValueNAN(glm::vec3 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z); }
	inline bool anyValueNAN(glm::vec4 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z) || glm::isnan(vec.w); };
	inline bool anyValueNAN(glm::quat quat)
	{
		glm::bvec4 vec = glm::isnan(quat);
		return vec.x || vec.y || vec.z || vec.w;
	};

	glm::vec3 projectAontoB(glm::vec3 a, glm::vec3 b)
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
		//		vec3 projection =     ((a dot b) / (b dot b)) * b;	//recall that dot(b,b) == ||b||^2 == ||b||*||b||
		vec3 projection = (glm::dot(a, b) / glm::dot(b, b)) * b;
		return projection;
	}

	float clampPerc(float start, float now, float max)
	{
		float perc = glm::clamp<float>((now - start) / max, 0.f, 1.f);
		return perc;
	}

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



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MOLLER TRUMBORE CODE
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EPSILON 0.000001
#define CROSS(dest, v1, v2)\
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1];\
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2];\
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])
#define SUB(dest, v1, v2)\
	dest[0]=v1[0]-v2[0];\
	dest[1]=v1[1]-v2[1];\
	dest[2]=v1[2]-v2[2];
//#define TEST_CULL
	int moller_trumbore_intersect_triangle_original(
		double orig[3], double dir[3],
		double vert0[3], double vert1[3], double vert2[3],
		double*t, double*u, double*v)	//out params
	{
		double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
		double det, inv_det;

		/* find vectors for two edges sharing vert0 */
		SUB(edge1, vert1, vert0);
		SUB(edge2, vert2, vert0);

		/*begin calculating determinant - also used to calculate u parameter*/
		CROSS(pvec, dir, edge2);

		/*if determinant is near zero, ray lies in plane of triangle */
		det = DOT(edge1, pvec);

#ifdef TEST_CULL	/*define TEST_CULL if culling is desired */
		if (det < EPSILON)
			return 0;

		/* calculate distance from vert0 to ray origin*/
		SUB(tvec, orig, vert0);

		/*calculate U parameter and test bounds */
		*u = DOT(tvec, pvec);
		if (*u < 0.0 || *u > det)	
			return 0;

		/*prepare to test V parameter */
		CROSS(qvec, tvec, edge1);

		/*calculate V parameter and test bounds */
		*v = DOT(dir, qvec);
		if (*v < 0.0 || *u + *v > det)
			return 0;
		/* calculate t, scale parameters, ray intersects triangle */
		*t = DOT(edge2, qvec);
		inv_det = 1.0 / det;
		*t *= inv_det;
		*u *= inv_det;
		*v *= inv_det;
#else /*the non-culling branch*/
		if (det > -EPSILON && det < EPSILON)
			return 0;
		inv_det = 1.0 / det;

		/* calculate distances from vert0 to ray origin */
		SUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = DOT(tvec, pvec) * inv_det;
		if (*u < 0.0 || *u > 1.0)
			return 0;

		/* prepare to test V parameter */
		CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qvec) * inv_det;
		if (*v < 0.0 || *u + *v > 1.0)
			return 0;

		*t = DOT(edge2, qvec) * inv_det;
#endif
		return 1;
	}

	int moller_trumbore_ray_triangle_commented(
		double orig[3], double dir[3],
		double vert0[3], double vert1[3], double vert2[3],
		double*t, double*u, double*v)	//out params
	{
		double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
		double det, inv_det;

		/* find vectors for two edges sharing vert0 */
		SUB(edge1, vert1, vert0);												//vec3 edge1 = vert1 - vert0;
		SUB(edge2, vert2, vert0);												//edge2 = vert2 - vert0;

		/*begin calculating determinant - also used to calculate u parameter*/
		CROSS(pvec, dir, edge2);												//pvec = cross(dir, edge2);		//this is first half scalar triple product

		/*if determinant is near zero, ray lies in plane of triangle */
		det = DOT(edge1, pvec);													//dot(cross(dir, edge2), edge1) - second half of scalar triple product, basically is dot(cross(dir, edge2), edge1) but ordered in a weird way

#ifdef TEST_CULL	/*define TEST_CULL if culling is desired */
		if (det < EPSILON)														//don't divide by zero, no unique solution - ray parallel (ie never hit plane or within plane)
			return 0;															//early out if we can't find a unique solution. We only need to check positive determinants because we know front-facing tri will have positve det (use your fingers, rhand rule; say edge1=x,edge2=y,dir=z (all cases similar to this), the cross product result, in a way, canbe though to be projected on edge1(x), which will be positive dot

		/* calculate distance from vert0 to ray origin*/
		SUB(tvec, orig, vert0);													//tvec = ori-v0; setting up for solving our system of equations via determinant

		/*calculate U parameter and test bounds */
		*u = DOT(tvec, pvec);													//1. dot(cross(dir,edge2),ori-v0) =>scalarTripleProduct => determinant
		if (*u < 0.0 || *u > det)												// we didn't finish cramer's rule just yet, because we may not need to. we're goint to divide by det, so we can see if it will be greater than 1 by just seeing if it is bigger than the det
			return 0;															// we know we're out of range, no need to finish up the calculations

		/*prepare to test V parameter */
		CROSS(qvec, tvec, edge1);												//qvec = cross(ori-v0, edge1);  -- preparing for another scalarTripleProduct	

		/*calculate V parameter and test bounds */
		*v = DOT(dir, qvec);													//2. dot(cross(ori-v0, edge1),dir) -- finding determinant; but holding off on divide as we may not need it
		if (*v < 0.0 || *u + *v > det)											//see if barycentric is out of range; again we apply similar logic that we're divide by det, so we can see if passed 1 by seeing if sum is greater than the divsor
			return 0;															//if we're out of barycentric bounds, return false -- no hit.
		/* calculate t, scale parameters, ray intersects triangle */
		*t = DOT(edge2, qvec);													//3. dot(cross(ori-v0,edge1), edge2) we know we're in bounds, prep last cramer rule numerator det, 
		inv_det = 1.0 / det;													//we didn't early out, spend time to finish up cramer's rule. 
		*t *= inv_det;															//finish t by dividing by det(fullMatrix)
		*u *= inv_det;															//finish barycentric u by det(fullMatrix)
		*v *= inv_det;															//finish barycentric v by det(fullMatrix)
#else /*the non-culling branch*/
		if (det > -EPSILON && det < EPSILON)									//don't divide by zero, if det of system is 0; no unique solution -- ray paralel (or within tri plane)
			return 0;
		inv_det = 1.0 / det;													//part of cramers rule; we ultimately divide by determinat of entire matrix (same as multiplying by inverse)

		/* calculate distances from vert0 to ray origin */
		SUB(tvec, orig, vert0);													//tvec = ori-vert0;		//this is the RHS of system of equations (ie vec from V0 to origin)

		/* calculate U parameter and test bounds */
		*u = DOT(tvec, pvec) * inv_det;											//1. rearranged: dot(cross(dir,edge2),ori-v0) / det => scalarTripleProduct/det => that is: determinant(byScalarTripleProduct)/determinant(fullMatrix)
		if (*u < 0.0 || *u > 1.0)
			return 0;															//if the barycentric coordinate u is out of bounds, then we know we didn't hit triangle; early out

		/* prepare to test V parameter */
		CROSS(qvec, tvec, edge1);												//qvec = cross(ori-v0, edge1);	//setting up to calculate another detminant from another scalar triple product

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qvec) * inv_det;											//2. dot(cross(ori-v0,edge1), dir) / det => scalarTripleProduct/det => cramers rule => det(matrixWithAdjustColumn)/det(fullmatrix)
		if (*v < 0.0 || *u + *v > 1.0)											//by checking the sum of barycentrics u and v, we can tell if the 3rd w is negative (as they should sum to 1); we can test v and w in one check this way. we know u<1.0; so we really just need to see if u+v<1.0 too
			return 0;															//if this barycentric (or if the 3rd barycentric will be out of bounds), then early out

		*t = DOT(edge2, qvec) * inv_det;										//3. the last bit of cramers rule, dot(cross(ori-v0,edge1), edge2) / det => scalarTripleProduct/det => det(adjustedMatrix)/det(systemOfEqMatrix)
#endif
		return 1;																//we hit triangle, return true!
	}



	int moller_trumbore_ray_triangle_commented_renamed(
		double orig[3], double rayDir[3],					//ray data
		double vert0[3], double vert1[3], double vert2[3],	//triangle data
		double*rayT, double*baryU, double*baryV)			//out params
	{
		double edge1[3], edge2[3], orig_minus_vert0[3], cross_rayDir_edge2[3], cross_oriMinusVert0_edge1[3]; //strict c89 requires variables be declared at start of block scope
		double det, inv_det;

		/* find vectors for two edges sharing vert0 */
		SUB(edge1, vert1, vert0);												//vec3 edge1 = vert1 - vert0;
		SUB(edge2, vert2, vert0);												//edge2 = vert2 - vert0;

		/*begin calculating determinant - also used to calculate u parameter*/
		CROSS(cross_rayDir_edge2, rayDir, edge2);								//cross_rayDir_edge2 = cross(dir, edge2);		//this is first half scalar triple product

		/*if determinant is near zero, ray lies in plane of triangle */
		det = DOT(edge1, cross_rayDir_edge2);									//dot(cross(dir, edge2), edge1) - second half of scalar triple product, basically is dot(cross(dir, edge2), edge1) but ordered in a weird way

#ifdef TEST_CULL	/*define TEST_CULL if culling is desired */
		if (det < EPSILON)														//don't divide by zero, no unique solution - ray parallel (ie never hit plane or within plane)
			return 0;															//early out if we can't find a unique solution. We only need to check positive determinants because we know front-facing tri will have positve det (use your fingers, rhand rule; say edge1=x,edge2=y,dir=z (all cases similar to this), the cross product result, in a way, canbe though to be projected on edge1(x), which will be positive dot

		/* calculate distance from vert0 to ray origin*/
		SUB(orig_minus_vert0, orig, vert0);										//tvec = ori-v0; setting up for solving our system of equations via determinant

		/*calculate U parameter and test bounds */
		*baryU = DOT(orig_minus_vert0, cross_rayDir_edge2);						//1. dot(cross(dir,edge2),ori-v0) =>scalarTripleProduct => determinant
		if (*baryU < 0.0 || *baryU > det)										// we didn't finish cramer's rule just yet, because we may not need to. we're goint to divide by det, so we can see if it will be greater than 1 by just seeing if it is bigger than the det
			return 0;															// we know we're out of range, no need to finish up the calculations

		/*prepare to test V parameter */
		CROSS(cross_oriMinusVert0_edge1, orig_minus_vert0, edge1);				//qvec = cross(ori-v0, edge1);  -- preparing for another scalarTripleProduct	

		/*calculate V parameter and test bounds */
		*baryV = DOT(rayDir, cross_oriMinusVert0_edge1);						//2. dot(cross(ori-v0, edge1),dir) -- finding determinant; but holding off on divide as we may not need it
		if (*baryV < 0.0 || *baryU + *baryV > det)								//see if barycentric is out of range; again we apply similar logic that we're divide by det, so we can see if passed 1 by seeing if sum is greater than the divsor
			return 0;															//if we're out of barycentric bounds, return false -- no hit.
		/* calculate t, scale parameters, ray intersects triangle */
		*rayT = DOT(edge2, cross_oriMinusVert0_edge1);							//3. dot(cross(ori-v0,edge1), edge2) we know we're in bounds, prep last cramer rule numerator det, 
		inv_det = 1.0 / det;													//we didn't early out, spend time to finish up cramer's rule. 
		*rayT *= inv_det;														//finish t by dividing by det(fullMatrix)
		*baryU *= inv_det;														//finish barycentric u by det(fullMatrix)
		*baryV *= inv_det;														//finish barycentric v by det(fullMatrix)
#else /*the non-culling branch*/
		if (det > -EPSILON && det < EPSILON)									//don't divide by zero, if det of system is 0; no unique solution -- ray paralel (or within tri plane)
			return 0;
		inv_det = 1.0 / det;													//part of cramers rule; we ultimately divide by determinat of entire matrix (same as multiplying by inverse)

		/* calculate distances from vert0 to ray origin */
		SUB(orig_minus_vert0, orig, vert0);										//tvec = ori-vert0;		//this is the RHS of system of equations (ie vec from V0 to origin)

		/* calculate U parameter and test bounds */
		*baryU = DOT(orig_minus_vert0, cross_rayDir_edge2) * inv_det;			//1. rearranged: dot(cross(rayDir,edge2),ori-v0) / det => scalarTripleProduct/det => that is: determinant(byScalarTripleProduct)/determinant(fullMatrix)
		if (*baryU < 0.0 || *baryU > 1.0)
			return 0;															//if the barycentric coordinate u is out of bounds, then we know we didn't hit triangle; early out

		/* prepare to test V parameter */
		CROSS(cross_oriMinusVert0_edge1, orig_minus_vert0, edge1);				//qvec = cross(ori-v0, edge1);	//setting up to calculate another detminant from another scalar triple product

		/* calculate V parameter and test bounds */
		*baryV = DOT(rayDir, cross_oriMinusVert0_edge1) * inv_det;				//2. dot(cross(ori-v0,edge1), rayDir) / det => scalarTripleProduct/det => cramers rule => det(matrixWithAdjustColumn)/det(fullmatrix)
		if (*baryV < 0.0 || *baryU + *baryV > 1.0)								//by checking the sum of barycentrics u and v, we can tell if the 3rd w is negative (as they should sum to 1); we can test v and w in one check this way. we know u<1.0; so we really just need to see if u+v<1.0 too
			return 0;															//if this barycentric (or if the 3rd barycentric will be out of bounds), then early out

		*rayT = DOT(edge2, cross_oriMinusVert0_edge1) * inv_det;				//3. the last bit of cramers rule, dot(cross(ori-v0,edge1), edge2) / det => scalarTripleProduct/det => det(adjustedMatrix)/det(systemOfEqMatrix)
#endif
		return 1;																//we hit triangle, return true!
	}


	int moller_trumbore_intersect_triangle_GLM(
		glm::dvec3 orig, glm::dvec3 rayDir,
		glm::dvec3 vert0, glm::dvec3 vert1, glm::dvec3 vert2,
		double& rayT,	//out ray t value
		double& baryU,	//out barycentric 1
		double& baryV	//out barycentric 2
	)
	{

		glm::dvec3 edge1, edge2, tvec, pvec, qvec;
		double det, inv_det;

		/* find vectors for two edges sharing vert0 */
		edge1 = vert1 - vert0;
		edge2 = vert2 - vert0;

		/*begin calculating determinant - also used to calculate u parameter*/
		pvec = glm::cross(rayDir, edge2);//this is first half scalar triple product

		/*if determinant is near zero, ray lies in plane of triangle */
		det = glm::dot(edge1, pvec);//dot(cross(dir, edge2), edge1) -- second half of scalar triple product, basically is dot(cross(dir, edge2), edge1) but ordered in a weird way

#ifdef TEST_CULL	/*define TEST_CULL if culling is desired */
		if (det < EPSILON)														//don't divide by zero, no unique solution - ray parallel (ie never hit plane or within plane)
			return 0;															//early out if we can't find a unique solution. We only need to check positive determinants because we know front-facing tri will have positve det (use your fingers, rhand rule; say edge1=x,edge2=y,dir=z (all cases similar to this), the cross product result, in a way, canbe though to be projected on edge1(x), which will be positive dot

		/* calculate distance from vert0 to ray origin*/
		tvec = orig - vert0;													//tvec = ori-v0; setting up for solving our system of equations via determinant

		/*calculate U parameter and test bounds */
		baryU = glm::dot(tvec, pvec);													//1. dot(cross(dir,edge2),ori-v0) =>scalarTripleProduct => determinant
		if (baryU < 0.0 || baryU > det)												// we didn't finish cramer's rule just yet, because we may not need to. we're goint to divide by det, so we can see if it will be greater than 1 by just seeing if it is bigger than the det
			return 0;															// we know we're out of range, no need to finish up the calculations

		/*prepare to test V parameter */
		qvec = glm::cross(tvec, edge1);												//qvec = cross(ori-v0, edge1);  -- preparing for another scalarTripleProduct	

		/*calculate V parameter and test bounds */
		baryV = glm::dot(rayDir, qvec);													//2. dot(cross(ori-v0, edge1),dir) -- finding determinant; but holding off on divide as we may not need it
		if (baryV < 0.0 || baryU + baryV > det)											//see if barycentric is out of range; again we apply similar logic that we're divide by det, so we can see if passed 1 by seeing if sum is greater than the divsor
			return 0;															//if we're out of barycentric bounds, return false -- no hit.
		/* calculate t, scale parameters, ray intersects triangle */
		rayT = glm::dot(edge2, qvec);													//3. dot(cross(ori-v0,edge1), edge2) we know we're in bounds, prep last cramer rule numerator det, 
		inv_det = 1.0 / det;													//we didn't early out, spend time to finish up cramer's rule. 
		rayT *= inv_det;															//finish t by dividing by det(fullMatrix)
		baryU *= inv_det;															//finish barycentric u by det(fullMatrix)
		baryV *= inv_det;															//finish barycentric v by det(fullMatrix)
#else /*the non-culling branch*/
		if (det > -EPSILON && det < EPSILON)//don't divide by zero, if det of system is 0; no unique solution -- ray paralel (or within tri plane)
			return 0;
		inv_det = 1.0 / det; //part of cramers rule; we ultimately divide by determinat of entire matrix (same as multiplying by inverse)

		/* calculate distances from vert0 to ray origin */
		tvec = orig - vert0;		//this is the RHS of system of equations (ie vec from V0 to origin)

		/* calculate U parameter and test bounds */
		baryU = glm::dot(tvec, pvec) * inv_det;//1. rearranged: dot(cross(dir,edge2),ori-v0) / det => scalarTripleProduct/det => that is: determinant(byScalarTripleProduct)/determinant(fullMatrix)
		if (baryU < 0.0 || baryU > 1.0)
			return 0;//if the barycentric coordinate u is out of bounds, then we know we didn't hit triangle; early out

		/* prepare to test V parameter */
		qvec = glm::cross(tvec, edge1);//qvec = cross(ori-v0, edge1);	//setting up to calculate another detminant from another scalar triple product

		/* calculate V parameter and test bounds */
		baryV = glm::dot(rayDir, qvec) * inv_det;//2. dot(cross(ori-v0,edge1), dir) / det => scalarTripleProduct/det => cramers rule => det(matrixWithAdjustColumn)/det(fullmatrix)
		if (baryV < 0.0 || baryU + baryV > 1.0)//by checking the sum of barycentrics u and v, we can tell if the 3rd w is negative (as they should sum to 1); we can test v and w in one check this way. we know u<1.0; so we really just need to see if u+v<1.0 too
			return 0;//if this barycentric (or if the 3rd barycentric will be out of bounds), then early out

		rayT = glm::dot(edge2, qvec) * inv_det;//3. the last bit of cramers rule, dot(cross(ori-v0,edge1), edge2) / det => scalarTripleProduct/det => det(adjustedMatrix)/det(systemOfEqMatrix)
#endif
		return 1;//we hit triangle, return true!
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end moller trumbore code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

		////////////////////////////////////////////////////////
		// Vector projection video
		////////////////////////////////////////////////////////
		if constexpr (constexpr bool bEnableProjectionSlides = true)
		{
			slides.push_back(new_sp<Slide_VectorProjectionExplanation>());
			slides.push_back(new_sp<Slide_ProjectionOnAxesGrid>());
		}

		////////////////////////////////////////////////////////
		// MollerAndTrumbore Video
		////////////////////////////////////////////////////////
		if constexpr (constexpr bool bEnableMTRayTriSlides = true)
		{
			slides.push_back(new_sp<Slide_BaryCentricsExplanation>());
			slides.push_back(new_sp<Slide_ScalarTripleProductReview>());
			slides.push_back(new_sp<Slide_DeterminantReview>());
			slides.push_back(new_sp<Slide_MollerAndTrumbore>());
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// intuitive ray-triangle video
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

		triRender = new_sp<ho::ImmediateTriangle>();
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
		using namespace glm;
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


			if (bRenderArea)
			{
				vec3 pointA = aVec->getStart();
				vec3 pointB = aVec->getStart() + aVec->getVec();
				vec3 pointC = aVec->getStart() + bVec->getVec();

				//first half of area
				triRender->renderTriangle(
					pointA,
					pointB,
					pointC,
					/*color*/glm::vec3(0.25f, 0.25f, 1.f), rd->projection_view);

				//second half of area
				triRender->renderTriangle(
					pointB,
					pointB + bVec->getVec(),
					pointC,
					/*color*/glm::vec3(0.25f, 0.25f, 1.f), rd->projection_view);
			}
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
		ImGui::Checkbox("show area", &bRenderArea);
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// vector projection explanation
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Slide_VectorProjectionExplanation::init()
	{
		using namespace glm;
		Parent::init();

		//genericVector = new_sp<nho::VisualVector>();
		//genericVector->bUseCenteredMesh = false;
		//genericPoint = new_sp<nho::VisualPoint>();

		aVec = new_sp<ClickableVisualVector>();
		bVec = new_sp<ClickableVisualVector>();
		projectionAnim = new_sp<nho::VectorProjectionAnimation>();
		projectionAnim->setColor(vec3(1, 0, 0));

		aVec->setVector(vec3(2.f, 1.f, 0));
		bVec->setVector(vec3(2.5f, 0.f, 0));

		aVec->eventValuesUpdated.addWeakObj<Slide_VectorProjectionExplanation>(event_this(), &Slide_VectorProjectionExplanation::handleVisualVectorUpdated);
		bVec->eventValuesUpdated.addWeakObj<Slide_VectorProjectionExplanation>(event_this(), &Slide_VectorProjectionExplanation::handleVisualVectorUpdated);

		text = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
		text->setLocalPosition(vec3(0.f, -1.f, 0.f));

	}

	void Slide_VectorProjectionExplanation::render_game(float dt_sec)
	{
		using namespace glm;

		Parent::render_game(dt_sec);

		if (rd->camera)
		{
			aVec->render(rd->projection_view, rd->camera->getPosition());
			bVec->render(rd->projection_view, rd->camera->getPosition());

			if (QuaternionCamera* camera = dynamic_cast<QuaternionCamera*>(rd->camera))
			{
				text->setLocalRotation(camera->getRotation());
				text->setLocalScale(vec3(5.f));
				text->wrappedText->text = "vector projection review";
				//text->render(rd->projection, rd->view);
			}

			projectionAnim->render(rd->projection_view, rd->camera->getPosition());
		}
	}

	void Slide_VectorProjectionExplanation::render_UI(float dt_sec)
	{
		Parent::render_UI(dt_sec);

		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Vector projections", nullptr, flags);
		{
			if (ImGui::Button("project"))
			{
				if (const sp<nho::ClickableVisualVector>& selection = getSelection())
				{
					projectionAnim->projectFromAtoB(*selection, selection == aVec ? *bVec:*aVec);
				}
				else
				{
					projectionAnim->projectFromAtoB(*aVec, *bVec);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("clear"))
			{
				projectionAnim->setShouldRender(false);
			}
			if (projectionAnim->animMode == nho::EAnimMode::FASTER_TIP)
			{
				ImGui::SameLine();
				ImGui::SliderFloat("tip speed factor", &projectionAnim->animSpeedupFactor, 0.f, 1.f);
			}
			ImGui::SliderFloat("duration sec", &projectionAnim->animDurSec, 0.20f, 5.f);
			static int proxyAnimMode = int(projectionAnim->animMode);
			if (ImGui::SliderInt("anim", &proxyAnimMode, 0, 2))
			{
				projectionAnim->animMode = static_cast<nho::EAnimMode::type>(proxyAnimMode);
			}
			ImGui::Checkbox("loop", &projectionAnim->bLoop);

			ImGui::Checkbox("project with line", &projectionAnim->bRenderLineFromTip);

		}
		ImGui::End();
	}

	void Slide_VectorProjectionExplanation::tick(float dt_sec)
	{
		Parent::tick(dt_sec);

		projectionAnim->tick(dt_sec);
	}

	void Slide_VectorProjectionExplanation::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		Parent::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&aVec->startCollision->getTriangleList());
		objectList.push_back(&aVec->endCollision->getTriangleList());
		objectList.push_back(&bVec->startCollision->getTriangleList());
		objectList.push_back(&bVec->endCollision->getTriangleList());
	}


	void Slide_VectorProjectionExplanation::handleVisualVectorUpdated(const ClickableVisualVector& updatedVec)
	{
		if (projectionAnim)
		{
			projectionAnim->setShouldRender(false);
		}
	}

	////////////////////////////////////////////////////////
	// vector grid projection
	////////////////////////////////////////////////////////
	void Slide_ProjectionOnAxesGrid::init()
	{
		using namespace glm;
		Parent::init();

		bEnablePointDrawing = false;
		bEnableVectorDrawing = false;
		bColorSelectedVectors = false;

		vectorToProject = new_sp<ClickableVisualVector>();
		vectorToProject->setVector(vec3(2.f, 1.f, 0));
		vectorToProject->bUseCenteredMesh = false;

		x_basis = new_sp<ClickableVisualVector>();
		x_basis->setVector(vec3(1, 0, 0));
		x_basis->color = glm::vec3(1.0f, 0.5f, 0.5f);

		y_basis = new_sp<ClickableVisualVector>();
		y_basis->setVector(vec3(0, 1, 0));
		y_basis->color = glm::vec3(0.5f, 1.0f, 0.5f);

		z_basis = new_sp<ClickableVisualVector>();
		z_basis->setVector(vec3(0, 0, 1));
		z_basis->color = glm::vec3(0.5f, 0.5f, 1.0f);

		projectionAnim_x = new_sp<nho::VectorProjectionAnimation>();
		projectionAnim_x->setColor(vec3(1, 0, 0));

		projectionAnim_y = new_sp<nho::VectorProjectionAnimation>();
		projectionAnim_y->setColor(vec3(0, 1, 0));

		projectionAnim_z = new_sp<nho::VectorProjectionAnimation>();
		projectionAnim_z->setColor(vec3(0, 0, 1));

		projectionAnim_x->bRenderLineFromTip 
			= projectionAnim_y->bRenderLineFromTip 
			= projectionAnim_z->bRenderLineFromTip 
			= false;

		gridX = new_sp<VectorGridLines>();
		gridX->setVector(x_basis);

		gridY = new_sp<VectorGridLines>();
		gridY->setVector(y_basis);

		gridZ = new_sp<VectorGridLines>();
		gridZ->setVector(z_basis);

		vectorToProject->eventValuesUpdated.addWeakObj(event_this(), &Slide_ProjectionOnAxesGrid::handleVisualVectorUpdated);
		x_basis->eventValuesUpdated.addWeakObj(event_this(), &Slide_ProjectionOnAxesGrid::handleVisualVectorUpdated);
		y_basis->eventValuesUpdated.addWeakObj(event_this(), &Slide_ProjectionOnAxesGrid::handleVisualVectorUpdated);
		z_basis->eventValuesUpdated.addWeakObj(event_this(), &Slide_ProjectionOnAxesGrid::handleVisualVectorUpdated);

		text = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");
		text->setLocalPosition(vec3(0.f, -1.f, 0.f));
	}

	void Slide_ProjectionOnAxesGrid::render_game(float dt_sec)
	{
		Parent::render_game(dt_sec);

		if (rd->camera)
		{
			glm::vec3 camPos = rd->camera->getPosition();
			vectorToProject->render(rd->projection_view, camPos);

			x_basis->render(rd->projection_view, camPos);
			gridX->render(rd->projection_view, camPos);

			y_basis->render(rd->projection_view, camPos);
			gridY->render(rd->projection_view, camPos);

			projectionAnim_x->render(rd->projection_view, camPos);
			projectionAnim_y->render(rd->projection_view, camPos);

			if (bRenderZ)
			{
				z_basis->render(rd->projection_view, camPos);
				gridZ->render(rd->projection_view, camPos);
				projectionAnim_z->render(rd->projection_view, camPos);
			}
		}
	}

	void Slide_ProjectionOnAxesGrid::render_UI(float dt_sec)
	{
		Parent::render_UI(dt_sec);

		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("grid vector projections", nullptr, flags);
		{
			if (ImGui::Button("project"))
			{
				bProject = true;
				projectionAnim_x->projectFromAtoB(*vectorToProject, *x_basis);
				projectionAnim_y->projectFromAtoB(*vectorToProject, *y_basis);
				projectionAnim_z->projectFromAtoB(*vectorToProject, *z_basis);

				//don't project all vectors at once, tick will enable animations
				projectionAnim_y->pauseAnimation();
				projectionAnim_z->pauseAnimation();
			}
			ImGui::SameLine();
			if (ImGui::Button("clear"))
			{
				bProject = false;
				//this will cause a clear of all data
				clearProjections();
			}
			
			float durationProxy = projectionAnim_x->animDurSec;
			if (ImGui::SliderFloat("duration sec", &durationProxy, 0.25f, 5.f))
			{
				projectionAnim_x->animDurSec = durationProxy;
				projectionAnim_y->animDurSec = durationProxy;
				projectionAnim_z->animDurSec = durationProxy;
			}

			//bool bLoopProxy = projectionAnim_x->bLoop;
			//if (ImGui::Checkbox("loop", &bLoopProxy))
			//{
			//	projectionAnim_x->bLoop = bLoopProxy;
			//	projectionAnim_y->bLoop = bLoopProxy;
			//	projectionAnim_z->bLoop = bLoopProxy;
			//}

			bool bDrawLineFromTip = projectionAnim_x->bRenderLineFromTip;
			if (ImGui::Checkbox("project with line", &bDrawLineFromTip))
			{
				projectionAnim_x->bRenderLineFromTip = bDrawLineFromTip;
				projectionAnim_y->bRenderLineFromTip = bDrawLineFromTip;
				projectionAnim_z->bRenderLineFromTip = bDrawLineFromTip;
			}

			if (ImGui::Checkbox("z", &bRenderZ))
			{
				//clear any projections
				clearProjections();
				gridZ->resetAnim();
			}

		}
		ImGui::End();
	}

	void Slide_ProjectionOnAxesGrid::tick(float dt_sec)
	{
		Parent::tick(dt_sec);

		if (rd->camera)
		{
			glm::vec3 fromOriginToCamera = rd->camera->getPosition();//consider this a vector from 0;
			float distToCam = glm::length(fromOriginToCamera);

			float projectionScale = distToCam * 0.25f;
			projectionAnim_x->setPointScale(projectionScale);
			projectionAnim_y->setPointScale(projectionScale);
			projectionAnim_z->setPointScale(projectionScale);
		}

		if (bProject)
		{
			if (projectionAnim_x->isAnimationDone())
			{
				projectionAnim_y->resumeAnimation();
			}
			if (projectionAnim_y->isAnimationDone())
			{
				projectionAnim_z->resumeAnimation();
			}
		}

		projectionAnim_x->tick(dt_sec);
		projectionAnim_y->tick(dt_sec);
		projectionAnim_z->tick(dt_sec);

		gridX->tick(dt_sec);
		gridY->tick(dt_sec);
		gridZ->tick(dt_sec);

	}

	void Slide_ProjectionOnAxesGrid::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		Parent::gatherInteractableCubeObjects(objectList);

		//objectList.push_back(&vectorToProject->startCollision->getTriangleList());
		objectList.push_back(&vectorToProject->endCollision->getTriangleList());
		
		//objectList.push_back(&x_basis->startCollision->getTriangleList());
		objectList.push_back(&x_basis->endCollision->getTriangleList());

		//objectList.push_back(&y_basis->startCollision->getTriangleList());
		objectList.push_back(&y_basis->endCollision->getTriangleList());

		//objectList.push_back(&z_basis->startCollision->getTriangleList());
		objectList.push_back(&z_basis->endCollision->getTriangleList());

	}

	void Slide_ProjectionOnAxesGrid::handleVisualVectorUpdated(const ClickableVisualVector& updatedVec)
	{
		if (projectionAnim_x && projectionAnim_y && projectionAnim_z
			&& projectionAnim_x->isAnimating() && projectionAnim_y->isAnimating() && projectionAnim_z->isAnimating()
			)
		{
			projectionAnim_x->projectFromAtoB(*vectorToProject, *x_basis, false);
			projectionAnim_y->projectFromAtoB(*vectorToProject, *y_basis, false);
			projectionAnim_z->projectFromAtoB(*vectorToProject, *z_basis, false);
		}
	}

	void Slide_ProjectionOnAxesGrid::clearProjections()
	{
		if (projectionAnim_x && projectionAnim_y && projectionAnim_z)
		{
			projectionAnim_x->setShouldRender(false);
			projectionAnim_y->setShouldRender(false);
			projectionAnim_z->setShouldRender(false);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// barycentric coordinate review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glm::vec3 Shirley_getBarycentricCoordinatesAt(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) 
	{
		//Graphics book Shirly method for calculating barycentrics using areas from cross product: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
		//tri is formed by a,b,c
		using namespace glm;

		glm::vec3 bary;
		glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));

		// The area of a triangle is 
		float areaABC = glm::dot(normal, glm::cross((b - a), (c - a)));
		float areaPBC = glm::dot(normal, glm::cross((b - p), (c - p)));
		float areaPCA = glm::dot(normal, glm::cross((c - p), (a - p)));

		bary.x = areaPBC / areaABC; // alpha
		bary.y = areaPCA / areaABC; // beta
		bary.z = 1.0f - bary.x - bary.y; // gamma

		return bary;
	}


	void realtimecollisiondetectionbook_Barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, float &u, float &v, float &w)
	{
		glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		v = (d11 * d20 - d01 * d21) / denom;
		w = (d00 * d21 - d01 * d20) / denom;
		u = 1.0f - v - w;
	}

	glm::vec3 Slide_BaryCentricsExplanation::calcBarycentrics_myMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC)
	{
		using namespace glm;

		////////////////////////////////////////////////////////
		//render barycentric coordinates of test point
		////////////////////////////////////////////////////////
		auto calcBarycentric = [&](
			  const glm::vec3& aPos
			, const glm::vec3& bPos
			, const glm::vec3& cPos
			)
		{
			const vec3 b_to_a = aPos - bPos;
			const vec3 b_to_c = cPos - bPos;

			const vec3 bc_proj = projectAontoB(b_to_a, b_to_c);

			const vec3 projectionPoint = bPos + bc_proj;
			const vec3 edgeProjPoint_to_TestPoint = testPoint - projectionPoint;

			const vec3 perpendicularWithEdgeVec = aPos - projectionPoint; //perpendicular line to triangle edge
			const vec3 testPoint_projOnTo_perpendicular = projectAontoB(edgeProjPoint_to_TestPoint, perpendicularWithEdgeVec);

			//this is using abs length, not scalar projection -- this is is probably correct but needs investigation as it doesn't produce negative outside
			//float lengthRatio = glm::length(testPoint_projOnTo_perpendicular) / glm::length(perpendicularWithEdgeVec);

			float scalarProj = glm::dot(edgeProjPoint_to_TestPoint, glm::normalize(perpendicularWithEdgeVec));
			float  lengthRatio = scalarProj / glm::length(perpendicularWithEdgeVec);

			return lengthRatio;
		};

		float barycentric_a = calcBarycentric(pntA, pntB, pntC);
		float barycentric_b = calcBarycentric(pntB, pntC, pntA);
		float barycentric_c = calcBarycentric(pntC, pntA, pntB);

		return glm::vec3(barycentric_a, barycentric_b, barycentric_c);
	}


	glm::vec3 Slide_BaryCentricsExplanation::calcBarycentrics_optimizedProjection(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Below can be highly optimized by analyzing the actual math symbolically and removing redundancies in calculating projections etc.
		// (see Math for Game Developers - Ray Triangle Intersection by Jorge Rodriguez). I'm more interested
		// in teaching the concept rather than every exact method that has been optimized completely.
		// I may look and see what what this all reduces down to and show it, but at the moment leaving an unoptimized and untested version commented out
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		using namespace glm;

		auto calcBarycentric = [&](
			const vec3& aPos
			, const vec3& bPos
			, const vec3& cPos
			)
		{
			const vec3 ab = bPos - aPos;
			const vec3 cb = cPos - bPos;
			const vec3 a_to_testPoint = testPoint - aPos;

			const vec3 perpendicular = ab - ((glm::dot(ab, cb) / glm::dot(cb, cb)) * cb);

			float proj_a_to_testpointFactor = glm::dot(a_to_testPoint, perpendicular) /* /dot(perpendicular,perpendicular)*/;
			float proj_ab = glm::dot(ab, perpendicular) /* /dot(perpendicular,perpendicular)*/;

			return 1 - (proj_a_to_testpointFactor / proj_ab);
		};

		float barycentric_a = calcBarycentric(pntA, pntB, pntC);
		float barycentric_b = calcBarycentric(pntB, pntC, pntA);
		float barycentric_c = 1 - (barycentric_a + barycentric_b);

		return glm::vec3(barycentric_a, barycentric_b, barycentric_c);
	}

	glm::vec3 Slide_BaryCentricsExplanation::calcBarycentrics_AreaMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC)
	{
		//Graphics book Shirley method for calculating barycentrics using areas from cross product: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
		//tri is formed by a,b,c
		using namespace glm;

		glm::vec3 bary;
		//glm::vec3 normal = glm::normalize(glm::cross(pntB - pntA, pntC - pntA));
		glm::vec3 normal = glm::cross(pntB - pntA, pntC - pntA);//it appears this will work even if normal isn't normalized, but areas apear to be scalar projections so this may be invalid in terms of correct area, but still works as it is same proportion?
		
		// The area of a triangle is 
		float areaABC = glm::dot(normal, glm::cross((pntB - pntA), (pntC - pntA)));
		float areaPBC = glm::dot(normal, glm::cross((pntB - testPoint), (pntC - testPoint)));
		float areaPCA = glm::dot(normal, glm::cross((pntC - testPoint), (pntA - testPoint)));

		bary.x = areaPBC / areaABC; // alpha
		bary.y = areaPCA / areaABC; // beta
		bary.z = 1.0f - bary.x - bary.y; // gamma

		return bary;
	}

	glm::vec3 Slide_BaryCentricsExplanation::calcBarycentrics_LinearSystemMethod(glm::vec3 testPoint, glm::vec3 pntA, glm::vec3 pntB, glm::vec3 pntC)
	{
		//real time collision method using cramers rule to solve system of equations

		glm::vec3 a_to_b = pntB - pntA;
		glm::vec3 a_to_c = pntC - pntA;
		glm::vec3 a_to_testpoint = testPoint - pntA;

		float d00 = glm::dot(a_to_b, a_to_b);
		float d01 = glm::dot(a_to_b, a_to_c);
		float d11 = glm::dot(a_to_c, a_to_c);
		float d20 = glm::dot(a_to_testpoint, a_to_b);
		float d21 = glm::dot(a_to_testpoint, a_to_c);

		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		return glm::vec3(u, v, w);
	}

	void Slide_BaryCentricsExplanation::init()
	{
		SlideBase::init();

		triRender = new_sp<ho::ImmediateTriangle>();

		pntA = new_sp<nho::ClickableVisualPoint>();
		pntB = new_sp<nho::ClickableVisualPoint>();
		pntC = new_sp<nho::ClickableVisualPoint>();
		testPoint = new_sp<nho::ClickableVisualPoint>();

		glm::vec3 triPoint_A = glm::vec3(-1, -1, -1); //left
		glm::vec3 triPoint_B = glm::vec3(1, -1, -1); //right
		glm::vec3 triPoint_C = glm::vec3(0, 1, -1); //top
		pntA->setPosition(triPoint_A);
		pntB->setPosition(triPoint_B);
		pntC->setPosition(triPoint_C);

		//testPoint->setPosition(0.33f*triPoint_A + 0.33f*triPoint_B + 0.33f*triPoint_C);

		//doing some tests to see if calculations match positions
		//testPoint->setPosition(0.66f*triPoint_A + 0.0f*triPoint_B + 0.33f*triPoint_C); //matches drag position
		//testPoint->setPosition(0.89f*triPoint_A + 1.17f*triPoint_B + 0.72f*triPoint_C); // does not match
		//testPoint->setPosition(0.19f*triPoint_A + 0.71f*triPoint_B + 0.009f*triPoint_C);
		//testPoint->setPosition(0.21f*triPoint_A + 0.73f*triPoint_B + 0.05f*triPoint_C);
		//testPoint->setPosition(0.674f*triPoint_A + 0.198f*triPoint_B + 0.128f*triPoint_C);  //exact match
		//testPoint->setPosition(0.224f*triPoint_A + 0.517f*triPoint_B + 0.260f*triPoint_C); //exact match
		//testPoint->setPosition(0.08f*triPoint_A + 0.152f*triPoint_B + 0.768f*triPoint_C); 

		//testPoint->setPosition(-0.112f*triPoint_A + 0.629f*triPoint_B + 0.484f*triPoint_C);  //test using a negative weight
		testPoint->setPosition(1.174f*triPoint_A + -0.483f*triPoint_B + 0.309f*triPoint_C);  //appears to match
		//testPoint->setPosition(-0.369f*triPoint_A + 1.1139f*triPoint_B + 0.229f*triPoint_C);   //appears  to match fairly close

		genericVector = new_sp<nho::VisualVector>();
		genericVector->bUseCenteredMesh = false;
		genericPoint = new_sp<nho::VisualPoint>();
		text = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");

		////////////////////////////////////////////////////////
		// my method
		////////////////////////////////////////////////////////
		projAnim_BC = new_sp<VectorProjectionAnimation>();
		projAnim_PointOnPerpendicular = new_sp<VectorProjectionAnimation>();

		testPoint->eventValuesUpdated.addWeakObj(event_this(), &Slide_BaryCentricsExplanation::handleTestPointUpdated);
		pntA->eventValuesUpdated.addWeakObj(event_this(), &Slide_BaryCentricsExplanation::handleTestPointUpdated);
		pntB->eventValuesUpdated.addWeakObj(event_this(), &Slide_BaryCentricsExplanation::handleTestPointUpdated);
		pntC->eventValuesUpdated.addWeakObj(event_this(), &Slide_BaryCentricsExplanation::handleTestPointUpdated);

		////////////////////////////////////////////////////////
		// optimized projection method
		////////////////////////////////////////////////////////
		projAnim_ab_onto_cb = new_sp<VectorProjectionAnimation>();
		projAnim_testPointOnPerpendicular = new_sp<VectorProjectionAnimation>();
		projAnim_aBOnPerpendicular = new_sp<VectorProjectionAnimation>();

	}

	void Slide_BaryCentricsExplanation::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);
		using namespace glm;

		if (bWireframe) { ec(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)); }
		triRender->renderTriangle(pntA->getPosition(), pntB->getPosition(), pntC->getPosition(), glm::vec3(0.0f, 0.5f, 0.0f), rd->projection_view);
		if (bWireframe) { ec(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)); }

		if (rd->camera)
		{
			glm::vec3 camPos = rd->camera->getPosition();
			pntA->render(rd->projection_view, camPos);
			pntB->render(rd->projection_view, camPos);
			pntC->render(rd->projection_view, camPos);

			testPoint->color = vec3(1.f, 1.f, 0);
			testPoint->render(rd->projection_view, camPos);

			//genericVector->setStart(rayStart);
			//genericVector->setEnd(rayEnd);
			//genericVector->render(rd->projection_view, rd->camera->getPosition());

			if (QuaternionCamera* camera = dynamic_cast<QuaternionCamera*>(rd->camera))
			{
				auto renderPointText = [&](
					const sp<nho::ClickableVisualPoint>& a
					,const sp<nho::ClickableVisualPoint>& b
					,const sp<nho::ClickableVisualPoint>& c
					,const char* const textStr
					//const sp<ho::TextBlockSceneNode>& text
					) 
				{
					vec3 c_to_a = a->getPosition() - c->getPosition();
					vec3 b_to_a = a->getPosition() - b->getPosition();
					vec3 textOffset = glm::normalize(c_to_a + b_to_a);
					constexpr float textOffsetDist = 0.25f;
					vec3 textPosition = textOffset * textOffsetDist + a->getPosition();

					text->setLocalPosition(textPosition);
					text->setLocalRotation(camera->getRotation());
					text->wrappedText->text = textStr;
					text->setLocalScale(vec3(5.f));
				};
				renderPointText(pntA, pntB, pntC, "(1,0,0)");
				text->render(rd->projection, rd->view);

				renderPointText(pntB, pntC, pntA, "(0,1,0)");
				text->render(rd->projection, rd->view);

				renderPointText(pntC, pntA, pntB, "(0,0,1)");
				text->render(rd->projection, rd->view);

				{
					glm::vec3 barycentrics = vec3(0.f);

					if (barymode == EBarycentricMode::MY_METHOD)
					{
						barycentrics = calcBarycentrics_myMethod(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
					}
					else if (barymode == EBarycentricMode::OPTIMIZED_PROJECTION)
					{
						barycentrics = calcBarycentrics_optimizedProjection(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
					}
					else if (barymode == EBarycentricMode::AREA_METHOD)
					{
						barycentrics = calcBarycentrics_AreaMethod(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
					}
					else if (barymode == EBarycentricMode::LINEAR_SYSTEMS_METHOD)
					{
						barycentrics = calcBarycentrics_LinearSystemMethod(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
					}
					else
					{
						//default ot mymethod if something went wrong
						barycentrics = calcBarycentrics_myMethod(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
					}

					char textBuffer[128];
					/*snprintf(textBuffer, sizeof(textBuffer), "(%3.2f, %3.2f, %3.2f)", barycentrics.x, barycentrics.y, barycentrics.z);*/
					snprintf(textBuffer, sizeof(textBuffer), "(%3.3f, %3.3f, %3.3f)", barycentrics.x, barycentrics.y, barycentrics.z);
					text->wrappedText->text = std::string(textBuffer);

					const float pointOffsetDis = 0.15f;
					text->setLocalPosition(
						testPoint->getPosition()
						//+ camera->getRight()*pointOffsetDis
						+ camera->getUp()*pointOffsetDis
						+ -camera->getFront()*pointOffsetDis);
					text->wrappedText->bitMapFont->setFontColor(vec3(1.f, 1.f, 0.5f));
					text->setLocalScale(vec3(4.f));
					text->render(rd->projection, rd->view);
					text->wrappedText->bitMapFont->setFontColor(vec3(1.0f));

					float movingGroundTruthTextOffset = 0.f; //let multiple truths be displayed by updated this
					if (bRenderShirleyVersion)
					{
						vec3 shirlyBary = Shirley_getBarycentricCoordinatesAt(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition());
						snprintf(textBuffer, sizeof(textBuffer), "(%3.3f, %3.3f, %3.3f)", shirlyBary.x, shirlyBary.y, shirlyBary.z);
						text->wrappedText->text = std::string(textBuffer);
						movingGroundTruthTextOffset += 0.2f;
						glm::vec3 adjustedPosForGroundTruth = text->getLocalPosition() + camera->getUp()*movingGroundTruthTextOffset;
						text->setLocalPosition(adjustedPosForGroundTruth);
						text->render(rd->projection, rd->view);
					}
					if (bRenderRealTimeCollisionBook)
					{
						float u,v,w;
						realtimecollisiondetectionbook_Barycentric(testPoint->getPosition(), pntA->getPosition(), pntB->getPosition(), pntC->getPosition(),u,v,w);

						snprintf(textBuffer, sizeof(textBuffer), "(%3.3f, %3.3f, %3.3f)", u, v, w);
						text->wrappedText->text = std::string(textBuffer);
						movingGroundTruthTextOffset += 0.1f;
						glm::vec3 adjustedPosForGroundTruth = text->getLocalPosition() + camera->getUp()*movingGroundTruthTextOffset;
						text->setLocalPosition(adjustedPosForGroundTruth);
						text->render(rd->projection, rd->view);
					}


					if (barymode == EBarycentricMode::MY_METHOD)
					{
						renderGame_Barycentric_myMethod(dt_sec);
					}
					else if (barymode == EBarycentricMode::OPTIMIZED_PROJECTION)
					{
						renderGame_Barycentric_OptimizedProjectionMethod(dt_sec);
					}
					else if (barymode == EBarycentricMode::AREA_METHOD)
					{
						renderGame_Barycentric_AreaMethod(dt_sec);
					}
					else if (barymode == EBarycentricMode::LINEAR_SYSTEMS_METHOD)
					{
						renderGame_Barycentric_SolvingLinearSystem(dt_sec);
					}
					else
					{
						renderGame_Barycentric_myMethod(dt_sec);
					}

				}

			}
		}

		bTestPointUpdated = false;
	}

	void Slide_BaryCentricsExplanation::renderGame_Barycentric_myMethod(float dt_sec)
	{
		using namespace glm;

		auto renderVectorExplanation = [&](
			const sp<nho::ClickableVisualPoint>& a
			, const sp<nho::ClickableVisualPoint>& b
			, const sp<nho::ClickableVisualPoint>& c)
		{
			if (rd->camera)
			{
				glm::vec3 camPos = rd->camera->getPosition();
				glm::vec3 camOffset = rd->camera->getUp()*0.01f; //create slight offset so that there isn't zfighting on lines

				const vec3 aPos = a->getPosition();
				const vec3 bPos = b->getPosition();
				const vec3 cPos = c->getPosition();

				const vec3 b_to_a = aPos - bPos;
				helper_renderVector(bRenderBToA, bPos + camOffset, b_to_a, vec3(1.f, 0.f, 0.f), timestamp_RenderBToA);

				const vec3 b_to_c = c->getPosition() - bPos;
				helper_renderVector(bRenderBToC, bPos + camOffset, b_to_c, vec3(0.f, 0.f, 1.f), timestamp_RenderBToC);

				const vec3 bc_proj = projectAontoB(b_to_a, b_to_c);
				helper_renderProjection(bRenderBCProj, *projAnim_BC, b_to_a, b_to_c, bPos + camOffset, bPos + camOffset, dt_sec, glm::vec3(1,0,0));

				const vec3 projectionPoint = bPos + bc_proj;
				const vec3 edgeProjPoint_to_TestPoint = testPoint->getPosition() - projectionPoint;
				helper_renderVector(bRender_EdgeProjectPointToTestPoint, projectionPoint + camOffset, edgeProjPoint_to_TestPoint, vec3(1.f, 1.f, 0.f), timestamp_Render_EdgeProjectPointToTestPoint);

				const vec3 perpendicularWithEdgeVec = aPos - projectionPoint; //perpendicular line to triangle edge
				helper_renderVector(bRender_PerpendicularToEdge, projectionPoint + camOffset, perpendicularWithEdgeVec, vec3(0.5f, 0.5f, 0.5f), timestamp_Render_PerpendicularToEdge);

				const vec3 testPoint_projOnTo_perpendicular = projectAontoB(edgeProjPoint_to_TestPoint, perpendicularWithEdgeVec);
				helper_renderProjection(bRenderTestPointProjectionOntoPerpendicular, *projAnim_PointOnPerpendicular, edgeProjPoint_to_TestPoint, perpendicularWithEdgeVec, projectionPoint + camOffset, projectionPoint + camOffset, dt_sec, vec3(1.f, 1.f, 0.f));

				//visual this ratio?
				//TODO this needs to be updated to scalar projection method
				float lengthRatio = glm::length(testPoint_projOnTo_perpendicular) / glm::length(perpendicularWithEdgeVec);
			}
		};

		if (bRenderBarycentricA) { renderVectorExplanation(pntA, pntB, pntC); }
		if (bRenderBarycentricB) { renderVectorExplanation(pntB, pntC, pntA); }
		if (bRenderBarycentricC) { renderVectorExplanation(pntC, pntA, pntB); }
	}

	void Slide_BaryCentricsExplanation::renderGame_Barycentric_OptimizedProjectionMethod(float dt_sec)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Below can be highly optimized by analyzing the actual math symbolically and removing redundancies in calculating projections etc.
		// (see Math for Game Developers - Ray Triangle Intersection by Jorge Rodriguez). I'm more interested
		// in teaching the concept rather than every exact method that has been optimized completely.
		// I may look and see what what this all reduces down to and show it, but at the moment leaving an unoptimized and untested version commented out
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		using namespace glm;

		auto calcBarycentric = [&](
			const vec3& aPos
			, const vec3& bPos
			, const vec3& cPos
			)
		{
			const vec3 ab = bPos - aPos;
			helper_renderVector(bRenderAB, aPos, ab, vec3(1,0,0),timestamp_renderbRenderAB, true);

			const vec3 cb = cPos - bPos;
			helper_renderVector(bRenderCB, bPos, cb, vec3(0, 1, 0),timestamp_renderbRenderCB,true);

			const vec3 a_to_testPoint = testPoint->getPosition() - aPos;
			helper_renderVector(bRender_AtoTestPnt, aPos, a_to_testPoint, vec3(1, 1, 0), timestamp_renderbRender_AtoTestPnt, true);

			const vec3 projectionToBuildPerpendicular = ((glm::dot(ab, cb) / glm::dot(cb, cb)) * cb);
			helper_renderProjection(bRender_ProjToCB, *projAnim_ab_onto_cb, ab, cb, vec3(0.f), vec3(0.f), dt_sec);

			glm::vec3 perpendicular = ab - projectionToBuildPerpendicular;
			helper_renderVector(bRender_VectorFromFirstProjection, projectionToBuildPerpendicular, ab-projectionToBuildPerpendicular, vec3(0.5f), timestamp_renderbRender_VectorFromFirstProjection, true);

			float proj_a_to_testpointFactor = glm::dot(a_to_testPoint, perpendicular) /* /dot(perpendicular,perpendicular)*/;
			helper_renderProjection(bRender_projTestPointOntoPerpendicular, *projAnim_testPointOnPerpendicular, a_to_testPoint, perpendicular, vec3(0.f), vec3(0.f), dt_sec);

			float proj_ab = glm::dot(ab, perpendicular) /* /dot(perpendicular,perpendicular)*/;
			helper_renderProjection(bRender_projABontoPerpendicular, *projAnim_aBOnPerpendicular, ab, perpendicular, vec3(0.f), vec3(0.f), dt_sec, vec3(1.f,0.f,0.f));

			return 1 - (proj_a_to_testpointFactor / proj_ab);
		};

		if (bRenderBarycentricA){ float barycentric_a = calcBarycentric(pntA->getPosition(), pntB->getPosition(), pntC->getPosition());}
		if (bRenderBarycentricB) {float barycentric_b = calcBarycentric(pntB->getPosition(), pntC->getPosition(), pntA->getPosition());}
		if (bRenderBarycentricC) { float barycentric_c = calcBarycentric(pntC->getPosition(), pntA->getPosition(), pntB->getPosition());}

		//glm::vec3 baryCentrics = glm::vec3(barycentric_a, barycentric_b, barycentric_c);
	}

	void Slide_BaryCentricsExplanation::renderGame_Barycentric_AreaMethod(float dt_sec)
	{
		auto renderCrossVecHelper = [this](bool bShouldRender, glm::vec3 first, glm::vec3 second, glm::vec3 start) 
		{
			if (bShouldRender)
			{
				helper_renderVector(bRenderCrossVec_first, start, first, glm::vec3(1, 0, 0), timestamp_crossvecfirst);
				helper_renderVector(bRenderCrossVec_second, start, second, glm::vec3(0, 1, 0), timestamp_crossvecsecond);
			}
		};


		//Graphics book Shirley method for calculating barycentrics using areas from cross product: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
		//tri is formed by a,b,c
		using namespace glm;
		const vec3 a = pntA->getPosition();
		const vec3 b = pntB->getPosition();
		const vec3 c = pntC->getPosition();
		const vec3 pnt = testPoint->getPosition();

		glm::vec3 bary;
		glm::vec3 normal_v = glm::cross(b - a, c - a);
		glm::vec3 normal_n = glm::normalize(normal_v);
		helper_renderVector(bAreaMethod_RenderTriNormals, a, bAreaMethod_normalizeNormals ? normal_n:normal_v, vec3(0.f, 0.f, 1.f), timestamp_area_normals);
		helper_renderVector(bAreaMethod_RenderTriNormals, b, bAreaMethod_normalizeNormals ? normal_n : normal_v, vec3(0.f, 0.f, 1.f), timestamp_area_normals);
		helper_renderVector(bAreaMethod_RenderTriNormals, c, bAreaMethod_normalizeNormals ? normal_n : normal_v, vec3(0.f, 0.f, 1.f), timestamp_area_normals);

		// The area of a triangle is 
		float areaABC = glm::dot(normal_v, glm::cross((b - a), (c - a)));
		helper_renderCrossArea(bAreaMethod_renderFullArea,(b - a), (c - a), a + /*back it up a bit*/normal_n *-0.01f, vec3(0.25f), timestamp_area_fullarea);
		renderCrossVecHelper(bAreaMethod_renderFullArea, (b - a), (c - a), a);

		float areaPBC = glm::dot(normal_v, glm::cross((b - pnt), (c - pnt)));
		helper_renderCrossArea(bAreaMethod_renderPBC_Area, (b - pnt), (c - pnt), pnt, vec3(0.5f, 0.f, 0.f), timestamp_area_PBC_area);
		renderCrossVecHelper(bAreaMethod_renderPBC_Area, (b - pnt), (c - pnt), pnt);

		float areaPCA = glm::dot(normal_v, glm::cross((c - pnt), (a - pnt)));
		helper_renderCrossArea(bAreaMethod_renderPCA_Area, (c - pnt), (a - pnt), pnt, vec3(0.0f, 0.5f, 0.f), timestamp_area_PCA_area);
		renderCrossVecHelper(bAreaMethod_renderPCA_Area, (c - pnt), (a - pnt), pnt);

		/////////////////////////////////////
		//area PAB isn't necessary, but perhaps should render it too?
		float areaPAB = glm::dot(normal_v, glm::cross((a - pnt), (b - pnt)));
		helper_renderCrossArea(bAreaMethod_renderPAB_Area, (a - pnt), (b - pnt), pnt, vec3(0.0f, 0.0f, 0.5f), timestamp_area_PAB_area);
		renderCrossVecHelper(bAreaMethod_renderPAB_Area, (a - pnt), (b - pnt), pnt);
		/////////////////////////////////////

		bary.x = areaPBC / areaABC; // alpha
		bary.y = areaPCA / areaABC; // beta
		bary.z = 1.0f - bary.x - bary.y; // gamma

	}

	void Slide_BaryCentricsExplanation::renderGame_Barycentric_SolvingLinearSystem(float dt_sec)
	{

	}

	void Slide_BaryCentricsExplanation::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Barycentrics review", nullptr, flags);
		{
			//static int baryModeProxy = 0;
			if (ImGui::RadioButton("MyMethod", barymode == EBarycentricMode::MY_METHOD))
			{
				barymode = EBarycentricMode::MY_METHOD;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Optimized Projection", barymode == EBarycentricMode::OPTIMIZED_PROJECTION))
			{
				barymode = EBarycentricMode::OPTIMIZED_PROJECTION;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Area Method", barymode == EBarycentricMode::AREA_METHOD))
			{
				barymode = EBarycentricMode::AREA_METHOD;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Linear Eq Method", barymode == EBarycentricMode::LINEAR_SYSTEMS_METHOD))
			{
				barymode = EBarycentricMode::LINEAR_SYSTEMS_METHOD;
			}

			if(ImGui::Checkbox("bRenderBarycentricA", &bRenderBarycentricA)){bTestPointUpdated=true;} //update test point so we refresh projection anims
			if(ImGui::Checkbox("bRenderBarycentricB", &bRenderBarycentricB)){bTestPointUpdated=true;}
			if(ImGui::Checkbox("bRenderBarycentricC", &bRenderBarycentricC)){bTestPointUpdated=true;}

			ImGui::Separator();

			ImGui::Checkbox("wireframe", &bWireframe);

			ImGui::Separator();
			if (barymode == EBarycentricMode::MY_METHOD)
			{
				if (ImGui::Checkbox("bRenderBToA", &bRenderBToA)) { timestamp_RenderBToA = tickedTime; }
				if (ImGui::Checkbox("bRenderBToC", &bRenderBToC)){ timestamp_RenderBToC = tickedTime; }
				if (ImGui::Checkbox("bRenderBCProj", &bRenderBCProj)){timestamp_RenderBCProj = tickedTime;}
				if (ImGui::Checkbox("bRender_PerpendicularToEdge", &bRender_PerpendicularToEdge)){timestamp_Render_PerpendicularToEdge = tickedTime;}
				if (ImGui::Checkbox("bRender_EdgeProjectPointToTestPoint", &bRender_EdgeProjectPointToTestPoint)){ timestamp_Render_EdgeProjectPointToTestPoint = tickedTime; }
				if (ImGui::Checkbox("bRenderTestPointProjectionOntoPerpendicular", &bRenderTestPointProjectionOntoPerpendicular)){timestamp_RenderTestPointProjectionOntoPerpendicular = tickedTime;}
			}
			else if (barymode == EBarycentricMode::OPTIMIZED_PROJECTION)
			{
				if (ImGui::Checkbox("bRenderAB" 													 , &bRenderAB 								)){timestamp_renderbRenderAB								=tickedTime;		}
				if (ImGui::Checkbox("bRenderCB" 													 , &bRenderCB 								)){timestamp_renderbRenderCB								=tickedTime;		}
				if (ImGui::Checkbox("bRender_AtoTestPnt" 											 , &bRender_AtoTestPnt 						)){timestamp_renderbRender_AtoTestPnt						=tickedTime;		}
				if (ImGui::Checkbox("bRender_ProjToCB" 												 , &bRender_ProjToCB 						)){timestamp_renderbRender_Perpendicular					=tickedTime;	 	}
				if (ImGui::Checkbox("bRender_VectorFromFirstProjection (hint: move A to origin)" 	 , &bRender_VectorFromFirstProjection 		)){timestamp_renderbRender_VectorFromFirstProjection	  	=tickedTime;		}
				if (ImGui::Checkbox("bRender_projTestPointOntoPerpendicular"						 , &bRender_projTestPointOntoPerpendicular	)){timestamp_renderbRender_projTestPointOntoPerpendicular	=tickedTime;		}
				if (ImGui::Checkbox("bRender_projABontoPerpendicular"								 , &bRender_projABontoPerpendicular			)){timestamp_renderbRender_projABontoPerpendicular			=tickedTime;		}
			}
			else if (barymode == EBarycentricMode::AREA_METHOD)
			{
				ImGui::Checkbox("normalize normals", &bAreaMethod_normalizeNormals);
				if(ImGui::Checkbox("bRenderCrossVec_first", &bRenderCrossVec_first))    {timestamp_crossvecfirst  = tickedTime;}
				if(ImGui::Checkbox("bRenderCrossVec_second ", &bRenderCrossVec_second)) {timestamp_crossvecsecond = tickedTime;}
				ImGui::Separator();

				if(ImGui::Checkbox("bAreaMethod_renderFullArea",   &bAreaMethod_renderFullArea  )){timestamp_area_fullarea = tickedTime;}
				if(ImGui::Checkbox("bAreaMethod_renderPBC_Area",   &bAreaMethod_renderPBC_Area  )){timestamp_area_PBC_area = tickedTime;}
				if(ImGui::Checkbox("bAreaMethod_renderPCA_Area",   &bAreaMethod_renderPCA_Area  )){timestamp_area_PCA_area = tickedTime;}
				if(ImGui::Checkbox("bAreaMethod_renderPAB_Area",   &bAreaMethod_renderPAB_Area  )){timestamp_area_PAB_area = tickedTime;}
				if(ImGui::Checkbox("bAreaMethod_RenderTriNormals", &bAreaMethod_RenderTriNormals)){timestamp_area_normals  = tickedTime;}
				if(ImGui::Checkbox("bAreaMethod_RenderCrossProductVectors", &bRenderCrossProductVectors)) { timestamp_crossproductVecs= tickedTime; }
				

				ImGui::Checkbox("Render half xproduct area", &bRenderHalfAreas);
				
			}

			else if (barymode == EBarycentricMode::LINEAR_SYSTEMS_METHOD)
			{

			}


			ImGui::Separator();
			ImGui::Checkbox("Shriley book ground truth", &bRenderShirleyVersion);
			ImGui::Checkbox("Real time collision book ground truth", &bRenderRealTimeCollisionBook);
		}


		ImGui::End();
	}

#define COMPILETIME_SQUARE(val) val * val

	void Slide_BaryCentricsExplanation::tick(float dt_sec)
	{
		using namespace glm;
		SlideBase::tick(dt_sec);

		//if test point is not within some distance to plane, move it to triangle plane
		{
			vec3 toTestPoint = testPoint->getPosition() - pntA->getPosition();

			vec3 edge1 = pntB->getPosition() - pntA->getPosition();
			vec3 edge2 = pntC->getPosition() - pntA->getPosition();
			vec3 normal = glm::cross(edge1, edge2);

			vec3 project = projectAontoB(toTestPoint, normal);
			float distFromPlane2 = glm::length2(project);
			constexpr float distTolerance2 = COMPILETIME_SQUARE(0.01f);
			if (distFromPlane2 > distTolerance2)
			{
				glm::vec3 newPos = testPoint->getPosition();
				newPos = newPos + -project; //may need to do some sign comparison here
				if (!anyValueNAN(newPos))
				{
					testPoint->setPosition(newPos);
				}
			}
		}

		projAnim_BC->tick(dt_sec);
		projAnim_PointOnPerpendicular->tick(dt_sec);
		projAnim_ab_onto_cb->tick(dt_sec);
		projAnim_testPointOnPerpendicular->tick(dt_sec);
		projAnim_aBOnPerpendicular->tick(dt_sec);

		tickedTime += dt_sec;
	}


	void Slide_BaryCentricsExplanation::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&pntA->pointCollision->getTriangleList());
		objectList.push_back(&pntB->pointCollision->getTriangleList());
		objectList.push_back(&pntC->pointCollision->getTriangleList());

		objectList.push_back(&testPoint->pointCollision->getTriangleList());
	}

	void Slide_BaryCentricsExplanation::handleTestPointUpdated(const nho::VisualPoint& pnt)
	{
		bTestPointUpdated = true;
	}



	void Slide_BaryCentricsExplanation::helper_renderVector(bool bShouldRender, glm::vec3 start, glm::vec3 dir, glm::vec3 color)
	{
		using namespace glm;

		if(bShouldRender)
		{
			genericVector->setStart(start);
			genericVector->setVector(dir);
			genericVector->color = color;
			genericVector->render(rd->projection_view, rd->camera ? rd->camera->getPosition() : vec3(0.f));
		}
	}

	void Slide_BaryCentricsExplanation::helper_renderVector(bool bShouldRender, glm::vec3 start, glm::vec3 dir, glm::vec3 color, float timestampSecs, bool bDriftToOrigin)
	{
		using namespace glm;

		if (bShouldRender)
		{
			if (bDriftToOrigin)
			{
				//drift starts after animation is complete, and lasts duration of animation
				//float adjustedTimeStamp = timestampSecs - vectorAnimSecs; //pull animation back by enough time for vector to grow
				//float flippedDriftPerc = tickedTime < timestampSecs + vectorAnimSecs ? 0 : (1 - calcPerc(timestampSecs+vectorAnimSecs,tickedTime,vectorAnimSecs));
				float flippedDriftPerc = 1 - clampPerc(timestampSecs + vectorAnimSecs, tickedTime, vectorAnimSecs);
				genericVector->setStart(start * flippedDriftPerc);
			}
			else
			{
				genericVector->setStart(start);
			}
			genericVector->setVector(dir * clampPerc(timestampSecs, tickedTime, vectorAnimSecs));
			genericVector->color = color;
			genericVector->render(rd->projection_view, rd->camera ? rd->camera->getPosition() : glm::vec3(0.f));
		}
	}

	void Slide_BaryCentricsExplanation::helper_renderProjection(bool bShouldRender, nho::VectorProjectionAnimation& projAnim, glm::vec3 aVec, glm::vec3 bVec, glm::vec3 aStart, glm::vec3 bStart, float dt_sec, std::optional<glm::vec3> color)
	{
		using namespace glm;

		if (bShouldRender)
		{
			if (!projAnim.isAnimating() || bTestPointUpdated)
			{
				projAnim.setColor(vec3(1.f, 1.f, 0.f));
				projAnim.projectFromAtoB(aVec, bVec, aStart, bStart, !bTestPointUpdated);//only reset anim if test point wasn't updated
				projAnim.tick(dt_sec * 0.001f);

				if (color.has_value())
				{
					projAnim.setColor(*color);
				}
			}
			projAnim.render(rd->projection_view, rd->camera ? rd->camera->getPosition() : glm::vec3(0.f)); //prevent flickering as it hasn't been ticked yet
		}
		else
		{
			projAnim.setShouldRender(false);
		}
	}

	void Slide_BaryCentricsExplanation::helper_renderCrossArea(bool bShouldRender, glm::vec3 first, glm::vec3 second, glm::vec3 start, glm::vec3 color, float timestamp_start)
	{
		if (bShouldRender)
		{
			float perc = clampPerc(timestamp_start, tickedTime, vectorAnimSecs);

			float fistHalfPerc = glm::clamp(perc, 0.f, 0.5f) / 0.5f;
			float secondHalfPerc = glm::clamp(perc-0.5f, 0.f, 0.5f) / 0.5f;

			triRender->renderTriangle(start, start + first* fistHalfPerc, start + second* fistHalfPerc, color, rd->projection_view);

			//second triangle
			glm::vec3 second_StartA = start + first;
			glm::vec3 second_StartB = start + second;

			if (!bRenderHalfAreas)
			{
				glm::vec3 second_End = second_StartA + second;
				glm::vec3 proj_ontoAB = projectAontoB(second_End-second_StartA,second_StartB-second_StartA); //similar to projection method of barycentrics, find a perpendicular to a point on a triangle -- then we're sliding up along that perpendcular
				glm::vec3 perpStartPoint = second_StartA + proj_ontoAB;
				glm::vec3 perpendicular_v = second_End - perpStartPoint;

				triRender->renderTriangle(second_StartA, second_StartB, perpStartPoint + secondHalfPerc*perpendicular_v , color, rd->projection_view);
			}

			glm::vec3 crossResult = glm::cross(first, second);
			helper_renderVector(bRenderCrossProductVectors, start, crossResult, color);

		}
	}

	////////////////////////////////////////////////////////
	// Moller Trumbore ray tri intersection
	////////////////////////////////////////////////////////
	void Slide_MollerAndTrumbore::init()
	{
		SlideBase::init();

		triRender = new_sp<ho::ImmediateTriangle>();
		genericVector = new_sp<nho::VisualVector>();
		genericVector->bUseCenteredMesh = false;
		genericPoint = new_sp<nho::VisualPoint>();
	}


	void Slide_MollerAndTrumbore::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		triRender->renderTriangle(triPoint_A, triPoint_B, triPoint_C, glm::vec3(1.f, 0, 0), rd->projection_view);

		if (rd->camera)
		{
			genericVector->setStart(rayStart);
			genericVector->setEnd(rayEnd);
			genericVector->render(rd->projection_view, rd->camera->getPosition());
		}
	}

	void Slide_MollerAndTrumbore::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Trumbore and Moller", nullptr, flags);
		{
			if (ImGui::Button("run ray tri intersection"))
			{
				doTrace();
			}
		}
		ImGui::End();
	}

	void Slide_MollerAndTrumbore::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);
	}

	void Slide_MollerAndTrumbore::doTrace()
	{
		using namespace glm;
		
		rayStart = rd->camera->getPosition();
		vec3 rayDir = rd->camera->getFront();


		double orig[3];
		double dir[3];
		double vert0[3];
		double vert1[3];
		double vert2[3];
		double t, u, v; //barycentrics

		auto VEC3_TO_ARRAY = [](glm::vec3 vec, double* c_array)
		{
			c_array[0] = double(vec.x);
			c_array[1] = double(vec.y);
			c_array[2] = double(vec.z);
		};

		VEC3_TO_ARRAY(rayStart, orig);
		VEC3_TO_ARRAY(rayDir, dir);
		VEC3_TO_ARRAY(triPoint_A, vert0);
		VEC3_TO_ARRAY(triPoint_B, vert1);
		VEC3_TO_ARRAY(triPoint_C, vert2);

		//if (moller_trumbore_intersect_triangle_original(orig, dir, vert0, vert1, vert2, &t, &u, &v))
		if (moller_trumbore_ray_triangle_commented_renamed(orig, dir, vert0, vert1, vert2, &t, &u, &v))
		{
			std::cout << "hit the triangle with moller_trumbore" << std::endl;
			rayEnd = float(t) * rayDir + rayStart;
		}
		else
		{
			std::cout << "missed moller_trumbore" << std::endl;
			rayEnd = 10.f * rayDir + rayStart;
		}
	}





	void Slide_MollerAndTrumbore::liveCodingRecorded()
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

		//note barycentrics are not exactly like weights
		//but behave like weights when point is within tri
		vec3 barycentrics(0.33f, 0.33f, 0.33f);

		vec3 baryTestPoint = barycentrics.r * triPoint_A
			+ barycentrics.g *triPoint_B
			+ barycentrics.b *triPoint_C;















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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// scalar triple product review
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Slide_ScalarTripleProductReview::init()
	{
		SlideBase::init();

		aVec = new_sp<ClickableVisualVector>();
		bVec = new_sp<ClickableVisualVector>();
		cVec = new_sp<ClickableVisualVector>();
		crossVecVisual = new_sp<ClickableVisualVector>();

		aVec->bUseCenteredMesh = false;
		bVec->bUseCenteredMesh = false;
		cVec->bUseCenteredMesh = false;

		projAnim = new_sp<nho::VectorProjectionAnimation>();

		aVec->setVector(glm::vec3(1, 0.f, 0));
		bVec->setVector(glm::vec3(0, 1, 0));
		cVec->setVector(glm::normalize(glm::vec3(1, 1, 1)));
		crossVecVisual->setVector(glm::cross(aVec->getVec(), bVec->getVec()));
		crossVecVisual->color = glm::vec3(0, 0, 1);

		crossProductText = new_sp<ho::TextBlockSceneNode>(RayTriDemo::font, "0.f");

		triRender = new_sp<ho::ImmediateTriangle>();
		lineRenderer = new_sp<ho::LineRenderer>();
	}

	void Slide_ScalarTripleProductReview::inputPoll(float dt_sec)
	{
		SlideBase::inputPoll(dt_sec);
	}

	void Slide_ScalarTripleProductReview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);
		timePassedSec += dt_sec;

		using namespace glm;

		//just ignore any color setting by selection so that it is always red x green = blue
		aVec->color = glm::vec3(1, 0, 0);
		bVec->color = glm::vec3(0, 1, 0);
		cVec->color = glm::vec3(1, 1, 0);
		if (bNormalizeSourceVecs)
		{
			aVec->setVector(glm::normalize(aVec->getVec()));
			bVec->setVector(glm::normalize(bVec->getVec()));
			cVec->setVector(glm::normalize(cVec->getVec()));
		}

		projAnim->tick(dt_sec);
	}

	void Slide_ScalarTripleProductReview::render_game(float dt_sec)
	{
		using namespace glm;
		SlideBase::render_game(dt_sec);

		TickData td;
		td.dt_sec = dt_sec;
		td.tickedTime = timePassedSec;

		glm::vec3 camPos = rd->camera->getPosition();

		aVec->render(rd->projection_view, camPos);
		bVec->render(rd->projection_view, camPos);
		cVec->render(rd->projection_view, camPos);

		glm::vec3 crossVec = glm::cross(aVec->getVec(), bVec->getVec());
		if (bNormalizeCrossResult)
		{
			crossVec = glm::normalize(crossVec);
		}

		if (bShowCrossProduct)
		{
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

			//render cross product
			if (bShowLength)
			{
				crossProductText->render(rd->projection, rd->view);
			}
			crossVecVisual->render(rd->projection_view, camPos);

		}
		////////////////////////////////////////////////////////
		//render cross area
		////////////////////////////////////////////////////////
		vec3 pipedPointI = aVec->getStart();						//piped point
		vec3 pipedPointII = aVec->getStart() + aVec->getVec();	//piped point
		vec3 pipedPointIII = aVec->getStart() + bVec->getVec();	//piped point
		vec3 pipedPointIV = pipedPointII + bVec->getVec();			//piped point
		AnimationHelperFunctions::renderCrossArea(bRenderArea, td, pipedPointII, pipedPointIII, /*start*/pipedPointI, /*color*/glm::vec3(0.25f,0.25f,0.5f),timestamp_showArea, /*animDur*/ 1.f, *triRender, *rd);

		////////////////////////////////////////////////////////
		//render projection anim
		////////////////////////////////////////////////////////
		td.timestamp_start = timestamp_projection;
		AnimationHelperFunctions::renderProjection(
			bProjectC, td, *projAnim, cVec->getVec(), crossVec, cVec->getStart(), cVec->getStart(), /*animDurSec*/1.f,/*something_moved*/ bResetAnimProjection, /*forceUPdate*/ true,*rd, /*color*/vec3(1,1,0)
		);
		bResetAnimProjection = false; //clear anim reset

		////////////////////////////////////////////////////////
		//render parallelepiped
		////////////////////////////////////////////////////////
		if (bShowParallelpiped)
		{
			float pipedAnimDurSec = 0.5f;
			float growthAlpha = clampPerc(timestamp_showPiped, timePassedSec, pipedAnimDurSec);
			vec3 shiftUp = growthAlpha * cVec->getVec();

			vec3 pipedColor = vec3(0.5f, 0.5f, 0);
			lineRenderer->renderLine(pipedPointI			, pipedPointII, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointI+shiftUp, pipedPointII+shiftUp, pipedColor, rd->projection_view);

			lineRenderer->renderLine(pipedPointI			 , pipedPointIII			, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointI + shiftUp, pipedPointIII + shiftUp, pipedColor, rd->projection_view);

			lineRenderer->renderLine(pipedPointII,			pipedPointIV, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointII + shiftUp, pipedPointIV + shiftUp, pipedColor, rd->projection_view);

			lineRenderer->renderLine(pipedPointIII			, pipedPointIV, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointIII + shiftUp , pipedPointIV + shiftUp, pipedColor, rd->projection_view);

			lineRenderer->renderLine(pipedPointI , pipedPointI + shiftUp, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointII , pipedPointII + shiftUp, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointIII, pipedPointIII + shiftUp, pipedColor, rd->projection_view);
			lineRenderer->renderLine(pipedPointIV, pipedPointIV + shiftUp, pipedColor, rd->projection_view);

			if (bRenderCrossAreaSweeping)
			{
				float growthAlpha = clampPerc(timestamp_areaSweep, timePassedSec, pipedAnimDurSec);
				vec3 sweepUpProgress = growthAlpha * cVec->getVec();
				AnimationHelperFunctions::renderCrossArea(bRenderArea, td, pipedPointII, pipedPointIII, /*start*/pipedPointI + sweepUpProgress, /*color*/glm::vec3(0.25f, 0.25f, 0.5f), timestamp_showArea, /*animDur*/ 1.f, *triRender, *rd);
			}
		}

	}

	void Slide_ScalarTripleProductReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}
		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Scalar Triple Product Review", nullptr, flags);
		ImGui::Checkbox("show crossproduct", &bShowCrossProduct);
		if (ImGui::Checkbox("show area", &bRenderArea)) { timestamp_showArea = timePassedSec; }
		ImGui::Checkbox("normalize cross result", &bNormalizeCrossResult);
		if (ImGui::Checkbox("project third vec onto cross product result", &bProjectC))
		{ 
			timestamp_projection = timePassedSec; 
			bResetAnimProjection = true; 
		}
		if (ImGui::Checkbox("show parallelepiped", &bShowParallelpiped)) { timestamp_showPiped = timePassedSec; }
		if (ImGui::Checkbox("sweep area", &bRenderCrossAreaSweeping)) { timestamp_areaSweep= timePassedSec; }
		
		ImGui::Separator();
		ImGui::Checkbox("normalize src vecs", &bNormalizeSourceVecs);
		ImGui::Checkbox("show value", &bShowLength);
		ImGui::End();
	}

	void Slide_ScalarTripleProductReview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

		objectList.push_back(&aVec->endCollision->getTriangleList());
		objectList.push_back(&bVec->endCollision->getTriangleList());
		objectList.push_back(&cVec->endCollision->getTriangleList());

		//objectList.push_back(&bVec->startCollision->getTriangleList());
		//objectList.push_back(&aVec->startCollision->getTriangleList());
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// anim helpers
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AnimationHelperFunctions::renderVector(
		bool bShouldRender,
		const TickData& td,
		glm::vec3 start,
		glm::vec3 dir,
		glm::vec3 color,
		VisualVector& genericVector,
		const WindowManager::FrameRenderData& rd
	)
	{
		using namespace glm;

		if (bShouldRender)
		{
			genericVector.setStart(start);
			genericVector.setVector(dir);
			genericVector.color = color;
			genericVector.render(rd.projection_view, rd.camera ? rd.camera->getPosition() : vec3(0.f));
		}
	}

	void AnimationHelperFunctions::renderVector(
		bool bShouldRender,
		const TickData& td,
		glm::vec3 start,
		glm::vec3 dir,
		glm::vec3 color,
		float timestampSecs,
		float animDurSecs,
		VisualVector& genericVector,
		const WindowManager::FrameRenderData& rd,
		bool bDriftToOrigin /*= false*/
	)
	{
		using namespace glm;

		if (bShouldRender)
		{
			if (bDriftToOrigin)
			{
				//drift starts after animation is complete, and lasts duration of animation
				//float adjustedTimeStamp = timestampSecs - vectorAnimSecs; //pull animation back by enough time for vector to grow
				//float flippedDriftPerc = tickedTime < timestampSecs + vectorAnimSecs ? 0 : (1 - calcPerc(timestampSecs+vectorAnimSecs,tickedTime,vectorAnimSecs));
				float flippedDriftPerc = 1 - clampPerc(timestampSecs + animDurSecs, td.tickedTime, animDurSecs);
				genericVector.setStart(start * flippedDriftPerc);
			}
			else
			{
				genericVector.setStart(start);
			}
			genericVector.setVector(dir * clampPerc(timestampSecs, td.tickedTime, animDurSecs));
			genericVector.color = color;
			genericVector.render(rd.projection_view, rd.camera ? rd.camera->getPosition() : glm::vec3(0.f));
		}
	}

	void AnimationHelperFunctions::renderProjection(
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
		std::optional<glm::vec3> color /*= std::nullopt*/
	)
	{
		using namespace glm;

		if (bShouldRender)
		{
			if (!projAnim.isAnimating() || bForceUpdate)
			{
				projAnim.projectFromAtoB(aVec, bVec, aStart, bStart, bResetAnim);//only reset anim if test point wasn't updated
				projAnim.tick(td.dt_sec * 0.0001f);

				if (color.has_value())
				{
					projAnim.setColor(*color);
				}
			}
			projAnim.render(rd.projection_view, rd.camera ? rd.camera->getPosition() : glm::vec3(0.f)); //prevent flickering as it hasn't been ticked yet
		}
		else
		{
			projAnim.setShouldRender(false);
		}
	}

	void AnimationHelperFunctions::renderCrossArea(
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
		bool bRenderHalfAreas /*= false*/
	)
	{
		if (bShouldRender)
		{
			float perc = clampPerc(timestamp_start, td.tickedTime, animDurSecs);

			float fistHalfPerc = glm::clamp(perc, 0.f, 0.5f) / 0.5f;
			float secondHalfPerc = glm::clamp(perc - 0.5f, 0.f, 0.5f) / 0.5f;

			triRender.renderTriangle(start, start + first * fistHalfPerc, start + second * fistHalfPerc, color, rd.projection_view);

			//second triangle
			glm::vec3 second_StartA = start + first;
			glm::vec3 second_StartB = start + second;

			if (!bRenderHalfAreas)
			{
				glm::vec3 second_End = second_StartA + second;
				glm::vec3 proj_ontoAB = projectAontoB(second_End - second_StartA, second_StartB - second_StartA); //similar to projection method of barycentrics, find a perpendicular to a point on a triangle -- then we're sliding up along that perpendcular
				glm::vec3 perpStartPoint = second_StartA + proj_ontoAB;
				glm::vec3 perpendicular_v = second_End - perpStartPoint;

				triRender.renderTriangle(second_StartA, second_StartB, perpStartPoint + secondHalfPerc * perpendicular_v, color, rd.projection_view);
			}

			//glm::vec3 crossResult = glm::cross(first, second);
		}
	}
	////////////////////////////////////////////////////////
	// Determinant Review
	////////////////////////////////////////////////////////

	void Slide_DeterminantReview::gatherInteractableCubeObjects(std::vector<const TriangleList_SNO*>& objectList)
	{
		SlideBase::gatherInteractableCubeObjects(objectList);

	}

	void Slide_DeterminantReview::init()
	{
		SlideBase::init();

		triRender = new_sp<ho::ImmediateTriangle>();
		genericVector = new_sp<nho::VisualVector>();
		genericVector->bUseCenteredMesh = false;
		genericPoint = new_sp<nho::VisualPoint>();

		matrix = new_sp<nho::Matrix_3D>();
		matrix->setElements<float>({ {0,1,2,3},{4,5,6,7},{8,9,10,11},{12,13,14,15} });

	}

	void Slide_DeterminantReview::render_game(float dt_sec)
	{
		SlideBase::render_game(dt_sec);

		matrix->render(dt_sec, rd->projection, rd->view);
	}

	void Slide_DeterminantReview::render_UI(float dt_sec)
	{
		SlideBase::render_UI(dt_sec);

		static bool bFirstDraw = true;
		if (bFirstDraw)
		{
			bFirstDraw = false;
			ImGui::SetNextWindowPos({ 1000, 0 });
		}

		ImGuiWindowFlags flags = 0;
		ImGui::Begin("Determinant Review", nullptr, flags);
		{

		}
		ImGui::End();

	}

	void Slide_DeterminantReview::tick(float dt_sec)
	{
		SlideBase::tick(dt_sec);
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