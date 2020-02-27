#pragma once
#include "ray_utils.h"


namespace ho
{
	struct TriangleCube
	{
		TriangleCube()
		{
			glm::vec3 LTF{ -0.5f,  0.5f, 0.5f };
			glm::vec3 RTF{  0.5f,  0.5f, 0.5f };
			glm::vec3 LBF{ -0.5f, -0.5f, 0.5f };
			glm::vec3 RBF{  0.5f, -0.5f, 0.5f };
			
			glm::vec3 LTB{ -0.5f,  0.5f, -0.5f };
			glm::vec3 RTB{  0.5f,  0.5f, -0.5f };
			glm::vec3 LBB{ -0.5f, -0.5f, -0.5f };
			glm::vec3 RBB{  0.5f, -0.5f, -0.5f };


			//front face
			triangles.push_back(Triangle{ LTF, LBF, RBF });
			triangles.push_back(Triangle{ RBF, RTF, LTF });

			//back face
			triangles.push_back(Triangle{ LTB, RBB, LBB });
			triangles.push_back(Triangle{ LTB, RTB, RBB });

			//right face
			triangles.push_back(Triangle{ RTF, RBF, RTB });
			triangles.push_back(Triangle{ RBF, RBB, RTB });

			//left face
			triangles.push_back(Triangle{ LTF, LTB, LBF });
			triangles.push_back(Triangle{ LBF, LTB, LBB });

			//top face
			triangles.push_back(Triangle{ LTF, RTF, LTB });
			triangles.push_back(Triangle{ RTF, RTB, LTB });

			//bottom face
			triangles.push_back(Triangle{ LBF, LBB, RBF });
			triangles.push_back(Triangle{ RBF, LBB, RBB });
		}
		std::vector<Triangle> triangles;
	};
}