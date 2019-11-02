#pragma once
#include<iostream>
#include <string>
#include<cmath>

#include<glad/glad.h> //include opengl headers, so should be before anything that uses those headers (such as GLFW)
#include<GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../old_utils/CameraFPS.h"
#include "../../old_utils/Shader.h"
#include <random>

namespace
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//opengl3.3 style of error checking; useful because immediate 
	inline void clearErrorsGL()
	{
		unsigned long error = GL_NO_ERROR;
		do
		{
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
		//make this a square so this demo doesn't have to do any special math with aspect to lay out points
		int width = 1200;
		int height = width;

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


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Compute Shaders - https://www.khronos.org/opengl/wiki/Compute_Shader
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////
		// Print stats about this device's compute abilities
		////////////////////////////////////////////////////////

		// The terminology on what gets work done can seem confusing and personally I think it is presented backwards. 
		// There's three levels things. I'll start from the bottom:
		//		lowest:		the worker threads;				OpenGL calls these `compute shader invocations`, they make up the `local size` of workgroup.
		//		middle:		the groups of worker threads;	OpenGL calls this the "work group", the number of threads is "work group size"
		//		highest:	the number of groups;			OpenGL calls this the "work group count"
		// The work-group and the collection of work-groups have a 3D structure.  A work group can have an x, y, z number of threads.
		// The work-group-count is a collection of work-groups defined in 3D space; it can have an x,y,z number of groups.
		// There are some limitations; for compute-shader-invocation threads can only share memory within their 'workgroup'. 
		// Also a work group has a limited 3d shape, but that shader is further limited by the max number of `CS_Invocations` a workgroup can have
		// We can ask OpenGL to give us the device's limits on these dimensions. 

		int maxWorkgroup_size[3]; //how big an individual workgroup is; eg the number of compute-shader-invocations it contains.
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
		// Random Number Generator
		////////////////////////////////////////////////////////
		std::random_device rng;
		std::seed_seq seed{ 28, 7, 9, 3, 101 };
		std::mt19937 rng_eng = std::mt19937(seed);
		std::uniform_real_distribution<float> ndcDist(-1.f, 1.f); //[a, b)
		std::uniform_real_distribution<float> attractionDist(0.01f, 1.f); //[a, b)

		////////////////////////////////////////////////////////
		// Compute shader storage buffer Stats
		////////////////////////////////////////////////////////
		std::cout << "\n" << std::endl;

		GLint maxSimultaneousShaderBufferObjects = 0;
		glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &maxSimultaneousShaderBufferObjects);
		std::cout << "max number of shader storage blocks that can be access from a compute shader: " << maxSimultaneousShaderBufferObjects << std::endl;

		GLint maxActiveShaderBufferObjects = 0;
		glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &maxActiveShaderBufferObjects);
		std::cout << "max number of shader storage blocks that can be active across all ACTIVE shaders: " << maxActiveShaderBufferObjects << std::endl;

		GLint maxShaderBufferSizeBytes = 0; //This is in basic machine units(ie: bytes).  https://www.khronos.org/opengl/wiki/GLAPI/glGet <uses machine units https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object <machine unit definition
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxShaderBufferSizeBytes);
		std::cout << "max shader block size: " << maxShaderBufferSizeBytes << std::endl;
		std::cout << std::endl;

		////////////////////////////////////////////////////////
		// Compute shader storage buffer loading
		////////////////////////////////////////////////////////

		const size_t rows =
			1024
			//1024 * 16
			//1024 * 32	
			//1024 * 64				//~64_000
			//1024 * 640			//~640_000
			//1024 * 1024			//~1mil
			//1024 * 1024 * 10		//~10mil
			;

		const size_t numParticles = rows * rows;

		std::vector<glm::vec4> posBufferCpu;
		std::vector<glm::vec4> velBufferCpu;
		std::vector<float> attractionBufferCpu;
		for (size_t idx = 0; idx < numParticles; ++idx)
		{
			float x = float(idx % rows) / rows;
			float y = float(idx / rows) / rows;
			float z = 0.0f;

			//make a random position within normalized device coordinates (ndc)
			posBufferCpu.emplace_back(x, y, z, 1.0f);
			velBufferCpu.emplace_back(0, 0, 0, 0);
			attractionBufferCpu.emplace_back(attractionDist(rng));
		}

		GLuint position_SSBO;
		glGenBuffers(1, &position_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, position_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, posBufferCpu.size() * sizeof(glm::vec4), &posBufferCpu[0], GL_DYNAMIC_COPY);	//https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml 

		GLuint velocity_SSBO;
		glGenBuffers(1, &velocity_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocity_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, velBufferCpu.size() * sizeof(glm::vec4), &velBufferCpu[0], GL_DYNAMIC_COPY);

		GLuint attractionStrength_SSBO;
		glGenBuffers(1, &attractionStrength_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, attractionStrength_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, attractionBufferCpu.size() * sizeof(float), &attractionBufferCpu[0], GL_DYNAMIC_COPY);

		////////////////////////////////////////////////////////
		// Creating a compute shader
		////////////////////////////////////////////////////////
		constexpr uint32_t workgroup_local_size = 32u;

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

		auto createComputeShader_closure = [verifyShaderCompiled, verifyShaderLink]
		(GLuint& shaderProgram, const char* const src)
		{
			GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(computeShader, 1, &src, nullptr);
			glCompileShader(computeShader);
			verifyShaderCompiled("compute shader", computeShader);

			shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, computeShader);
			glLinkProgram(shaderProgram);
			verifyShaderLink(shaderProgram);

			glDeleteShader(computeShader);
		};

		const char* const particle_compute_src = R"(
			#version 430	

			/////////////////////////////////////////////////////
			//  workgroup set up
			//	I've read nvidia warp size=32; amd wavefront size=64; probably good idea to match those
			/////////////////////////////////////////////////////
			layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

			/////////////////////////////////////////////////////
			// shader block storage
			/////////////////////////////////////////////////////
			layout(std430, binding = 0) buffer posSSBO
			{
				vec4 positions[];	//last element of buffer struct can be of unbounded size
			};

			layout(std430, binding = 1) buffer velSSBO
			{
				vec4 velocities[];	
			};

			layout(std430, binding=2) buffer mouseAttractionSSBO
			{
				float attractions[];	
			};

			/////////////////////////////////////////////////////
			// uniforms storage
			/////////////////////////////////////////////////////				
			uniform vec2 mouseLoc_ndc;
			uniform float deltaTimeSec;

			/////////////////////////////////////////////////////
			// main
			/////////////////////////////////////////////////////
			void main()
			{
				//___________BUILT-IN-CS-VARS___________\\
				// in uvec3 gl_NumWorkGroups;			// xyz of the work group count (what is passed to the dispatch function)
				// in uvec3 gl_WorkGroupID;				// the workgroup ID this thread(shader invocation) belongs to
				// in uvec3 gl_LocalInvocationID;		// The local thread number to this work group (xyz)
				// in uvec3 gl_GlobalInvocationID;		// The thread (xyz) ID out of all threads in this compute dispatch, NOT relative to workgroup
				// in uint  gl_LocalInvocationIndex;	// The thread linear ID of all the threads in the work group
				// const uvec3 gl_WorkGroupSize;		// the size of the current work group (ie what is specified at the top)

				uint globalIndex = gl_GlobalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y
								+ gl_GlobalInvocationID.y * gl_WorkGroupSize.x
								+ gl_GlobalInvocationID.x;
				
				vec4 position = positions[globalIndex];
				vec4 velocity = velocities[globalIndex];
				float attraction = attractions[globalIndex];

				position = velocity * deltaTimeSec + position;
				
				vec4 toMouse = vec4(mouseLoc_ndc, 0.f,1.f) - position;
				float distToMouse = length(toMouse);
				toMouse = toMouse / distToMouse;			//normalize, but use distance we already calculated

				const float maxInfluenceDistance = 0.1;
				float distanceFactor = clamp(distToMouse / maxInfluenceDistance, 0, 1);

				velocity = velocity + attraction * distanceFactor * toMouse;

				//keep particles within the NDC bounds; velocities will correct themselves over time
				position = clamp(position, vec4(-1,-1,-1, 1), vec4(1, 1, 1, 1));	//note, the w coordinate is 1; that is not a typo!

				//write data back to buffers
				positions[globalIndex] = position;
				velocities[globalIndex] = velocity;
			}
		)";
		GLuint particleComputeShader;
		createComputeShader_closure(particleComputeShader, particle_compute_src);

		GLuint particleVAO;
		glGenVertexArrays(1, &particleVAO);

		////////////////////////////////////////////////////////
		// Point Rendering Shaders
		////////////////////////////////////////////////////////
		const char* const pointVS_src = R"(
			#version 430

			layout(std430, binding = 0) buffer posSSBO
			{
				vec4 positions[];
			};

			void main()
			{
				gl_Position = positions[gl_VertexID];
			}
		)";
		const char* const pointsFG_src = R"(
			#version 430

			out vec4 fragColor;

			void main()
			{
				fragColor = vec4(1,1,1,1);
			}
		)";

		GLuint pointsVS = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(pointsVS, 1, &pointVS_src, nullptr);
		glCompileShader(pointsVS);
		verifyShaderCompiled("pointsVS", pointsVS);

		GLuint pointsFS = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(pointsFS, 1, &pointsFG_src, nullptr);
		glCompileShader(pointsFS);
		verifyShaderCompiled("pointsFS", pointsFS);

		GLuint pointShader = glCreateProgram();
		glAttachShader(pointShader, pointsVS);
		glAttachShader(pointShader, pointsFS);
		glLinkProgram(pointShader);
		verifyShaderLink(pointShader);

		glDeleteShader(pointsVS);
		glDeleteShader(pointsFS);

		glEnable(GL_DEPTH_TEST);

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

			glUseProgram(particleComputeShader);

			double xPosDouble, yPosDouble;
			glfwGetCursorPos(window, &xPosDouble, &yPosDouble);
			float xPos = (float)xPosDouble / width;		//[0, 1]
			float yPos = (float)yPosDouble / height;	//[0, 1]

			float xPosNDC = (xPos * 2) - 1.0f;
			float yPosNDC = -((yPos * 2) - 1.0f);

			static int mouseUniformLoc = glGetUniformLocation(particleComputeShader, "mouseLoc_ndc");
			static int timeUniformLoc = glGetUniformLocation(particleComputeShader, "deltaTimeSec");
			glUniform2f(mouseUniformLoc, xPosNDC, yPosNDC);
			glUniform1f(timeUniformLoc, deltaTime);

			//you can query the shader resource index, then set a binding that way... but it isn't really needed in OpenGL4.3 because shader storage bindings can be specified in layout
			//GLuint posSSBO_ShaderIndex = glGetProgramResourceIndex(particleComputeShader, GL_SHADER_STORAGE_BLOCK, "posSSBO");
			//GLuint posBlockBinding = 0;
			//glShaderStorageBlockBinding(particleComputeShader, posSSBO_ShaderIndex, posBlockBinding);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position_SSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocity_SSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, attractionStrength_SSBO);

			glDispatchCompute(numParticles / workgroup_local_size, 1, 1);

			//since we're not just writing to a shader storage; only wait on that.
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			////////////////////////////////////////////////////////
			// On screen frame buffer
			////////////////////////////////////////////////////////
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(pointShader);
			glBindVertexArray(particleVAO);
			glDrawArrays(GL_POINTS, 0, numParticles); //it appears you only need a VAO, you don't need any vert attributes!

			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		glfwTerminate();

		glDeleteBuffers(1, &position_SSBO);
		glDeleteBuffers(1, &velocity_SSBO);
		glDeleteBuffers(1, &attractionStrength_SSBO);
		glDeleteProgram(particleComputeShader);
		glDeleteProgram(pointShader);
		glDeleteVertexArrays(1, &particleVAO);
	}
}

//int main()
//{
//	true_main();
//}