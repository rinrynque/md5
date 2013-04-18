#pragma once;

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#ifdef WIN32
#define BOOST_THREAD_USE_LIB
#endif

#include <filesystem.hpp>
#include <filesystem/fstream.hpp>
namespace fs = boost::filesystem;

#include <gl/glfw.h>

#define GLM_SWIZZLE_XYZW
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/swizzle.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <SOIL/SOIL.h>
