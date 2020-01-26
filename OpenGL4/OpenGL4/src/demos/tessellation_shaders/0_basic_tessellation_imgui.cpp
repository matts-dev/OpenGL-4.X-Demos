#include<iostream>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>
#include <string>
#include "imgui.1.69.gl/imgui.h"
#include <imgui.1.69.gl/imgui_impl_glfw.h>
#include <imgui.1.69.gl/imgui_impl_opengl3.h>


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

		float vertices[] = {
			//x    y      z				//rgb
			-0.5f, -0.5f, 0.0f,		     1.f,0.f,0.f,
			0.5f, -0.5f, 0.0f,			 0.f,1.f,0.f,
			0.0f, 0.5f, 0.0f,			 0.f,0.f,1.f
		};

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

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


		// set number of verts we're going to be passing
		glPatchParameteri(GL_PATCH_VERTICES, 3); //telling opengl we're sending 3 control points
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create the shaders
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		const char* vertex_shader_src = R"(
				#version 410 core

				layout (location = 0) in vec3 position;				
				layout (location = 1) in vec3 vertColor;				

				out vec3 color_tcs_in;
				out vec3 pos_tcs_in;

				void main(){
					//note, when doing tessellation, you should wait to apply clip space projection until after tessellation
					//NOTICE: we do not write to gl_Position 
					color_tcs_in = vertColor;
					pos_tcs_in = position; //do not perspective divide this yet, wait until after TES
				}
			)";

		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
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
				layout (vertices = 3) out;	//define OUTPUT patch_size (ie the control points), must be less than patch size limit
				
				in vec3 pos_tcs_in[];
				in vec3 color_tcs_in[];

				out vec3 pos_es_in[];
				out vec3 color_es_in[];

				uniform float innerTessLevel[2];
				uniform float outerTessLevels[4];
				uniform bool bUseSingleGlobalTL = false;
				uniform bool bUseSingleOuterTL = true;

				float tessLevelForEdge(vec3 posA, vec3 posB, uint outerIndex)
				{
					//TL's are roughly how many things to subdivide an edge into
					if(bUseSingleGlobalTL) return innerTessLevel[0];
					else return outerTessLevels[bUseSingleOuterTL ? 0 : outerIndex];
				}

				void main(){

					//pass the vert data to TES to be interpolated using the PG provided barycentric coordinates
					pos_es_in[gl_InvocationID] = pos_tcs_in[gl_InvocationID];
					color_es_in[gl_InvocationID] = color_tcs_in[gl_InvocationID];
					
					//provide TLs (tessellation levels) for this given domain (eg triangle vs quad vs isoline)
					gl_TessLevelOuter[0] = tessLevelForEdge(pos_es_in[1], pos_es_in[2], 0);
					gl_TessLevelOuter[1] = tessLevelForEdge(pos_es_in[2], pos_es_in[0], 1);
					gl_TessLevelOuter[2] = tessLevelForEdge(pos_es_in[0], pos_es_in[1], 2);
					
					gl_TessLevelInner[0] = innerTessLevel[0];
				}
			)";
		GLuint tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
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
		const char* tessellation_evaluation_shader_src = R"(
				#version 410 core
				
				//layout(triangles, equal_spacing, ccw) in;
				layout(triangles) in;
				layout(equal_spacing) in;
				layout(ccw) in;

				in vec3 pos_es_in[];
				in vec3 color_es_in[];				

				out vec3 pos_fs_in;
				out vec3 color_fs_in;

				vec3 barycentric_interpolate_3d(vec3 a, vec3 b, vec3 c)
				{
					return (a*gl_TessCoord.x + b*gl_TessCoord.y + c*gl_TessCoord.z);
				}

				void main(){
					pos_fs_in = barycentric_interpolate_3d(pos_es_in[0], pos_es_in[1], pos_es_in[2]);
					color_fs_in = barycentric_interpolate_3d(color_es_in[0], color_es_in[1], color_es_in[2]);

					//here is where we should do any perspective transformation for setting w values for clipping
					gl_Position = vec4(pos_fs_in, 1.0f);
				}
			)";
		GLuint tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tessEvaluationShader, 1, &tessellation_evaluation_shader_src, nullptr);
		glCompileShader(tessEvaluationShader);
		verifyShaderCompiled("tess evaluation shader", tessEvaluationShader);

		const char* frag_shader_src = R"(
				#version 410 core
				out vec4 fragmentColor;
				
				in vec3 color_fs_in;
				in vec3 pos_fs_in;

				void main(){
					fragmentColor = vec4(color_fs_in, 1.0f);
				}
			)";
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &frag_shader_src, nullptr);
		glCompileShader(fragShader);
		verifyShaderCompiled("fragment shader", fragShader);

		GLuint shaderProg = glCreateProgram();
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

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//uniforms
		bool bUseSingleOuterTL = 1;
		bool bUseSingleGlobalTL = 1;
		GLfloat innerTessLevels[] = { 3.0f, 3.0f };
		GLfloat outerTessLevels[] = { 3.0f, 3.0f, 3.0f, 3.0f };

		const GLint bUseSingleOuterTL_ul = glGetUniformLocation(shaderProg, "bUseSingleOuterTL");
		const GLint bUseSingleGlobalTL_ul = glGetUniformLocation(shaderProg, "bUseSingleGlobalTL");
		const GLint innerTessLevel_ul = glGetUniformLocation(shaderProg, "innerTessLevel");
		const GLint outerTessLevels_ul = glGetUniformLocation(shaderProg, "outerTessLevels");

		//set defaults
		glUseProgram(shaderProg);


		while (!glfwWindowShouldClose(window))
		{
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, true);
			}

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(shaderProg);
			glBindVertexArray(vao);

			glUniform1i(bUseSingleGlobalTL_ul, int(bUseSingleGlobalTL)); //controls whether or not to allow independent outer levels
			glUniform1i(bUseSingleOuterTL_ul, int(bUseSingleOuterTL)); //controls whether or not to allow independent outer levels
			glUniform1fv(innerTessLevel_ul, 2, &innerTessLevels[0]);
			glUniform1fv(outerTessLevels_ul, 4, &outerTessLevels[0]); 

			glDrawArrays(GL_PATCHES, 0, 3);


			{ //interactive UI
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				{
					ImGui::SetNextWindowPos({ 0,0 });
					//ImGui::SetNextWindowSize({ 300,300 });
					ImGuiWindowFlags flags = 0;
					//flags |= ImGuiWindowFlags_NoMove;
					//flags |= ImGuiWindowFlags_NoResize;
					//flags |= ImGuiWindowFlags_NoCollapse;
					ImGui::Begin("OpenGL Tweaker (imgui library)", nullptr, flags);
					{
						ImGui::Checkbox("Use single tess level everywhere", &bUseSingleGlobalTL);
						if (bUseSingleGlobalTL)
						{
							ImGui::SliderFloat("Tess Level to use", &innerTessLevels[0], 0.f, 10.01f);
						}
						else
						{
							ImGui::SliderFloat("Inner Tess Levels 0", &innerTessLevels[0], 0.f, 10.01f);
							ImGui::SliderFloat("Inner Tess Levels 1", &innerTessLevels[1], 0.f, 15.f);
						
							ImGui::Dummy({0.f, 10.0f}); //make some space between the sliders

							ImGui::Checkbox("Use single outer tess level", &bUseSingleOuterTL);

							if (bUseSingleOuterTL)
							{
								ImGui::SliderFloat("Outer Tess Levels 0", &outerTessLevels[0], 0.f, 10.f);
							}
							else
							{
								ImGui::SliderFloat("Outer Tess Levels 0", &outerTessLevels[0], 0.f, 10.f);
								ImGui::SliderFloat("Outer Tess Levels 1", &outerTessLevels[1], 0.f, 10.f);
								ImGui::SliderFloat("Outer Tess Levels 2", &outerTessLevels[2], 0.f, 10.f);
								ImGui::SliderFloat("Outer Tess Levels 3", &outerTessLevels[3], 0.f, 10.f);
							}
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

//int main()
//{
//	true_main();
//}