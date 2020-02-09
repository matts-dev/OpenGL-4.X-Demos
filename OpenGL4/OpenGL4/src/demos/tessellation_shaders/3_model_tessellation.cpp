#include<iostream>
#include<string>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>

#include <imgui.1.69.gl/imgui.h>
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../new_utils/header_only/static_mesh.h"
#include "../../new_utils/header_only/simple_quaternion_camera.h"


namespace
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opengl 4.3 style of error checking
	static void APIENTRY openGLErrorCallback_4_3(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		auto typeToText = [](GLenum type)
		{
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR: return "GL_DEBUG_TYPE_ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY: return "GL_DEBUG_TYPE_PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: return "GL_DEBUG_TYPE_PERFORMANCE";
				default: return "unknown type string";
			}
		};

		auto severityToText = [](GLenum severity)
		{
			switch (severity)
			{
				case GL_DEBUG_SEVERITY_HIGH: return "GL_DEBUG_SEVERITY_HIGH";
				case GL_DEBUG_SEVERITY_MEDIUM: return "GL_DEBUG_SEVERITY_MEDIUM";
				case GL_DEBUG_SEVERITY_LOW: return "GL_DEBUG_SEVERITY_LOW";
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "GL_DEBUG_SEVERITY_NOTIFICATION";
				default: return "unknown";
			}
		};

		if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || type == GL_DEBUG_TYPE_PORTABILITY || type == GL_DEBUG_TYPE_PERFORMANCE)
		{
			std::cout << "OpenGL Error : " << message << std::endl;
			std::cout << "source : " << source << "\ttype : " << typeToText(type) << "\tid : " << id << "\tseverity : " << severityToText(severity) << std::endl;
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void verifyShaderCompiled(const char* shadername, GLuint shader)
	{
		GLint success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			constexpr int size = 512;
			char infolog[size];

			glGetShaderInfoLog(shader, size, nullptr, infolog);
			std::cerr << "shader failed to compile: " << shadername << infolog << std::endl;
			exit(-1);
		}
	}

	void verifyShaderLink(GLuint shaderProgram)
	{
		GLint success = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			constexpr int size = 1024;
			char infolog[size];

			glGetProgramInfoLog(shaderProgram, size, nullptr, infolog);
			std::cerr << "SHADER LINK ERROR: " << infolog << std::endl;
			exit(-1);
		}

	}

	inline bool anyValueNAN(glm::vec3 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z); }
	inline bool anyValueNAN(glm::vec4 vec) { return glm::isnan(vec.x) || glm::isnan(vec.y) || glm::isnan(vec.z) || glm::isnan(vec.w); };
	inline bool anyValueNAN(glm::quat quat)
	{
		glm::bvec4 vec = glm::isnan(quat);
		return vec.x || vec.y || vec.z || vec.w;
	};
#if _DEBUG && _WIN32
#include <intrin.h>

