#include "main.h"

int main (int argc, char** argv)
{
    ///Initialisation OpenGL
    glfwInit();

    ///Initialisation Rude::Config
    rude::Config config;
    assert(config.load("config.ini"));
    config.setSection("window");
    coords<int> resolution(800,600);

    resolution.x = config.getIntValue("WIDTH");
    resolution.y = config.getIntValue("HEIGHT");


    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    assert(glfwOpenWindow(resolution.x, resolution.y, 0,0,0,0,0,0, GLFW_WINDOW ));
    glfwSetWindowTitle("md5 loader");
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(70,(double)resolution.x/resolution.y,1,1000);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    double beginTime = 0;
    unsigned int fps = 0;
    glewInit();

	std::cout << "OpenGL initialized: OpenGL version: " << glGetString(GL_VERSION) << " GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    MD5Model g_Model;
    // Load a model file
    config.setSection("model");
    g_Model.LoadModel( config.getStringValue("MODELPATH"));//"Ressources/bras/test_02.md5mesh" );
    g_Model.LoadAnim( config.getStringValue("ANIMPATH"));//"Ressources/bras/test_02.md5anim" );
    g_Model.setShaders("Ressources/shaders/md5");

    // Main loop
    glm::vec3 camPos(50,50,50);
    int hauteur(0);
    while(glfwGetWindowParam( GLFW_OPENED ) )
    {
        beginTime = glfwGetTime();
// OpenGL rendering goes here...
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
        gluLookAt(camPos.x,camPos.y,camPos.z,0,0,hauteur,0,0,1);

        if(glfwGetKey(GLFW_KEY_UP))
            camPos.x++;
        if(glfwGetKey(GLFW_KEY_DOWN))
            camPos.x--;
        if(glfwGetKey(GLFW_KEY_LEFT))
            camPos.y++;
        if(glfwGetKey(GLFW_KEY_RIGHT))
            camPos.y--;
        if(glfwGetKey(GLFW_KEY_KP_8))
            camPos.z++;
        if(glfwGetKey(GLFW_KEY_KP_5))
            camPos.z--;
        if(glfwGetKey(GLFW_KEY_KP_9))
            hauteur++;
        if(glfwGetKey(GLFW_KEY_KP_6))
            hauteur--;

        g_Model.Render();
// Swap front and back rendering buffers
        glFlush();
        glfwSwapBuffers();

        double loopTime = glfwGetTime() - beginTime;
        g_Model.Update( 0.05 );
        fps = 1.0/loopTime;
        if(fps >  _MAX_FPS_)
        {
            glfwSleep(1.0/(double)_MAX_FPS_ );
            fps = 1.0/(glfwGetTime() - beginTime);
        }

    }
    ///Terminer GLFW
    glfwTerminate();
    return 0;
}
