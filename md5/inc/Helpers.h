#pragma once

#define BUFFER_OFFSET(a) ((char*)NULL + (a))

#include <iostream>
void catchError(const char* operation);

// Remove the quotes from a string
void RemoveQuotes( std::string& str );

// Get's the size of the file in bytes.
int GetFileLength( std::istream& file );

// Ignore everything else that comes on the line, up to 'length' characters.
void IgnoreLine( std::istream& file, int length );

// Computes the W component of the quaternion based on the X, Y, and Z components.
// This method assumes the quaternion is of unit length.
void ComputeQuatW( glm::quat& quat );

struct glPixels
{
    GLint width;
    GLint height;
    GLint internalFormat;
    unsigned char* data;
};
inline GLint getPixelSize(GLint format)
{
    switch(format)
    {
    case GL_RGB:
        return 3;
        break;
    case GL_RGBA:
        return 4;
        break;
    default: // unsupported type
        return 0;
        break;
    }
};
glPixels getPixels(const GLuint texture);
GLuint getTextureFromPixels (const glPixels pixels, const bool mipmap);
GLuint getNormalMap(const GLuint heightMap);

// Helper class to count frame time
class ElapsedTime
{
public:
    ElapsedTime( float maxTimeStep = 0.03333f );
    float GetElapsedTime() const;

private:
    float m_fMaxTimeStep;
    mutable float m_fPrevious;
};

