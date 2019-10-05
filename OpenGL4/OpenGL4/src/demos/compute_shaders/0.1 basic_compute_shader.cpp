#pragma once
#include<iostream>
#include <string>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../old_utils/CameraFPS.h"
#include "../../old_utils/Shader.h"

namespace
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//opengl3.3 style of error checking; useful because immediate 
	inline void clearErrorsGL()
	{
		unsigned long error = GL_NO_ERROR;
		do{
			error = glGetError();
		} while (error != GL_NO_ERROR);
	}

	inline void LogGLErrors(const char* functionName, const char* file, int line)
	{
		bool bError = false;
		while (GLenum error = glGetError())  //GL_NO_ERROR is defined as 0; which means we can use it in condition statements like below
		{
			std::cerr << "OpenGLError:" << std::hex << error << " " << functionName << " " << file << " " << line << std::endl;
#ifdef _WIN32 
			__debugbreak();
#endif //_WIN32
		}
	}

#define ec(func) clearErrorsGL(); /*clear previous errors */\
	func;			/*call function*/ \
	LogGLErrors(#func, __FILE__, __LINE__)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

	void true_main()
	{
		//#todo clean up all resources? not deleting shaders after linkage (compute, vert, or frag)

		int width = 800;
		int height = 600;

		glfwInit();
		glfwSetErrorCallback([](int errorCode, const char* errorDescription) {std::cout << "GLFW error : " << errorCode << " : " << errorDescription << std::endl; });
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		GLFWwindow* window = glfwCreateWindow(width, height, "OpenglContext", nullptr, nullptr);
		if (!window)
		{
			std::cerr << "failed to create window" << std::endl;
			exit(-1);
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "failed to initialize glad with processes " << std::endl;
			exit(-1);
		}

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(&openGLErrorCallback_4_3, nullptr);

		glViewport(0, 0, width, height);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow*window, int width, int height) {  glViewport(0, 0, width, height); });

		////////////////////////////////////////////////////////
		// Post processing quad (ie render to texture)
		////////////////////////////////////////////////////////
		float quadVerts[] = {
			//x    y         s    t
			-1.0, -1.0,		0.0, 0.0,
			-1.0, 1.0,		0.0, 1.0,
			1.0, -1.0,		1.0, 0.0,

			1.0, -1.0,      1.0, 0.0,
			-1.0, 1.0,      0.0, 1.0,
			1.0, 1.0,		1.0, 1.0
		};

		GLuint quadVAO;
		glGenVertexArrays(1, &quadVAO);
		glBindVertexArray(quadVAO);

		GLuint quadVBO;
		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		const char* postprocess_vertex_shader_src = R"(
				#version 330 core
				layout (location = 0) in vec2 position;				
				layout (location = 1) in vec2 inTexCoord;
				
				out vec2 texCoord;

				void main(){
					texCoord = inTexCoord;
					gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
				}
			)";
		const char* postprocess_frag_shader_src = R"(
				#version 330 core
				out vec4 fragmentColor;

				in vec2 texCoord;

				uniform sampler2D screencapture;
				
				void main(){
					fragmentColor = texture(screencapture, texCoord);
				}
			)";

		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &postprocess_vertex_shader_src, nullptr);
		glCompileShader(vertShader);

		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &postprocess_frag_shader_src, nullptr);
		glCompileShader(fragShader);

		auto verifyShaderCompiled = [](const char* shadername, GLuint shader)
		{
			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				constexpr int size = 512;
				char infolog[size];

				glGetShaderInfoLog(shader, size, nullptr, infolog);
				std::cerr << "shader failed to compile: " << shadername << infolog << std::endl;
			}
		};
		verifyShaderCompiled("vertex shader", vertShader);
		verifyShaderCompiled("fragment shader", fragShader);

		GLuint texturedQuadShaderProgram = glCreateProgram();
		glAttachShader(texturedQuadShaderProgram, vertShader);
		glAttachShader(texturedQuadShaderProgram, fragShader);
		glLinkProgram(texturedQuadShaderProgram);
		auto verifyShaderLink = [](GLuint shaderProgram)
		{
			GLint success = 0;
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success)
			{
				constexpr int size = 512;
				char infolog[size];

				glGetProgramInfoLog(shaderProgram, size, nullptr, infolog);
				std::cerr << "SHADER LINK ERROR: " << infolog << std::endl;
			}
		};
		verifyShaderLink(texturedQuadShaderProgram);

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		int uniformLocation = glGetUniformLocation(texturedQuadShaderProgram, "screencapture");
		glUseProgram(texturedQuadShaderProgram);
		glUniform1i(uniformLocation, 0);

		glEnable(GL_DEPTH_TEST);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Compute Shaders - https://www.khronos.org/opengl/wiki/Compute_Shader
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////
		// Print stats about this device's compute abilities
		////////////////////////////////////////////////////////
		//#TODO clear definition of work groups and local sizes
		int maxWorkgroup_size[3]; //how big an individual workgroup is
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkgroup_size[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkgroup_size[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkgroup_size[2]);
		std::cout << "max work group size: " << "x=" << maxWorkgroup_size[0] << " y=" << maxWorkgroup_size[1] << " z=" << maxWorkgroup_size[2] << std::endl;

		int maxWorkgroup_count[3]; //how many work groups we can have in each dimension
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkgroup_count[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkgroup_count[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkgroup_count[2]);
		std::cout << "max work group count: " << "x=" << maxWorkgroup_count[0] << " y=" << maxWorkgroup_count[1] << " z=" << maxWorkgroup_count[2] << std::endl;

		int maxWorkGroup_invocations = 0; //the limit of jobs a single work group can do; workgroup x*y*z should always be less than #invocations value.
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroup_invocations);
		std::cout << "max work group invocations: " << maxWorkGroup_invocations << std::endl;

		int maxSharedBytes = 0;
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedBytes);
		std::cout << "max shared memory bytes between threads in a work group:" << maxSharedBytes << std::endl;

		////////////////////////////////////////////////////////
		// Compute shader output image
		////////////////////////////////////////////////////////
		int output_width = 1024;
		int output_height = 1024;
		GLuint outputTex = 0;
		glGenTextures(1, &outputTex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, outputTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, output_width, output_height, 0, GL_RGBA, GL_FLOAT, nullptr);

		//writing to a texture in shader uses "image storing functions"
		//the "image units" are not quite the same as "texture units", since we're using "image storing functions" we need to bind the texture in an image format.
		// The image lets us specify pixels with indices. I'm not sure about textures, normally we read them using floating point values on the range [0, 1] not uints.
		glBindImageTexture(
			0,				//GLuint unit,
			outputTex,		//GLuint texture,
			0,				//GLint level,
			GL_FALSE,		//GLboolean layered,
			0,				//GLint layer,
			GL_WRITE_ONLY,	//GLenum access,
			GL_RGBA32F		//GLenum format
		);

		////////////////////////////////////////////////////////
		// Creating a compute shader
		////////////////////////////////////////////////////////
		const char* const compute_src = R"(
			#version 430														//compute shaders were added in OpenGL 4.3
			layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;	//this compute shaders will have a workgroup for each pixel, so each work group only needs 1 thread.
			layout(rgba32f, binding = 0) uniform image2D img_output;			//We're going to be writing via "image" operations, so we need to set up our output to be an image2D, not a sampler2D
			
			void main()
			{
				vec4 outPixel = vec4(0,0,0,1.f);
				ivec2 pixelCoordinate = ivec2(gl_GlobalInvocationID.xy);		//gl_GlobalInvocationID is essentially a xyz thread number not accounting for workgroup layouts; it is calculated like: gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID
				
				outPixel.r = 1.0f;												//just make it red for testing

				imageStore(img_output, pixelCoordinate, outPixel);				//the image store function is how we write to our image
			}
		)";

		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeShader, 1, &compute_src, nullptr);
		glCompileShader(computeShader);
		verifyShaderCompiled("compute shader", computeShader);

 		GLuint computeProgram = glCreateProgram();
		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);
		verifyShaderLink(computeProgram);

		glDeleteShader(computeShader);

		glUseProgram(computeProgram);
		
		////////////////////////////////////////////////////////
		// Render Loop
		////////////////////////////////////////////////////////
		float lastFrameTime = 0;
		while (!glfwWindowShouldClose(window))
		{
			float currentTime = static_cast<float>(glfwGetTime());
			float deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			////////////////////////////////////////////////////////
			// process input
			////////////////////////////////////////////////////////
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, true);
			}

			////////////////////////////////////////////////////////
			// Run compute shader
			////////////////////////////////////////////////////////

			glUseProgram(computeProgram);
			glDispatchCompute(output_width, output_height, 1);

			////////////////////////////////////////////////////////
			// On screen frame buffer
			////////////////////////////////////////////////////////
			glUseProgram(texturedQuadShaderProgram);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing (although this doesn't seem to affect setup)

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, outputTex);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_DEPTH_TEST); //don't let depth buffer disrupt drawing

			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		glfwTerminate();

		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
		glDeleteProgram(computeProgram);
		glDeleteProgram(texturedQuadShaderProgram);
	}
}

int main()
{
	true_main();
}