#include "MD5ModelLoaderPCH.h"
#include "Helpers.h"

glPixels getPixelsFromTexture(const GLuint texture)
{
    glPixels pixels;
    pixels.data = NULL;
    pixels.height = 0;
    pixels.width = 0;
    pixels.internalFormat = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &(pixels.internalFormat)); // get internal format type of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &(pixels.width)); // get width of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &(pixels.height)); // get height of GL texture
// GL_TEXTURE_COMPONENTS and GL_INTERNAL_FORMAT are the same.
// just work with RGB8 and RGBA8
    GLint numBytes = 0;
    numBytes = pixels.width * pixels.height * getPixelSize(pixels.internalFormat);

    if(numBytes)
    {
        pixels.data = new unsigned char[numBytes]; // allocate image data into RAM
        glGetTexImage(GL_TEXTURE_2D, 0, pixels.internalFormat, GL_UNSIGNED_BYTE, (pixels.data));
        return pixels;
    }
    else
        return pixels;
}

GLuint getTextureFromPixels (const glPixels pixels, const bool mipmap)
{
    GLuint texture;
    glGenTextures(1,&texture);
    catchError("generation d\'un ID pour la normal map");

    // Constructs our Texture in the first position of our array
    glBindTexture(GL_TEXTURE_2D, texture);

    //Pixel storage mode : byte alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*
     * Defines filter used when texture is scaled (during the drawing)
     *    MAG_FILTER = magnified filter : when the texture is enlarged
     *    MIN_FILTER = minimized filter : when the texture is shrinked
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if(mipmap)
    {
        gluBuild2DMipmaps(GL_TEXTURE_2D, //target : usually GL_TEXTURE_2D
                              pixels.internalFormat,                              //internal format : RGB so 3 components
                              pixels.width,                //image size
                              pixels.height,
                              pixels.internalFormat,                      //fomat : RGB
                              GL_UNSIGNED_BYTE,            //data type : pixels are made of byte
                              pixels.data);        //picture datas
    }
    else
    {
        //set the properties of the texture : size, color type...
        glTexImage2D(GL_TEXTURE_2D,       //target : usually GL_TEXTURE_2D
                     0,                              //level : usually left to zero
                     pixels.internalFormat,                              //internal format : RGB so 3 components
                     pixels.width,                //image size
                     pixels.height,
                     0,                              //0 : no border
                     pixels.internalFormat,                       //format : usually RGB
                     GL_UNSIGNED_BYTE,            //data type : pixels are made of byte
                     pixels.data);        //picture datas
    }

    return texture;
}

GLuint getNormalMap(const GLuint heightMap)
{
    glPixels heightPixels = getPixelsFromTexture(heightMap);
    GLuint nMap(0);
    if(heightPixels.data != NULL)
    {
        glPixels normalPixels;
        normalPixels.height = heightPixels.height;
        normalPixels.width = heightPixels.width;
        normalPixels.internalFormat = heightPixels.internalFormat;
        normalPixels.data = new unsigned char[normalPixels.width * normalPixels.height * getPixelSize(normalPixels.internalFormat)];
        ///Calcul de la normal map
        GLint pixelSize = getPixelSize(normalPixels.internalFormat);

        glm::vec3 X(1,2,1), Y(1,0,-1);

        for(int i = 1; i < normalPixels.width-1; i++)
        {
            for(int j = 1; j < normalPixels.height-1; j++)
            {
                int dataPos = ((i*normalPixels.height)+j)*pixelSize;
                glm::vec3 normal(0,0,255);
                char tl =    heightPixels.data[(((i-1)*normalPixels.height)+j-1)*pixelSize]/255.0;
                char t =     heightPixels.data[((i*normalPixels.height)+j-1)*pixelSize]/255.0;
                char tr  =   heightPixels.data[(((i+1)*normalPixels.height)+j-1)*pixelSize]/255.0;

                char l =     heightPixels.data[(((i-1)*normalPixels.height)+j)*pixelSize]/255.0;
                char r =     heightPixels.data[(((i+1)*normalPixels.height)+j)*pixelSize]/255.0;

                char bl =    heightPixels.data[(((i-1)*normalPixels.height)+j+1)*pixelSize]/255.0;
                char b =     heightPixels.data[((i*normalPixels.height)+j+1)*pixelSize]/255.0;
                char br =    heightPixels.data[(((i+1)*normalPixels.height)+j+1)*pixelSize]/255.0;

                /*normal.x = tl * -1.0f + l* -2.0f + bl * -1.0f + tr * 1.0f + r*2.0f+br*1.0f;
                normal.y = tl* 1.0f + t * 2.0f + tr *1.0f + bl * -1.0f +b * -2.0f + br * -1.0f;*/
                 normal.x = (tr + 2.0 * r + br) - (tl + 2.0 * l + bl);
                normal.y = (bl + 2.0 * b + br) - (tl + 2.0 * t + tr);
                normal.z = 0.5;
                //normal = glm::vec3(0,0,255);
                normal = glm::normalize(normal);

                normalPixels.data[dataPos] = normal.x*255;
                normalPixels.data[dataPos+1] = normal.y*255;
                normalPixels.data[dataPos+2] = normal.z*255;
            }
        }
        ///Fin du calcul
        nMap = getTextureFromPixels(normalPixels, true);
        return nMap;
    }
    else
    {
        std::cerr<<"erreur a l\'extraction de la texture" << std::endl;
        return 0;
    }
}

void catchError(const char* operation)
{
    GLuint errorState = glGetError();
    unsigned int i = 1;
    while ( errorState != GL_NO_ERROR )
    {

        std::cerr << "\nErreur " << i << " : " << gluErrorString(errorState) << " (" << errorState << ") apres l'operation : "<< operation << "\n";
        errorState = glGetError();
        i++;
    }
}

// Remove the quotes from a string
void RemoveQuotes( std::string& str )
{
    size_t n;
    while ( ( n = str.find('\"') ) != std::string::npos ) str.erase(n,1);
}

// Get's the size of the file in bytes.
int GetFileLength( std::istream& file )
{
    int pos = file.tellg();
    file.seekg(0, std::ios::end );
    int length = file.tellg();
    // Restore the position of the get pointer
    file.seekg(pos);

    return length;
}

void IgnoreLine( std::istream& file, int length )
{
    file.ignore( length, '\n' );
}

// Computes the W component of the quaternion based on the X, Y, and Z components.
// This method assumes the quaternion is of unit length.
void ComputeQuatW( glm::quat& quat )
{
    float t = 1.0f - ( quat.x * quat.x ) - ( quat.y * quat.y ) - ( quat.z * quat.z );
    if ( t < 0.0f )
    {
        quat.w = 0.0f;
    }
    else
    {
        quat.w = -sqrtf(t);
    }
}

ElapsedTime::ElapsedTime( float maxTimeStep /* = 0.03333f */ )
    : m_fMaxTimeStep( maxTimeStep )
    , m_fPrevious ( std::clock() / (float)CLOCKS_PER_SEC )
{}

float ElapsedTime::GetElapsedTime() const
{
    float fCurrentTime = std::clock() / (float)CLOCKS_PER_SEC;
    float fDeltaTime = fCurrentTime - m_fPrevious;
    m_fPrevious = fCurrentTime;

    // Clamp to the max time step
    fDeltaTime = std::min( fDeltaTime, m_fMaxTimeStep );

    return fDeltaTime;
}

