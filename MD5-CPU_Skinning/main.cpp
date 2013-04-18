#include "main.h"

using namespace std;

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
    assert(glfwOpenWindow(resolution.x, resolution.y, 0,0,0,0,0,0, GLFW_WINDOW ));
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(70,(double)resolution.x/resolution.y,1,1000);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    double beginTime = 0;
    unsigned int fps = 0;

    MD5Model g_Model;
    // Load a model file
    g_Model.LoadModel( "Ressources/Boblamp/boblampclean.md5mesh" );
    g_Model.LoadAnim( "Ressources/Boblamp/boblampclean.md5anim" );

    // Main loop
    while(glfwGetWindowParam( GLFW_OPENED ) )
    {
        beginTime = glfwGetTime();
// OpenGL rendering goes here...
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
        gluLookAt(50,50,50,0,0,30,0,0,1);
        glRotatef(glfwGetTime()*8.0, 0,0.0,1.0);

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

void catchError(char* operation)
{
    GLuint errorState = glGetError();
    if ( errorState != GL_NO_ERROR )
    {
        cerr << "\nErreur : " << gluErrorString(errorState) << " (" << errorState << ") apres l'operation : "<< operation << "\n";
    }
}