#define NAN_BREAK(value)\
if(anyValueNAN(value))\
{\
	__debugbreak();\
}
#else
#define NAN_BREAK(value) 
#endif //_DEBUG

	void true_main()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		int windowWidth = 1200, windowHeight = 800;
		GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "OpenglContext", nullptr, nullptr);
		if (!window)
		{
			std::cerr << "failed to create window" << std::endl;
			return;
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "failed to initialize glad with processes " << std::endl;
			return;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set up ui
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO imguiIO = ImGui::GetIO();
		ImGui_ImplOpenGL3_Init("#version 330 core"); //example puts this after window binding, but seems okay
		ImGui_ImplGlfw_InitForOpenGL(window, /*install callbacks*/true); //false will require manually calling callbacks in our own handlers
		ImGui::GetStyle().ScaleAllSizes(0.5f);


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set up window
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		glViewport(0, 0, windowWidth, windowHeight);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow*window, int width, int height) {  glViewport(0, 0, width, height); });

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(&openGLErrorCallback_4_3, nullptr);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create the verts
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		StaticMesh::Model satelliteModel("./assets/models/satellite/GroundSatellite.obj");
		StaticMesh::Model nanosuitModel("./assets/models/nanosuit/nanosuit.obj");
		StaticMesh::Model manModel("./assets/models/animtutorial/boblampclean.md5mesh", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
		StaticMesh::Model* targetModel = &satelliteModel;
		int selectedMeshIdx = 0;
		glm::mat4 meshModel_m{ 1.f };
		auto refreshMesh = [&]()
		{
			switch (selectedMeshIdx)
			{
				case 1: 
				{
					targetModel = &nanosuitModel; 
					meshModel_m = glm::scale(glm::mat4(1.f), glm::vec3(0.15f));
					break;
				}
				case 2:
				{
					targetModel = &manModel; 
					meshModel_m = glm::toMat4(angleAxis(glm::radians<float>(-90), glm::vec3(1, 0, 0)));
					meshModel_m = glm::scale(meshModel_m, glm::vec3(0.05f));
					break;

				}
				case 0:
				default: 
				{
					targetModel = &satelliteModel; 
					meshModel_m = glm::scale(glm::mat4(1.f), glm::vec3(1.f));
					break;
				}
			}
		};
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// tessellation parameters
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		GLint maxPatchVerts = 0;
		glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVerts);
		std::cout << "max patch verts " << maxPatchVerts << "\n"; //32 on my nvidia based machine; 32 is minimum

		GLint maxTessPatchComponents = 0;
		glGetIntegerv(GL_MAX_TESS_PATCH_COMPONENTS, &maxTessPatchComponents);
		std::cout << "max patch components " << maxTessPatchComponents << "\n"; // num components active per vertex. 120 is min required by opengl

		GLint maxTessOutputComps = 0;
		glGetIntegerv(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, &maxTessOutputComps);
		std::cout << "max patch output components " << maxTessOutputComps << "\n"; //see khronos page for how to calculate current output comp, eg its active-per-vert-components*output_vert+number-active_per_patch_components. opengl min is 4096

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create the shaders
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		enum SpacingType { EQUAL_SPACING, FRACTIONAL_EVEN_SPACING, FRACTIONAL_ODD_SPACING };
		enum HandednessType { CCW, CW };

		//below are ints to work with ImGUI checkboxes
		int spacing = SpacingType::EQUAL_SPACING;
		int handedness = HandednessType::CCW;
		bool bEnablePointMode = false;
		bool bUseLargeValues = false;

		GLuint vertexShader = 0;
		GLuint tessControlShader = 0;
		GLuint tessEvaluationShader = 0;
		GLuint fragShader = 0;
		GLuint shaderProg = 0;

		auto buildTessellationShaders = [&]() {
			const char* vertex_shader_src = R"(
					#version 410 core

					layout (location = 0) in vec3 position;				
					layout (location = 1) in vec3 normal;
					layout (location = 2) in vec2 uv;
					//layout (location = 3) in vec3 tangent;
					//layout (location = 4) in vec3 bitangent;

					out vec3 pos_tcs_in;
					out vec3 normal_tcs_in;
					out vec2 uv_tcs_in;
					//out vec3 tangent_tcs_in;
					//out vec3 bitangent_tcs_in;

					void main(){
						//note, when doing tessellation, you should wait to apply clip space projection until after tessellation
						//NOTICE: we do not write to gl_Position 

						pos_tcs_in = position;					//do not perspective divide this yet, wait until TES
						normal_tcs_in = normalize(normal);		//if you do any transformations of normal here, it is a good idea to renormalize. interpolation will happen TES and geometry shader
						uv_tcs_in = uv;

						//enable this code for normal mapping
						//tangent_tcs_in = tangent;				//my model system normalizes these upon when loading
						//bitangent_tcs_in = bitangent;			//my model system normalizes these upon when loading
					}
				)";
			//-------- TCS provided input------------
			//in int gl_PatchVerticesIn;	//num verts in input patch
			//in int gl_PrimitiveID;		//index of current patch within rendering command
			//in int gl_InvocationID		//index of TCS invocation (eg which vert it is operating on)
			//in gl_PerVertex				//input from vert shader
			//{
			//  vec4 gl_Position;
			//  float gl_PointSize;
			//  float gl_ClipDistance[];
			//} gl_in[gl_MaxPatchVertices];

			//-------------provided outputs ------------
			//patch out float gl_TessLevelOuter[4];
			//patch out float gl_TessLevelInner[2];
			const char* tessellation_control_shader_src = R"(
					#version 410 core
					layout (vertices = 1) out;	//define OUTPUT patch_size (ie the control points), must be less than patch size limit. TCS executes the number of times matching output patch size(ie n output patch = n invocations.
				
					in vec3 pos_tcs_in[];
					in vec3 normal_tcs_in[];
					in vec2 uv_tcs_in[];

					//notation: (300) and (120) defines the cp's derivation verts. original verts are in the notation positions (ABC). 
					//(300) means it took 3As, 0Bs, 0Cs. Thus the original edge verts are (300), (030),(003).
					//(210) means a control point comes from 2As, 1B, and 0Cs. All control points in the PN scheme. When using edges to calc mid points
					//(210) will be closer to a, which means it is a + (1/3) of the edge between a-to-b (not 2/3, as that is further away)
					// sum up to 3. w_pos == world position.
					//this is mostly the control points, plus a bit extra.
					struct ControlPoints_OutputPatch
					{
						vec3 w_pos_300;
						vec3 w_pos_210;
						vec3 w_pos_201;

						vec3 w_pos_030;
						vec3 w_pos_120;
						vec3 w_pos_021;

						vec3 w_pos_003;
						vec3 w_pos_012;
						vec3 w_pos_102;

						vec3 w_pos_111;	//the center point

						vec3 normal[3];
						vec2 uv[3];
					};
					out patch ControlPoints_OutputPatch outPatch; //notice the patch keyword

					uniform float innerTessLevel[2];
					uniform float outerTessLevels[4];
					uniform bool bUseSingleGlobalTL = false;
					uniform bool bUseSingleOuterTL = true;
					uniform float centerControlPointOffsetDivisor = 2.0f;


					float getTessLevel(uint outerIndex) 
					{
						//TL's are roughly how many things to subdivide an edge into
						if(bUseSingleGlobalTL) return innerTessLevel[0];
						else return outerTessLevels[bUseSingleOuterTL ? 0 : outerIndex];
					}

					vec3 projectPntToNormalPlane(vec3 pnt, vec3 normal, vec3 normalPnt)
					{
						vec3 toPnt_v = pnt - normalPnt;
						vec3 proj_v = dot(toPnt_v, normal) * normal; //projection will face wrong direction
						vec3 finalPnt = pnt + -proj_v; //not this can be simplified to (pnt - proj)
						return finalPnt;
					}

					void main()
					{
						for(int cp = 0; cp < 3; ++cp)
						{
							outPatch.normal[cp] = normal_tcs_in[cp];
							outPatch.uv[cp] = uv_tcs_in[cp];
						}

						//set the corner positions
						outPatch.w_pos_300 = pos_tcs_in[0];
						outPatch.w_pos_030 = pos_tcs_in[1];
						outPatch.w_pos_003 = pos_tcs_in[2];

						//edge is associated with vertex opposite from it. I defined this in CCW order. edges vecs point toward last point.
						vec3 edge_030_to_003 = outPatch.w_pos_003 - outPatch.w_pos_030;
						vec3 edge_003_to_300 = outPatch.w_pos_300 - outPatch.w_pos_003;
						vec3 edge_300_to_030 = outPatch.w_pos_030 - outPatch.w_pos_300;

						// calculate mid points along the edges; these are not final positions. They are an intermediate position that will
						// be projected onto the plane of the normal of the corner vertices.
							//outPatch.w_pos_210 =  (0.335.f * outPatch.w_pos_030) + (0.665f * outPatch.w_pos_030); //blending points will work, but using edges means less complex ALU ops. blend_pnt=1+,2*. edge=1-,1*
						outPatch.w_pos_210 = outPatch.w_pos_300 + (1.0/3.0f)*edge_300_to_030;
						outPatch.w_pos_201 = outPatch.w_pos_300 - (1.0/3.0f)*(edge_003_to_300); //negate to flip edge
					
						outPatch.w_pos_021 = outPatch.w_pos_030 + (1.0/3.0f)*(edge_030_to_003);
						outPatch.w_pos_120 = outPatch.w_pos_030 - (1.0/3.0f)*(edge_300_to_030); //distribute -1 out of `pnt + (2/3)(-1edge)`

						outPatch.w_pos_102 = outPatch.w_pos_003 + (1.0/3.0f)*(edge_003_to_300);
						outPatch.w_pos_012 = outPatch.w_pos_003 - (1.0/3.0f)*(edge_030_to_003);
	
						//projection onto planes created by normals at corner vertices
						outPatch.w_pos_210 = projectPntToNormalPlane(outPatch.w_pos_210, outPatch.normal[0], outPatch.w_pos_300);
						outPatch.w_pos_201 = projectPntToNormalPlane(outPatch.w_pos_201, outPatch.normal[0], outPatch.w_pos_300);
					
						outPatch.w_pos_021 = projectPntToNormalPlane(outPatch.w_pos_021, outPatch.normal[1], outPatch.w_pos_030);
						outPatch.w_pos_120 = projectPntToNormalPlane(outPatch.w_pos_120, outPatch.normal[1], outPatch.w_pos_030);

						outPatch.w_pos_102 = projectPntToNormalPlane(outPatch.w_pos_102, outPatch.normal[2], outPatch.w_pos_003);
						outPatch.w_pos_012 = projectPntToNormalPlane(outPatch.w_pos_012, outPatch.normal[2], outPatch.w_pos_003);

						//calculate center point. this will be some distance up from the flat center of tri. dist: We create plane of mid points, then from center of that plane go up 1/2 distance of the vector from flat_tri_center to mid_plane_center.
						vec3 triCenter = (outPatch.w_pos_300 + outPatch.w_pos_030 + outPatch.w_pos_003) / 3.0f; //barycentric coordiantes to get center
						outPatch.w_pos_111 = (outPatch.w_pos_210 + outPatch.w_pos_201
											+ outPatch.w_pos_021 + outPatch.w_pos_120
											+ outPatch.w_pos_102 + outPatch.w_pos_012) / 6.f; //average all the generated cps
						outPatch.w_pos_111 += (outPatch.w_pos_111 - triCenter) / centerControlPointOffsetDivisor; // 2.0f;	//seems ad-hoc to take half of the vector upward from tri.
					
						//tessellation levels
						gl_TessLevelOuter[0] = getTessLevel(0);
						gl_TessLevelOuter[1] = getTessLevel(1);
						gl_TessLevelOuter[2] = getTessLevel(2);
					
						gl_TessLevelInner[0] = innerTessLevel[0];
					}
				)";

				////////////////////////////////////////////////////////
				//primitive generator (PG) is fixed function stage
				////////////////////////////////////////////////////////

				//---------- TES built in inputs------------
				//in vec3 gl_TessCoord;			//location within abstract patch
				//in int gl_PatchVerticesIn;	//vertex count for patch being processed
				//in int gl_PrimitiveID;		//index of current patch in series of patches being processed for this draw call
				//patch in float gl_TessLevelOuter[4];
				//patch in float gl_TessLevelInner[2];
				//in gl_PerVertex
				//{
				//  vec4 gl_Position;
				//  float gl_PointSize;
				//  float gl_ClipDistance[];
				//} gl_in[gl_MaxPatchVertices]; //only access within gl_PatchVerticesIn
				std::string tess_evaluation_shader_src = R"(
					#version 410 core
				
					layout(triangles) in;
					layout(ccw) in;
					LAYOUT_SPACING_STR	//text replaced to something like layout(equal_spacing) in;
					LAYOUT_POINTMODE;	//text replaced to enable/disable point mode

					struct ControlPoints_OutputPatch
					{
						vec3 w_pos_300;
						vec3 w_pos_210;
						vec3 w_pos_201;

						vec3 w_pos_030;
						vec3 w_pos_120;
						vec3 w_pos_021;

						vec3 w_pos_003;
						vec3 w_pos_012;
						vec3 w_pos_102;

						vec3 w_pos_111;	//the center point

						vec3 normal[3];
						vec2 uv[3];
					};
					in patch ControlPoints_OutputPatch outPatch;

					out vec3 pos_fs_in;
					out vec3 normal_fs_in;
					out vec2 uv_fs_in;

					uniform mat4 model = mat4(1.f);
					uniform mat4 view = mat4(1.f);
					uniform mat4 projection = mat4(1.f);

					vec3 barycentric_interpolate_3d(vec3 a, vec3 b, vec3 c)
					{
						return (a*gl_TessCoord.x + b*gl_TessCoord.y + c*gl_TessCoord.z);
					}
					vec2 barycentric_interpolate_2d(vec2 a, vec2 b, vec2 c)
					{
						return (a*gl_TessCoord.x + b*gl_TessCoord.y + c*gl_TessCoord.z);
					}

					//the bezier formula for this triangle is following. Notice association between 111 and uvw
					// cp300*w^3 + cp030*u^3 + cp003*v^3
					// + cp210*3(w^2)*u  + cp201*3*(w^2)*v
					// + cp021*3(u^2)*v + cp120*3w(u^2)
					// + cp102*3(v^2)w + cp012*3u*v(^2)
					// + cp111*6wuv		//this is 6, not 3
					void main(){

						float w = gl_TessCoord.x;
						float u = gl_TessCoord.y;
						float v = gl_TessCoord.z;
						float w2 = pow(w,2);
						float u2 = pow(u,2);
						float v2 = pow(v,2);
						float w3 = w * w2;
						float u3 = u * u2;
						float v3 = v * v2;

						//bezier 					
						uv_fs_in = barycentric_interpolate_2d(outPatch.uv[0], outPatch.uv[1], outPatch.uv[2]);
						normal_fs_in = barycentric_interpolate_3d(outPatch.normal[0],outPatch.normal[1],outPatch.normal[2]);
						vec3 bezierPos = 
							(outPatch.w_pos_300 * w3) + (outPatch.w_pos_030 * u3) + (outPatch.w_pos_003 * v3)
							 + outPatch.w_pos_210*3*w2*u  + outPatch.w_pos_201*3*w2*v
							 + outPatch.w_pos_021*3*u2*v  + outPatch.w_pos_120*3*w*u2
							 + outPatch.w_pos_102*3*v2*w  + outPatch.w_pos_012*3*u*v2
							 + outPatch.w_pos_111*6*w*u*v;								//note: this is 6, not 3

						//here is where we should do any perspective transformation for setting w values for clipping
						gl_Position = projection * view * model * vec4(bezierPos, 1.0f);
					}
				)";

				const char* frag_shader_src = R"(
					#version 410 core
					out vec4 fragmentColor;
				
					in vec3 pos_fs_in;
					in vec3 normal_fs_in;
					in vec2 uv_fs_in;

					uniform vec3 dirLight = normalize(vec3(-1,-1,-1));

					struct Material
					{
						sampler2D texture_diffuse0;
					};
					uniform Material material;					

					void main()
					{
						vec4 diffuseTexture = texture(material.texture_diffuse0, uv_fs_in);

						float diffuseFactor = max(dot(dirLight, normal_fs_in.xyz),0);
						vec3 diffuse = diffuseTexture.xyz * diffuseFactor;
						vec3 ambient = diffuseTexture.xyz * 0.05;

						fragmentColor = vec4(ambient+diffuse, 1.0f);
					}
				)";

			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertex_shader_src, nullptr);
			glCompileShader(vertexShader);
			verifyShaderCompiled("vertex shader", vertexShader);


			tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
			glShaderSource(tessControlShader, 1, &tessellation_control_shader_src, nullptr);
			glCompileShader(tessControlShader);
			verifyShaderCompiled("tess control shader", tessControlShader);

			const std::string equal_spacing_str = "layout(equal_spacing) in;";
			const std::string fractional_even_spacing_str = "layout(fractional_even_spacing) in;";
			const std::string fractional_odd_spacing_str = "layout(fractional_odd_spacing) in;";
			const std::string point_mode_on = "layout(point_mode) in;";
			const std::string point_mode_off = "";

			std::string spacing_str;
			switch (SpacingType(spacing))
			{
				case EQUAL_SPACING:				spacing_str = equal_spacing_str; break;
				case FRACTIONAL_EVEN_SPACING:	spacing_str = fractional_even_spacing_str; break;
				default:						spacing_str = fractional_odd_spacing_str; break;
			}

			const std::string replaceStr_space("LAYOUT_SPACING_STR");
			const std::string replaceStr_pointMode("LAYOUT_POINTMODE");

			tess_evaluation_shader_src.replace(
				tess_evaluation_shader_src.find(replaceStr_space),
				replaceStr_space.length(),
				spacing_str
			);
			tess_evaluation_shader_src.replace(
				tess_evaluation_shader_src.find(replaceStr_pointMode),
				replaceStr_pointMode.length(),
				bEnablePointMode ? point_mode_on : point_mode_off
			);

			const char* TES_cstr = tess_evaluation_shader_src.c_str();

			tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
			glShaderSource(tessEvaluationShader, 1, &TES_cstr, nullptr);
			glCompileShader(tessEvaluationShader);
			verifyShaderCompiled("tess evaluation shader", tessEvaluationShader);

			fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, &frag_shader_src, nullptr);
			glCompileShader(fragShader);
			verifyShaderCompiled("fragment shader", fragShader);

			if (shaderProg) { glDeleteProgram(shaderProg); }
			shaderProg = glCreateProgram();
			glAttachShader(shaderProg, vertexShader);
			glAttachShader(shaderProg, tessControlShader);
			glAttachShader(shaderProg, tessEvaluationShader);
			glAttachShader(shaderProg, fragShader);
			glLinkProgram(shaderProg);
			verifyShaderLink(shaderProg);

			glDeleteShader(vertexShader);
			glDeleteShader(tessControlShader);
			glDeleteShader(tessEvaluationShader);
			glDeleteShader(fragShader);
		};
		buildTessellationShaders();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// shader that displays normals at verts
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const char* normalDisplay_vs_src = R"(
					#version 410 core

					layout (location = 0) in vec3 position;				
					layout (location = 1) in vec3 normal;

					out InterfaceBlockVSOUT{
						vec4 vertNormal;
						vec4 vertPos;
						vec3 vertColor;
					} vs_out;
				
					void main(){

						const vec3 yellow = vec3(1,1,0);
						const vec3 pink = vec3(1, 0.4f, 0.7f);
						const vec3 purple = vec3(0.5f, 0.5f, 0);
						const vec3 red = vec3(1,0,0);
						const vec3 blue = vec3(0,1,0);
						const vec3 green = vec3(0,0,1);

						vs_out.vertColor = vec3(0,0,0);
						vs_out.vertColor += clamp(normal.r, 0, 1) * red;
						vs_out.vertColor += abs(clamp(normal.r, -1, 0)) * purple;

						vs_out.vertColor += clamp(normal.g, 0, 1) * green;
						vs_out.vertColor += abs(clamp(normal.g, -1, 0)) * pink;

						vs_out.vertColor += clamp(normal.b, 0, 1) * blue;
						vs_out.vertColor += abs(clamp(normal.b, -1, 0)) * yellow;

						vs_out.vertPos = vec4(position, 1.0f);					
						vs_out.vertNormal = normalize(vec4(normal,0));

						gl_Position = vec4(position, 1.f);
					}
	
				)";

		const char* normalDisplay_gs_src = R"(
				#version 330 core
		
				layout (triangles) in;
				layout (line_strip, max_vertices=6) out;

				in InterfaceBlockVSOUT{
					vec4 vertNormal;
					vec4 vertPos;
					vec3 vertColor;
				} vertices[];

				out vec3 fragNormal_ws;
				out vec3 fragPosition_ws;
				out vec4 fragColor;

				uniform mat4 model = mat4(1.f);
				uniform mat4 view = mat4(1.f);
				uniform mat4 projection = mat4(1.f);
				uniform float normalDisplayLength = 0.5f;

				void main(){

					//hopefully compiler will loop-unroll this, otherwise it might be better to hand-write each vertex
					for(int i = 0; i < 3; ++i)
					{
						//normal base
						gl_Position = projection * view * model * gl_in[i].gl_Position;	
						fragNormal_ws = normalize(vec3( inverse(transpose(model)) * vertices[i].vertNormal ));  
						fragPosition_ws = vec3(model * vertices[i].vertPos);
						fragColor = vec4(vertices[i].vertColor,1.f);
						EmitVertex();

						//normal tip
						fragPosition_ws = vec3(model * vertices[i].vertPos) + (fragNormal_ws * normalDisplayLength);
						gl_Position = projection * view * vec4(fragPosition_ws, 1.0f);
						EmitVertex();

						EndPrimitive();
					}
				}
			)";

		const char* normalDisplay_fs_src = R"(
					#version 410 core
					out vec4 fragmentColor;
				
					in vec3 fragNormal_ws;
					in vec3 fragPosition_ws;
					in vec4 fragColor;

					void main(){
						fragmentColor = fragColor;
					}
				)";


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// shader that display plane quads
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const char* planeDisplay_gs_src = R"(
				#version 330 core
		
				layout (triangles) in;
				layout (triangle_strip, max_vertices=12) out; //4 verts for each vertex

				in InterfaceBlockVSOUT{
					vec4 vertNormal;
					vec4 vertPos;
					vec3 vertColor;
				} vertices[];

				out vec3 fragNormal_ws;
				out vec3 fragPosition_ws;
				out vec4 fragColor;

				uniform mat4 model = mat4(1.f);
				uniform mat4 view = mat4(1.f);
				uniform mat4 projection = mat4(1.f);
				uniform float normalDisplayLength = 0.5f;
				uniform float planeSize = 0.25f;

				void main(){

					mat4 project_view = projection * view;

					vec3 triCenter = vec3(model * (0.333*vertices[0].vertPos + 0.333*vertices[1].vertPos + 0.333*vertices[2].vertPos));

					for(int i = 0; i < 3; ++i)
					{
						fragColor = vec4(vertices[i].vertColor, 0.5f);
						vec3 vert_center_ws = vec3(model*vertices[i].vertPos);

						fragNormal_ws = vec3( inverse(transpose(model)) * vertices[i].vertNormal );
						//vec3 temp = getDifferentVector(fragNormal_ws);
						vec3 temp = triCenter - vert_center_ws;
						vec3 u_ws_n = normalize(cross(fragNormal_ws, temp));
						vec3 v_ws_n = normalize(cross(fragNormal_ws, u_ws_n));
						
						vec3 vert_a =  vert_center_ws + (-u_ws_n * planeSize) + (-v_ws_n * planeSize);
						vec3 vert_b =  vert_center_ws + (u_ws_n * planeSize) + (-v_ws_n * planeSize);
						vec3 vert_c =  vert_center_ws + (-u_ws_n * planeSize) + (v_ws_n * planeSize);
						vec3 vert_d =  vert_center_ws + (u_ws_n * planeSize) + (v_ws_n * planeSize);

						gl_Position = project_view * vec4(vert_a, 1.0f);	
						fragPosition_ws = vert_a;
						EmitVertex();

						gl_Position = project_view * vec4(vert_b, 1.0f);	
						fragPosition_ws = vert_b;
						EmitVertex();

						gl_Position = project_view * vec4(vert_c, 1.0f);	
						fragPosition_ws = vert_c;
						EmitVertex();

						gl_Position = project_view * vec4(vert_d, 1.0f);	
						fragPosition_ws = vert_d;
						EmitVertex();

						EndPrimitive();
					}
				}
			)";

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// control point generator
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const char* controlPointDisplay_gs_src = R"(
				#version 330 core
		
				layout (triangles) in;
				layout (points, max_vertices=10) out; //4 verts for each vertex

				in InterfaceBlockVSOUT{
					vec4 vertNormal;
					vec4 vertPos;
					vec3 vertColor;
				} vertices[];

				out vec3 fragNormal_ws;
				out vec3 fragPosition_ws;
				out vec4 fragColor;

				uniform mat4 model = mat4(1.f);
				uniform mat4 view = mat4(1.f);
				uniform mat4 projection = mat4(1.f);
				uniform float centerControlPointOffsetDivisor = 2.0f;

				vec3 projectPntToNormalPlane(vec3 pnt, vec3 normal, vec3 normalPnt)
				{
					vec3 toPnt_v = pnt - normalPnt;
					vec3 proj_v = dot(toPnt_v, normal) * normal; //projection will face wrong direction
					vec3 finalPnt = pnt + -proj_v; //not this can be simplified to (pnt - proj)
					return finalPnt;
				}

				//mirroring what is done in the tessellation shader for display purposes.
				struct ControlPoints
				{
					vec3 w_pos_300;
					vec3 w_pos_210;
					vec3 w_pos_201;

					vec3 w_pos_030;
					vec3 w_pos_120;
					vec3 w_pos_021;

					vec3 w_pos_003;
					vec3 w_pos_012;
					vec3 w_pos_102;

					vec3 w_pos_111;	//the center point

					vec3 normal[3];
					vec3 color[3]; //unused... but mirroring structure in tessellation shader for clarity
				};
				ControlPoints points;

				void main(){

					/////////////////////////////////////
					// from tessellation control shader
					/////////////////////////////////////

					//set the corner positions
					points.w_pos_300 = vec3(model * vertices[0].vertPos);
					points.w_pos_030 = vec3(model * vertices[1].vertPos);
					points.w_pos_003 = vec3(model * vertices[2].vertPos);

					mat4 normalModel = inverse(transpose(model));
					points.normal[0] = vec3(normalModel * vertices[0].vertNormal);
					points.normal[1] = vec3(normalModel * vertices[1].vertNormal);
					points.normal[2] = vec3(normalModel * vertices[2].vertNormal);

					//edge is associated with vertex opposite from it. I defined this in CCW order. edges vecs point toward last point.
					vec3 edge_030_to_003 = points.w_pos_003 - points.w_pos_030;
					vec3 edge_003_to_300 = points.w_pos_300 - points.w_pos_003;
					vec3 edge_300_to_030 = points.w_pos_030 - points.w_pos_300;

					points.w_pos_210 = points.w_pos_300 + (1.0/3.0f)*edge_300_to_030;
					points.w_pos_201 = points.w_pos_300 - (1.0/3.0f)*(edge_003_to_300); //negate to flip edge
					
					points.w_pos_021 = points.w_pos_030 + (1.0/3.0f)*(edge_030_to_003);
					points.w_pos_120 = points.w_pos_030 - (1.0/3.0f)*(edge_300_to_030); //distribute -1 out of `pnt + (2/3)(-1edge)`

					points.w_pos_102 = points.w_pos_003 + (1.0/3.0f)*(edge_003_to_300);
					points.w_pos_012 = points.w_pos_003 - (1.0/3.0f)*(edge_030_to_003);
	
					//projection onto planes created by normals at corner vertices
					points.w_pos_210 = projectPntToNormalPlane(points.w_pos_210, points.normal[0], points.w_pos_300);
					points.w_pos_201 = projectPntToNormalPlane(points.w_pos_201, points.normal[0], points.w_pos_300);
					
					points.w_pos_021 = projectPntToNormalPlane(points.w_pos_021, points.normal[1], points.w_pos_030);
					points.w_pos_120 = projectPntToNormalPlane(points.w_pos_120, points.normal[1], points.w_pos_030);

					points.w_pos_102 = projectPntToNormalPlane(points.w_pos_102, points.normal[2], points.w_pos_003);
					points.w_pos_012 = projectPntToNormalPlane(points.w_pos_012, points.normal[2], points.w_pos_003);

					vec3 triCenter = (points.w_pos_300 + points.w_pos_030 + points.w_pos_003) / 3.0f; //barycentric coordiantes to get center
					points.w_pos_111 = (points.w_pos_210 + points.w_pos_201
										+ points.w_pos_021 + points.w_pos_120
										+ points.w_pos_102 + points.w_pos_012) / 6.f; //average all the generated cps
					points.w_pos_111 += (points.w_pos_111 - triCenter) / centerControlPointOffsetDivisor; // 2.0f;	//seems ad-hoc to take half of the vector upward from tri.
					
					///////////////////////////////////
					// geometry shader point display
					///////////////////////////////////

					mat4 project_view = projection * view;

					
					//generate flat normal
					vec4 a = vertices[0].vertPos, b = vertices[1].vertPos, c = vertices[2].vertPos;
					vec3 flatNormal = normalize(cross(vec3(b - a), vec3(c - a)));

					fragNormal_ws = vec3(inverse(transpose(model)) * vec4(flatNormal, 0.f));

					/////////////////////////
					//original verts
					/////////////////////////
					fragColor = vec4(1.0f, 0.f, 0.f, 1.0f);
		
					fragColor = vec4(1.0f, 0.f, 0.f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_300, 1.0f);	
					fragPosition_ws = points.w_pos_300;
					EmitVertex();

					fragColor = vec4(0.0f, 1.f, 0.f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_030, 1.0f);	
					fragPosition_ws = points.w_pos_030;
					EmitVertex();

					fragColor = vec4(0.0f, 0.f, 1.f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_003, 1.0f);	
					fragPosition_ws = points.w_pos_003;
					EmitVertex();

					/////////////////////////
					// intermediate verts
					/////////////////////////

					fragColor = vec4(0.65f, 0.35f, 0.f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_210, 1.0f);	
					fragPosition_ws = points.w_pos_210;
					EmitVertex();

					fragColor = vec4(0.65f, 0.f, 0.35f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_201, 1.0f);	
					fragPosition_ws = points.w_pos_201;
					EmitVertex();


					fragColor = vec4(0.0f, 0.35f, 0.65f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_012, 1.0f);	
					fragPosition_ws = points.w_pos_012;
					EmitVertex();

					fragColor = vec4(0.35f, 0.0f, 0.65f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_102, 1.0f);	
					fragPosition_ws = points.w_pos_102;
					EmitVertex();

					fragColor = vec4(0.35f, 0.65f, 0.f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_120, 1.0f);	
					fragPosition_ws = points.w_pos_120;
					EmitVertex();

					fragColor = vec4(0.0f, 0.65f, 0.35f, 1.0f);
					gl_Position = project_view * vec4(points.w_pos_021, 1.0f);	
					fragPosition_ws = points.w_pos_021;
					EmitVertex();

					//////////////////////
					//center cp
					//////////////////////
					fragColor = vec4(0.33f, 0.34f, 0.33f, 1.0f);

					gl_Position = project_view * vec4(points.w_pos_111, 1.0f);	
					fragPosition_ws = points.w_pos_111;
					EmitVertex();

					EndPrimitive();
				}
			)";


		auto buildGeometryShader = [](const char* vs, const char* gs, const char* fs, GLuint& outProg) {
			GLuint vertexShader = 0;
			GLuint geometryShader = 0;
			GLuint fragShader = 0;

			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vs, nullptr);
			glCompileShader(vertexShader);
			verifyShaderCompiled("vertex shader", vertexShader);

			geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometryShader, 1, &gs, nullptr);
			glCompileShader(geometryShader);
			verifyShaderCompiled("geometry shader", geometryShader);

			fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, &fs, nullptr);
			glCompileShader(fragShader);
			verifyShaderCompiled("fragment shader", fragShader);

			if (outProg) { glDeleteProgram(outProg); }
			outProg = glCreateProgram();
			glAttachShader(outProg, vertexShader);
			glAttachShader(outProg, geometryShader);
			glAttachShader(outProg, fragShader);
			glLinkProgram(outProg);
			verifyShaderLink(outProg);

			glDeleteShader(vertexShader);
			glDeleteShader(geometryShader);
			glDeleteShader(fragShader);
		};
		GLuint normalDisplayShader = 0;
		buildGeometryShader(normalDisplay_vs_src, normalDisplay_gs_src, normalDisplay_fs_src, normalDisplayShader);

		GLuint normalPlaneDisplayShader = 0;
		buildGeometryShader(normalDisplay_vs_src, planeDisplay_gs_src, normalDisplay_fs_src, normalPlaneDisplayShader);

		GLuint controlPointShader = 0;
		buildGeometryShader(normalDisplay_vs_src, controlPointDisplay_gs_src, normalDisplay_fs_src, controlPointShader);

		//ui
		bool bEnableDepth = 1;
		bool bPolygonMode = 1;
		bool bShowVertNormals = 1;
		bool bShowNormalPlanes = 0;
		bool bShowControlPoints = 0;
		float normalPlaneSize = 0.025f;
		float normalDisplayLength = 0.05f;
		const GLint planeSize_ul = glGetUniformLocation(normalPlaneDisplayShader, "planeSize");


		//uniforms
		bool bUseSingleOuterTL = 1;
		bool bUseSingleGlobalTL = 1;
		float centerControlPointOffsetDivisor = 2.0f;
		float controlPointSize = 8.0f;
		const float startTessellationLevel = 1.0f;
		GLfloat innerTessLevels[] = { startTessellationLevel, startTessellationLevel };
		GLfloat outerTessLevels[] = { startTessellationLevel, startTessellationLevel, startTessellationLevel, startTessellationLevel };

		const GLint bUseSingleOuterTL_ul = glGetUniformLocation(shaderProg, "bUseSingleOuterTL");
		const GLint bUseSingleGlobalTL_ul = glGetUniformLocation(shaderProg, "bUseSingleGlobalTL");
		const GLint innerTessLevel_ul = glGetUniformLocation(shaderProg, "innerTessLevel");
		const GLint outerTessLevels_ul = glGetUniformLocation(shaderProg, "outerTessLevels");
		const GLint model_ul = glGetUniformLocation(shaderProg, "model");
		const GLint view_ul = glGetUniformLocation(shaderProg, "view");
		const GLint projection_ul = glGetUniformLocation(shaderProg, "projection");
		const GLint centerControlPointOffsetDivisor_ul = glGetUniformLocation(shaderProg, "centerControlPointOffsetDivisor");

		////////////////////////////////////////////////////////
		// normal shader display
		////////////////////////////////////////////////////////
		const GLint gs_model_ul = glGetUniformLocation(normalDisplayShader, "model");
		const GLint gs_view_ul = glGetUniformLocation(normalDisplayShader, "view");
		const GLint gs_projection_ul = glGetUniformLocation(normalDisplayShader, "projection");
		const GLint gs_normalDisplayLength_ul = glGetUniformLocation(normalDisplayShader, "normalDisplayLength");

		////////////////////////////////////////////////////////
		// normal shader display
		////////////////////////////////////////////////////////
		const GLint gs_plane_model_ul = glGetUniformLocation(normalPlaneDisplayShader, "model");
		const GLint gs_plane_view_ul = glGetUniformLocation(normalPlaneDisplayShader, "view");
		const GLint gs_plane_projection_ul = glGetUniformLocation(normalPlaneDisplayShader, "projection");

		////////////////////////////////////////////////////////
		// control point display shader
		////////////////////////////////////////////////////////
		const GLint gs_cp_model_ul = glGetUniformLocation(controlPointShader, "model");
		const GLint gs_cp_view_ul = glGetUniformLocation(controlPointShader, "view");
		const GLint gs_cp_projection_ul = glGetUniformLocation(controlPointShader, "projection");
		const GLint gs_cp_centerControlPointOffsetDivisor_ul = glGetUniformLocation(controlPointShader, "centerControlPointOffsetDivisor");

		QuaternionCamera camera;
		camera.pos = glm::vec3{ 0.5f, 1.f, 1.f };
		camera.cameraSpeed = 2.5f;
		camera.rotation = glm::angleAxis(glm::radians<float>(180.f), glm::vec3(0, 1, 0));
		camera.updateBasisVectors();

		bool bRegenerateVerts = false;
		bool bRebuildShaders = false;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!glfwWindowShouldClose(window))
		{
			static float lastFrameTime = -1.f; //simulate some time on the first frame where time is 0.
			float thisFrameTime = float(glfwGetTime());
			float dt_sec = thisFrameTime - lastFrameTime;
			lastFrameTime = thisFrameTime;

			camera.tick(dt_sec, window);

			bool bShift = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && bShift)
			{
				glfwSetWindowShouldClose(window, true);
			}
			if (bRebuildShaders)
			{
				bRebuildShaders = false;
				buildTessellationShaders(); //do not rebuild shaders during GUI rendering.
			}

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			bEnableDepth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, bPolygonMode ? GL_FILL : GL_LINE);

			glUseProgram(shaderProg);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Render model
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			glm::mat4 view = camera.getView();
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(windowWidth) / windowHeight, 0.1f, 100.f);
			glUniform1i(bUseSingleGlobalTL_ul, int(bUseSingleGlobalTL)); //controls whether or not to allow independent outer levels
			glUniform1i(bUseSingleOuterTL_ul, int(bUseSingleOuterTL)); //controls whether or not to allow independent outer levels
			glUniform1fv(innerTessLevel_ul, 2, &innerTessLevels[0]);
			glUniform1fv(outerTessLevels_ul, 4, &outerTessLevels[0]);
			glUniform1f(centerControlPointOffsetDivisor_ul, centerControlPointOffsetDivisor);
			glUniformMatrix4fv(model_ul, 1, GL_FALSE, glm::value_ptr(meshModel_m));
			glUniformMatrix4fv(view_ul, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projection_ul, 1, GL_FALSE, glm::value_ptr(projection));
			targetModel->draw(shaderProg, GL_PATCHES);

			{
				////////////////////////////////////////////////////////
				// Use geometry shader debug visuals
				////////////////////////////////////////////////////////
				glUseProgram(normalDisplayShader);
				glUniformMatrix4fv(gs_model_ul, 1, GL_FALSE, glm::value_ptr(meshModel_m));
				glUniformMatrix4fv(gs_view_ul, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(gs_projection_ul, 1, GL_FALSE, glm::value_ptr(projection));
				glUniform1f(gs_normalDisplayLength_ul, normalDisplayLength);
				if (bShowVertNormals) { targetModel->draw(normalDisplayShader); }

				////////////////////////////////////////////////////////
				// render normal's planes at verts
				////////////////////////////////////////////////////////
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glUseProgram(normalPlaneDisplayShader);
				glUniform1f(planeSize_ul, normalPlaneSize);
				glUniformMatrix4fv(gs_plane_model_ul, 1, GL_FALSE, glm::value_ptr(meshModel_m));
				glUniformMatrix4fv(gs_plane_view_ul, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(gs_plane_projection_ul, 1, GL_FALSE, glm::value_ptr(projection));
				if (bShowNormalPlanes) { targetModel->draw(normalPlaneDisplayShader); }

				////////////////////////////////////////////////////////
				// Render control points
				////////////////////////////////////////////////////////
				glUseProgram(controlPointShader);
				glUniformMatrix4fv(gs_cp_model_ul, 1, GL_FALSE, glm::value_ptr(meshModel_m));
				glUniformMatrix4fv(gs_cp_view_ul, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(gs_cp_projection_ul, 1, GL_FALSE, glm::value_ptr(projection));
				glUniform1f(gs_cp_centerControlPointOffsetDivisor_ul, centerControlPointOffsetDivisor);
				glPointSize(controlPointSize);
				if (bShowControlPoints) { targetModel->draw(controlPointShader); }
				glPointSize(1.0f);	
			}

			{ //interactive UI
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				{
					ImGui::SetNextWindowPos({ 0,0 });
					ImGuiWindowFlags flags = 0;
					ImGui::Begin("OpenGL Tweaker (imgui library)", nullptr, flags);
					{
						static bool bExtendCenterCPDivisorRange = false;
						//ImGui::Checkbox("+10", &bExtendCenterCPDivisorRange); ImGui::SameLine();

						//below invalidates the point normal algorithm
						ImGui::SliderFloat("center CP offset divisor", &centerControlPointOffsetDivisor, 0.05f, bExtendCenterCPDivisorRange ? 12.f : 2.f);

						ImGui::Checkbox("vert normals", &bShowVertNormals);
						ImGui::SameLine();
						ImGui::Checkbox("normal planes", &bShowNormalPlanes);
						ImGui::SameLine();
						ImGui::Checkbox("control points", &bShowControlPoints);
						if (bShowVertNormals)
						{
							ImGui::SliderFloat("normal display length", &normalDisplayLength, 0.001f, 1.0f);
						}
						if (bShowNormalPlanes)
						{
							ImGui::SliderFloat("normal plane size", &normalPlaneSize, 0.001f, 2.0f);
						}
						if (bShowControlPoints)
						{
							ImGui::SliderFloat("control point size ", &controlPointSize, 1.0f, 16.0f);
						}

						ImGui::Dummy({ 0.f, 10.0f }); //make some space
						ImGui::Checkbox("enable depth test", &bEnableDepth);
						ImGui::SameLine();
						ImGui::Checkbox("render polygons", &bPolygonMode);
						if (ImGui::Checkbox("layout(point_mode) in;", &bEnablePointMode)) { bRebuildShaders = true; }

						ImGui::Checkbox("Use single tess level everywhere", &bUseSingleGlobalTL);
						ImGui::SameLine();
						ImGui::Checkbox("use large values", &bUseLargeValues);
						float maxScale = 10.0f * ((bUseLargeValues) ? 10.f : 1.f);

						if (bUseSingleGlobalTL)
						{
							ImGui::SliderFloat("Tess Level to use", &innerTessLevels[0], 0.f, maxScale);
						}
						else
						{
							ImGui::SliderFloat("Inner Tess Levels 0", &innerTessLevels[0], 0.f, maxScale);
							ImGui::SliderFloat("Inner Tess Levels 1", &innerTessLevels[1], 0.f, maxScale);

							ImGui::Dummy({ 0.f, 10.0f }); //make some space between the sliders

							ImGui::Checkbox("Use single outer tess level", &bUseSingleOuterTL);

							if (bUseSingleOuterTL)
							{
								ImGui::SliderFloat("Outer Tess Levels 0", &outerTessLevels[0], 0.f, maxScale);
							}
							else
							{
								ImGui::SliderFloat("Outer Tess Levels 0", &outerTessLevels[0], 0.f, maxScale);
								ImGui::SliderFloat("Outer Tess Levels 1", &outerTessLevels[1], 0.f, maxScale);
								ImGui::SliderFloat("Outer Tess Levels 2", &outerTessLevels[2], 0.f, maxScale);
								ImGui::SliderFloat("Outer Tess Levels 3", &outerTessLevels[3], 0.f, maxScale);
							}
						}
						ImGui::Separator();
						{
							if (ImGui::RadioButton("equal_spacing", &spacing, int(SpacingType::EQUAL_SPACING))) { bRebuildShaders = true; }
							ImGui::SameLine();
							if (ImGui::RadioButton("fractional_even_spacing", &spacing, int(SpacingType::FRACTIONAL_EVEN_SPACING))) { bRebuildShaders = true; }
							ImGui::SameLine();
							if (ImGui::RadioButton("fractional_odd_spacing", &spacing, int(SpacingType::FRACTIONAL_ODD_SPACING))) { bRebuildShaders = true; }
						}
						ImGui::Separator();
						{
							if (ImGui::RadioButton("satellite", &selectedMeshIdx, 0)) { refreshMesh(); }
							ImGui::SameLine();
							if (ImGui::RadioButton("nanosuit", &selectedMeshIdx, 1)) { refreshMesh(); }
							ImGui::SameLine();
							if (ImGui::RadioButton("man", &selectedMeshIdx, 2)) { refreshMesh(); }
						}
					}
					ImGui::End();
				}
				ImGui::EndFrame(); //added this to test decoupling rendering from frame; seems okay
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		glfwTerminate();

		glDeleteProgram(shaderProg);
		glDeleteProgram(normalDisplayShader);
		glDeleteProgram(normalPlaneDisplayShader);
		glDeleteProgram(controlPointShader);
	}
}


//influential sources
//	http://ogldev.atspace.co.uk/www/tutorial31/tutorial31.html


int main()
{
	true_main();
}