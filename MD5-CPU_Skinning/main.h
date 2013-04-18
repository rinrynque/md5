#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <new>

#include <fstream>
#include <cmath>
#include <ctime>
#include <GL/glfw.h>

#include "MD5ModelLoaderPCH.h"
#include "Helpers.h"
#include "MD5Model.h"
//#include "PivotCamera.h"
#include "config.h"
#include "coords.h"

#define _MAX_FPS_ 60

void catchError(char* operation);

#endif // MAIN_H_INCLUDED
