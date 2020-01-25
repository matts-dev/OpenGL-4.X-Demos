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

		float sphereOffset = 1.0f;  //the closer this is to the tri points, the more different the normals will be.
		GLuint vao = 0;
		GLuint vbo = 0;
		auto regenerateVerts = [&]()
		{
			//create some ad-hoc normals that are generated as if this triangle is a tri on surface on a sphere
			struct Vert
			{
				glm::vec3 pos;
				glm::vec3 color;
				glm::vec3 normal;
			};
			Vert verts[] = {
				//						x    y      z				//rgb								normal: nx,ny,nz
					Vert{	glm::vec3{1.0f, -0.0f, 0.0f},			glm::vec3{1.f,0.f,0.f},		glm::vec3{0.f, 0.f, 1.f}},
					Vert{	glm::vec3{-0.5f, 0.866f, 0.0f},			glm::vec3{0.f,1.f,0.f},		glm::vec3{0.f, 0.f, 1.f}},
					Vert{	glm::vec3{-0.5f, -0.866f, 0.0f},		glm::vec3{0.f,0.f,1.f},		glm::vec3{0.f, 0.f, 1.f}}
			};
			glm::vec3 simSphereCenter{ 0.f, 0.f, -sphereOffset };
			verts[0].normal = glm::normalize(verts[0].pos - simSphereCenter);
			verts[1].normal = glm::normalize(verts[1].pos - simSphereCenter);
			verts[2].normal = glm::normalize(verts[2].pos - simSphereCenter);

			if (vao) { glDeleteVertexArrays(1, &vao); }
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			if (vbo) { glDeleteBuffers(1, &vbo); }
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //TODO remove
			glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), reinterpret_cast<void*>(0));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
			glEnableVertexAttribArray(2);
		};
		regenerateVerts();

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

		GLuint vertShader = 0;
		GLuint tessControlShader = 0;
		GLuint tessEvaluationShader = 0;
		GLuint fragShader = 0;
		GLuint shaderProg = 0;

		auto buildShaders = [&]() {
			const char* vertex_shader_src = R"(
					#version 410 core

					layout (location = 0) in vec3 position;				
					layout (location = 1) in vec3 vertColor;				
					layout (location = 2) in vec3 normal;

					out vec3 color_tcs_in;
					out vec3 pos_tcs_in;
					out vec3 normal_tcs_in;

					void main(){
						//note, when doing tessellation, you should wait to apply clip space projection until after tessellation
						//NOTICE: we do not write to gl_Position 
						color_tcs_in = vertColor;
						pos_tcs_in = position;					//do not perspective divide this yet, wait until after TES
						normal_tcs_in = normalize(normal);		//if you do any transformations of normal here, it is a good idea to renormalize. interpolation will happen TES and geometry shader
					}
				)";

			vertShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertShader, 1, &vertex_shader_src, nullptr);
			glCompileShader(vertShader);
			verifyShaderCompiled("vertex shader", vertShader);


			//--------provided input------------
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
					in vec3 color_tcs_in[];
					in vec3 normal_tcs_in[];

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
						vec3 color[3];
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
							outPatch.color[cp] = color_tcs_in[cp];
							outPatch.normal[cp] = normal_tcs_in[cp];
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

						//calculate center point. this will be some distnace up from the flat center of tri. dist: We create plane of mid points, then from center of that plane go up 1/2 distance of the vector from flat_tri_center to mid_plane_center.
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
			tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
			glShaderSource(tessControlShader, 1, &tessellation_control_shader_src, nullptr);
			glCompileShader(tessControlShader);
			verifyShaderCompiled("tess control shader", tessControlShader);

			////////////////////////////////////////////////////////
			//primitive generator (PG) is fixed function stage
			////////////////////////////////////////////////////////

			//----------built in inputs------------
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
						vec3 color[3];
						//vec3 position[3];
					};
					in patch ControlPoints_OutputPatch outPatch;

					out vec3 pos_fs_in;
					out vec3 color_fs_in;
					out vec3 normal_fs_in;

					uniform mat4 view = mat4(1.f);
					uniform mat4 projection = mat4(1.f);

					vec3 barycentric_interpolate_3d(vec3 a, vec3 b, vec3 c)
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

						color_fs_in = barycentric_interpolate_3d(outPatch.color[0], outPatch.color[1], outPatch.color[2]);
					
						vec3 bezierPos = 
							(outPatch.w_pos_300 * w3) + (outPatch.w_pos_030 * u3) + (outPatch.w_pos_003 * v3)
							 + outPatch.w_pos_210*3*w2*u  + outPatch.w_pos_201*3*w2*v
							 + outPatch.w_pos_021*3*u2*v  + outPatch.w_pos_120*3*w*u2
							 + outPatch.w_pos_102*3*v2*w  + outPatch.w_pos_012*3*u*v2
							 + outPatch.w_pos_111*6*w*u*v;								//note: this is 6, not 3

						//here is where we should do any perspective transformation for setting w values for clipping
						gl_Position = projection * view * vec4(bezierPos, 1.0f);
					}
				)";

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

			const char* frag_shader_src = R"(
					#version 410 core
					out vec4 fragmentColor;
				
					in vec3 color_fs_in;
					in vec3 pos_fs_in;
					in vec3 normal_fs_in;

					void main(){
						fragmentColor = vec4(color_fs_in, 1.0f);
					}
				)";
			fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, &frag_shader_src, nullptr);
			glCompileShader(fragShader);
			verifyShaderCompiled("fragment shader", fragShader);

			if (shaderProg) { glDeleteProgram(shaderProg); }
			shaderProg = glCreateProgram();
			glAttachShader(shaderProg, vertShader);
			glAttachShader(shaderProg, tessControlShader);
			glAttachShader(shaderProg, tessEvaluationShader);
			glAttachShader(shaderProg, fragShader);
			glLinkProgram(shaderProg);
			verifyShaderLink(shaderProg);

			glDeleteShader(vertShader);
			glDeleteShader(tessControlShader);
			glDeleteShader(tessEvaluationShader);
			glDeleteShader(fragShader);
		};
		buildShaders();

		//ui
		bool bEnableDepth = 1;
		bool bPolygonMode = 0;

		//uniforms
		bool bUseSingleOuterTL = 1;
		bool bUseSingleGlobalTL = 1;
		float centerControlPointOffsetDivisor = 2.0f;
		GLfloat innerTessLevels[] = { 3.0f, 3.0f };
		GLfloat outerTessLevels[] = { 3.0f, 3.0f, 3.0f, 3.0f };

		const GLint bUseSingleOuterTL_ul = glGetUniformLocation(shaderProg, "bUseSingleOuterTL");
		const GLint bUseSingleGlobalTL_ul = glGetUniformLocation(shaderProg, "bUseSingleGlobalTL");
		const GLint innerTessLevel_ul = glGetUniformLocation(shaderProg, "innerTessLevel");
		const GLint outerTessLevels_ul = glGetUniformLocation(shaderProg, "outerTessLevels");
		const GLint view_ul = glGetUniformLocation(shaderProg, "view");
		const GLint projection_ul = glGetUniformLocation(shaderProg, "projection");
		const GLint centerControlPointOffsetDivisor_ul = glGetUniformLocation(shaderProg, "centerControlPointOffsetDivisor");

		
		struct QuatOrbitCam
		{
			glm::quat rotation{ 1.f,0,0,0 };
			glm::vec3 u_axis{ 1.f,0.f,0.f };
			glm::vec3 v_axis{ 0.f,1.f,0.f };
			glm::vec3 w_axis{ 0.f,0.f,1.f  };
			float offsetDistance = 2.5f;
			glm::vec3 pos{ 0.f, 0.f, offsetDistance };
			float mouseSensitivity = 0.0125f;

			void mouseMoved(const glm::vec2& deltaMouse)
			{
				glm::vec3 uvPlaneVec = u_axis* deltaMouse.x;
				uvPlaneVec += v_axis * deltaMouse.y;

				float rotationMagnitude = glm::length(uvPlaneVec);
				if (rotationMagnitude == 0.0f) { return; }
				uvPlaneVec = glm::normalize(uvPlaneVec);

				glm::vec3 rotationAxis = glm::normalize(glm::cross(uvPlaneVec, -w_axis));
				glm::quat deltaQuat = glm::angleAxis(mouseSensitivity * rotationMagnitude, rotationAxis);
				NAN_BREAK(deltaQuat);

				rotation = deltaQuat * rotation;

				glm::mat4 transform = glm::toMat4(rotation);
				u_axis = glm::normalize(glm::vec3(transform * glm::vec4{ 1,0, 0,0 })); //#optimize the normalization of the basis may be superfluous, but needs testing due to floating point error
				v_axis = glm::normalize(glm::vec3(transform * glm::vec4{ 0,1, 0,0 }));
				w_axis = glm::normalize(glm::vec3(transform * glm::vec4{ 0,0,-1,0})); 
				NAN_BREAK(u_axis);
				NAN_BREAK(v_axis);
				NAN_BREAK(w_axis);

				//fix on point (0,0,0)
				glm::vec3 front_offset = w_axis * offsetDistance;
				pos = /*vec3(0,0,0) + */ front_offset;
			}
			glm::mat4 getView() 
			{ 
				return glm::lookAt(pos, glm::vec3(0.f), v_axis); 
			}
		};
		QuatOrbitCam camera = {};

		bool bRegenerateVerts = false;
		bool bRebuildShaders = false;

		while (!glfwWindowShouldClose(window))
		{
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, true);
			}
			if (bRegenerateVerts)
			{
				bRegenerateVerts = false;
				regenerateVerts();	//don't make opengl calls while ImGUI is rendering
			}
			if (bRebuildShaders)
			{
				bRebuildShaders = false;
				buildShaders(); //do not rebuild shaders during GUI rendering.
			}
			static bool bMiddleBtnPressed = false;
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE))
			{
				static double lastX = 0;
				static double lastY = 0;
				double x, y;
				//just polling to keep this demo more readable from top to bottom, rather than having to jump between callback functions
				glfwGetCursorPos(window, &x, &y);
				x += double(windowWidth) / 0.5;
				y += double(windowHeight) / 0.5;

				if (!bMiddleBtnPressed) //reset the lastx/y between middle button clicks
				{
					lastX = x; lastY = y;
					bMiddleBtnPressed = true;
				}
				glm::vec2 deltaMouse{ float(x - lastX), float(y - lastY) };
				lastX = x;
				lastY = y;
				camera.mouseMoved(deltaMouse);
			}
			else
			{
				bMiddleBtnPressed = false;
			}

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			bEnableDepth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, bPolygonMode ? GL_FILL : GL_LINE);

			glUseProgram(shaderProg);
			glBindVertexArray(vao);

			glm::mat4 view = camera.getView();
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(windowWidth) / windowHeight, 0.1f, 100.f);
			glUniform1i(bUseSingleGlobalTL_ul, int(bUseSingleGlobalTL)); //controls whether or not to allow independent outer levels
			glUniform1i(bUseSingleOuterTL_ul, int(bUseSingleOuterTL)); //controls whether or not to allow independent outer levels
			glUniform1fv(innerTessLevel_ul, 2, &innerTessLevels[0]);
			glUniform1fv(outerTessLevels_ul, 4, &outerTessLevels[0]);
			glUniform1f(centerControlPointOffsetDivisor_ul, centerControlPointOffsetDivisor);
			glUniformMatrix4fv(view_ul, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projection_ul, 1, GL_FALSE, glm::value_ptr(projection));

			
			glPatchParameteri(GL_PATCH_VERTICES, 3); //doesn't need to be done every frame, doing here for demo clarity
			glDrawArrays(GL_PATCHES, 0, 3);

			{ //interactive UI
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				{
					ImGui::SetNextWindowPos({ 0,0 });
					ImGuiWindowFlags flags = 0;
					ImGui::Begin("OpenGL Tweaker (imgui library)", nullptr, flags);
					{
						ImGui::Checkbox("enable depth test", &bEnableDepth);
						ImGui::SameLine();
						ImGui::Checkbox("render polygons", &bPolygonMode);
						if (ImGui::Checkbox("layout(point_mode) in;", &bEnablePointMode)) { bRebuildShaders = true; }

						if(ImGui::SliderFloat("normal adjustment", &sphereOffset, 0.001f, 5.0f)) { bRegenerateVerts = true; }
						ImGui::SliderFloat("center CP divisor", &centerControlPointOffsetDivisor, 0.05f, 2.f);

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
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
}


//influential sources
//	http://ogldev.atspace.co.uk/www/tutorial31/tutorial31.html


int main()
{
	true_main();
}