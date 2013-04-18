#include <iostream>

#include "MD5ModelLoaderPCH.h"

/*#include "EffectManager.h"
#include "Effect.h"
#include "EffectParameter.h"
#include "Technique.h"
#include "Pass.h"*/

#include "Helpers.h"
#include "MD5Model.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

void DeleteVertexBuffer( GLuint& vboID )
{
    if ( vboID != 0 )
    {
        glDeleteBuffers( 1, &vboID );
        catchError("delete buffer.");
        vboID = 0;
    }
}

inline void CreateVertexBuffer( GLuint& vboID )
{
    // Make sure we don't loose the reference to the previous VBO if there is one
    DeleteVertexBuffer( vboID );
    glGenBuffers( 1, &vboID );
    catchError("generation d\'un buffer.");
}

MD5Model::MD5Model(  )
: m_iMD5Version(-1)
, m_iNumJoints(0)
, m_iNumMeshes(0)
, m_bHasAnimation(false)
, m_LocalToWorldMatrix(1)
, m_WorldToLoacalMatrix(1)
, m_VertexSkinning( VS_GPU )
{
    m_DefaultMaterial.SpecularPower = 78.0f;
}

MD5Model::~MD5Model()
{
    /*MeshList::iterator iter = m_Meshes.begin();
    while ( iter != m_Meshes.end() )
    {
        DestroyMesh( *iter );
        ++iter;
    }
    m_Meshes.clear();*/
    /*for(unsigned int i = 0; i < m_Meshes.size(); i++)
    {
        DestroyMesh( m_Meshes[i] );
    }*/
}

void MD5Model::DestroyMesh( Mesh& mesh )
{
    // Delete all the VBO's
    DeleteVertexBuffer( mesh.m_GLPositionBuffer );
    DeleteVertexBuffer( mesh.m_GLNormalBuffer );
    DeleteVertexBuffer( mesh.m_GLTexCoord );
    DeleteVertexBuffer( mesh.m_GLBoneIndex );
    DeleteVertexBuffer( mesh.m_GLBoneWeights );
    DeleteVertexBuffer( mesh.m_GLIndexBuffer );
}

void MD5Model::SetVertexSkinning( VertexSkinning skinning )
{
    m_VertexSkinning = skinning;
}

VertexSkinning MD5Model::GetVertexSkinning() const
{
    return m_VertexSkinning;
}

void MD5Model::SetWorldTransform( const glm::mat4x4 world )
{
    m_LocalToWorldMatrix = world;
    m_WorldToLoacalMatrix = glm::inverse( m_LocalToWorldMatrix );
}

glm::mat4x4 MD5Model::GetWorldTransform() const
{
    return m_LocalToWorldMatrix;
}

glm::mat4x4 MD5Model::GetInverseWorldTransform() const
{
    return m_WorldToLoacalMatrix;
}

bool MD5Model::LoadModel( const std::string &filename )
{
    if ( !fs::exists(filename) )
    {
        std::cerr << "MD5Model::LoadModel: Failed to find file: " << filename << std::endl;
        return false;
    }

    fs::path filePath = filename;
    // store the parent path used for loading images relative to this file.
    fs::path parent_path = filePath.parent_path();

    std::string param;
    std::string junk;   // Read junk from the file

    fs::ifstream file(filename);
    int fileLength = GetFileLength( file );
    assert( fileLength > 0 );

    m_Joints.clear();
    m_Meshes.clear();

    file >> param;

    while ( !file.eof() )
    {
        if ( param == "MD5Version" )
        {
            file >> m_iMD5Version;
            assert( m_iMD5Version == 10 );
        }
        else if ( param == "commandline" )
        {
            IgnoreLine(file, fileLength ); // Ignore the contents of the line
        }
        else if ( param == "numJoints" )
        {
            file >> m_iNumJoints;
            m_Joints.reserve(m_iNumJoints);
            m_AnimatedBones.assign( m_iNumJoints, glm::mat4x4(1.0f) );
        }
        else if ( param == "numMeshes" )
        {
            file >> m_iNumMeshes;
            m_Meshes.reserve(m_iNumMeshes);
        }
        else if ( param == "joints" )
        {
            Joint joint;
            file >> junk; // Read the '{' character
            for ( int i = 0; i < m_iNumJoints; ++i )
            {
                file >> joint.m_Name >> joint.m_ParentID >> junk
                     >> joint.m_Pos.x >> joint.m_Pos.y >> joint.m_Pos.z >> junk >> junk
                     >> joint.m_Orient.x >> joint.m_Orient.y >> joint.m_Orient.z >> junk;

                RemoveQuotes( joint.m_Name );
                ComputeQuatW( joint.m_Orient );

                m_Joints.push_back(joint);
                // Ignore everything else on the line up to the end-of-line character.
                IgnoreLine( file, fileLength );
            }
            file >> junk; // Read the '}' character

            BuildBindPose( m_Joints );
        }
        else if ( param == "mesh" )
        {
            Mesh mesh;
            int numVerts, numTris, numWeights;

            file >> junk; // Read the '{' character
            file >> param;
            while ( param != "}" )  // Read until we get to the '}' character
            {
                if ( param == "shader" )
                {
                    file >> mesh.m_ShaderName;
                    RemoveQuotes( mesh.m_ShaderName );

                    fs::path shaderPath( mesh.m_ShaderName );
                    fs::path texturePath;
                    if ( shaderPath.has_parent_path() )
                    {
                        texturePath = shaderPath;
                    }
                    else
                    {
                        texturePath = parent_path / shaderPath;
                    }

                    if ( !texturePath.has_extension() )
                    {
                        texturePath.replace_extension( ".tga" );
                    }
                    mesh.m_TexID = SOIL_load_OGL_texture( texturePath.string().c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );
                    mesh.m_Material = m_DefaultMaterial;
                    catchError("chargement texture");

                    file.ignore(fileLength, '\n' ); // Ignore everything else on the line
                }
                else if ( param == "numverts")
                {
                    file >> numVerts;               // Read in the vertices
                    IgnoreLine(file, fileLength);
                    for ( int i = 0; i < numVerts; ++i )
                    {
                        Vertex vert;

                        file >> junk >> junk >> junk                    // vert vertIndex (
                            >> vert.m_Tex0.x >> vert.m_Tex0.y >> junk  //  s t )
                            >> vert.m_StartWeight >> vert.m_WeightCount;

                        IgnoreLine(file, fileLength);

                        mesh.m_Verts.push_back(vert);
                        mesh.m_Tex2DBuffer.push_back(vert.m_Tex0);
                    }
                }
                else if ( param == "numtris" )
                {
                    file >> numTris;
                    IgnoreLine(file, fileLength);
                    for ( int i = 0; i < numTris; ++i )
                    {
                        Triangle tri;
                        file >> junk >> junk >> tri.m_Indices[0] >> tri.m_Indices[1] >> tri.m_Indices[2];

                        IgnoreLine( file, fileLength );

                        mesh.m_Tris.push_back(tri);
                        mesh.m_IndexBuffer.push_back( (GLuint)tri.m_Indices[0] );
                        mesh.m_IndexBuffer.push_back( (GLuint)tri.m_Indices[1] );
                        mesh.m_IndexBuffer.push_back( (GLuint)tri.m_Indices[2] );
                    }
                }
                else if ( param == "numweights" )
                {
                    file >> numWeights;
                    IgnoreLine( file, fileLength );
                    for ( int i = 0; i < numWeights; ++i )
                    {
                        Weight weight;
                        file >> junk >> junk >> weight.m_JointID >> weight.m_Bias >> junk
                            >> weight.m_Pos.x >> weight.m_Pos.y >> weight.m_Pos.z >> junk;

                        IgnoreLine( file, fileLength );
                        mesh.m_Weights.push_back(weight);
                    }
                }
                else
                {
                    IgnoreLine(file, fileLength);
                }

                file >> param;
            }

            PrepareMesh(mesh);
            PrepareNormals(mesh);
            CreateVertexBuffers(mesh);

            m_Meshes.push_back(mesh);
            std::cout << "1 mesh charge, maintenant "<< m_Meshes.size() << " en memoire"<<std::endl;
        }

        file >> param;
    }

    assert( m_Joints.size() == m_iNumJoints );
    assert( m_Meshes.size() == m_iNumMeshes );

    return true;
}

bool MD5Model::LoadAnim( const std::string& filename )
{
    if ( m_Animation.LoadAnimation( filename ) )
    {
        // Check to make sure the animation is appropriate for this model
        m_bHasAnimation = CheckAnimation( m_Animation );
    }
    return m_bHasAnimation;
}

void MD5Model::setShaders(const std::string& filePath )
{
    const std::string vsPath = filePath + ".vs", fsPath = filePath + ".fs";

    m_shader.loadProgram(vsPath.c_str(), fsPath.c_str());
    #define aID_TexCoords 1
    std::vector <char*> locations;
    locations.push_back("texCoords");
    m_shader.bindAttribsLocations(locations);
    m_shader.linkProgram();

    std::vector < char* > uniforms;
    #define i_uID_baseTex 0
    uniforms.push_back("baseTexture");
    #define i_uID_gBoneMatrix 1
    uniforms.push_back("gBoneMatrix[0]");
    glUseProgram(m_shader.getProgramID());
    m_uniformIDs = m_shader.getUniformLocations(uniforms);

    for (int i = 0; i<m_uniformIDs.size(); i++)
        std::cout << uniforms[i] << " IDlol : " << m_uniformIDs[i] << std::endl;
}

bool MD5Model::CheckAnimation( const MD5Animation& animation ) const
{
    if ( m_iNumJoints != animation.GetNumJoints() )
    {
        return false;
    }

    // Check to make sure the joints match up
    for ( unsigned int i = 0; i < m_Joints.size(); ++i )
    {
        const Joint& meshJoint = m_Joints[i];
        const MD5Animation::JointInfo& animJoint = animation.GetJointInfo( i );

        if ( meshJoint.m_Name != animJoint.m_Name ||
             meshJoint.m_ParentID != animJoint.m_ParentID )
        {
            return false;
        }
    }

    return true;
}

void MD5Model::BuildBindPose( const JointList& joints )
{
    m_BindPose.clear();
    m_InverseBindPose.clear();

    JointList::const_iterator iter = joints.begin();
    while ( iter != joints.end() )
    {
        const Joint& joint = (*iter);
        glm::mat4x4 boneTranslation = glm::translate( joint.m_Pos );
        glm::mat4x4 boneRotation = glm::toMat4( joint.m_Orient );

        glm::mat4x4 boneMatrix = boneTranslation * boneRotation;

        glm::mat4x4 inverseBoneMatrix = glm::inverse( boneMatrix );

        m_BindPose.push_back( boneMatrix );
        m_InverseBindPose.push_back( inverseBoneMatrix );

        ++iter;
    }
}

// Compute the position of the vertices in object local space
// in the skeleton's bind pose
bool MD5Model::PrepareMesh( Mesh& mesh )
{
    catchError("debut prepareMesh");
    mesh.m_PositionBuffer.clear();
    mesh.m_Tex2DBuffer.clear();
    mesh.m_BoneIndex.clear();
    mesh.m_BoneWeights.clear();

    // Compute vertex positions
    for ( unsigned int i = 0; i < mesh.m_Verts.size(); ++i )
    {
        glm::vec3 finalPos(0);
        Vertex& vert = mesh.m_Verts[i];

        vert.m_Pos = glm::vec3(0);
        vert.m_Normal = glm::vec3(0);
        vert.m_BoneWeights = glm::vec4(0);
        vert.m_BoneIndices = glm::vec4(0);

        // Sum the position of the weights
        for ( int j = 0; j < vert.m_WeightCount; ++j )
        {
            assert( j < 4 );

            Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
            Joint& joint = m_Joints[weight.m_JointID];

            // Convert the weight position from Joint local space to object space
            glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;

            vert.m_Pos += ( joint.m_Pos + rotPos ) * weight.m_Bias;
            vert.m_BoneIndices[j] = (float)weight.m_JointID;
            vert.m_BoneWeights[j] = weight.m_Bias;
        }

        mesh.m_PositionBuffer.push_back(vert.m_Pos);
        mesh.m_Tex2DBuffer.push_back(vert.m_Tex0);
        mesh.m_BoneIndex.push_back(vert.m_BoneIndices);
        mesh.m_BoneWeights.push_back(vert.m_BoneWeights);
    }
    catchError("Fin prepareMash");
    return true;
}

// Compute the vertex normals in the Mesh's bind pose
bool MD5Model::PrepareNormals( Mesh& mesh )
{
    mesh.m_NormalBuffer.clear();

    // Loop through all triangles and calculate the normal of each triangle
    for ( unsigned int i = 0; i < mesh.m_Tris.size(); ++i )
    {
        glm::vec3 v0 = mesh.m_Verts[ mesh.m_Tris[i].m_Indices[0] ].m_Pos;
        glm::vec3 v1 = mesh.m_Verts[ mesh.m_Tris[i].m_Indices[1] ].m_Pos;
        glm::vec3 v2 = mesh.m_Verts[ mesh.m_Tris[i].m_Indices[2] ].m_Pos;

        glm::vec3 normal = glm::cross( v2 - v0, v1 - v0 );

        mesh.m_Verts[ mesh.m_Tris[i].m_Indices[0] ].m_Normal += normal;
        mesh.m_Verts[ mesh.m_Tris[i].m_Indices[1] ].m_Normal += normal;
        mesh.m_Verts[ mesh.m_Tris[i].m_Indices[2] ].m_Normal += normal;
    }

    // Now normalize all the normals
    for ( unsigned int i = 0; i < mesh.m_Verts.size(); ++i )
    {
        Vertex& vert = mesh.m_Verts[i];

        vert.m_Normal = glm::normalize( vert.m_Normal );
        mesh.m_NormalBuffer.push_back( vert.m_Normal );
    }

    return true;
}

bool MD5Model::PrepareMesh( Mesh& mesh, const std::vector<glm::mat4x4>& skel )
{
    for ( unsigned int i = 0; i < mesh.m_Verts.size(); ++i )
    {
        const Vertex& vert = mesh.m_Verts[i];
        glm::vec3& pos = mesh.m_PositionBuffer[i];
        glm::vec3& normal = mesh.m_NormalBuffer[i];

        pos = glm::vec3(0);
        normal = glm::vec3(0);

        for ( int j = 0; j < vert.m_WeightCount; ++j )
        {
            const Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
            const glm::mat4x4 boneMatrix = skel[weight.m_JointID];

            pos += glm::vec3( ( boneMatrix * glm::vec4( vert.m_Pos, 1.0f ) ) * weight.m_Bias );
            normal += glm::vec3( ( boneMatrix * glm::vec4( vert.m_Normal, 0.0f ) ) * weight.m_Bias );
        }
    }
    return true;
}

bool MD5Model::CreateVertexBuffers( Mesh& mesh )
{
    catchError("debut de CreateVertexBuffers");
    CreateVertexBuffer( mesh.m_GLPositionBuffer );
    CreateVertexBuffer( mesh.m_GLNormalBuffer );
    CreateVertexBuffer( mesh.m_GLTexCoord );
    CreateVertexBuffer( mesh.m_GLBoneWeights );
    CreateVertexBuffer( mesh.m_GLBoneIndex );
    CreateVertexBuffer( mesh.m_GLIndexBuffer );

    // Populate the VBO's
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLPositionBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.m_PositionBuffer.size(), &(mesh.m_PositionBuffer[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer de positions");

    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLNormalBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.m_NormalBuffer.size(), &(mesh.m_NormalBuffer[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer de normales");

    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLTexCoord );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.m_Tex2DBuffer.size(), &(mesh.m_Tex2DBuffer[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer de coords de texture");

    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLBoneWeights );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec4) * mesh.m_BoneWeights.size(), &(mesh.m_BoneWeights[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer de poids d\'os");

    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLBoneIndex );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec4) * mesh.m_BoneIndex.size(), &(mesh.m_BoneIndex[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer d\'index d\'os");

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh.m_GLIndexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.m_IndexBuffer.size(), &(mesh.m_IndexBuffer[0]), GL_STATIC_DRAW );
    catchError("remplissage buffer d\'index");

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    catchError("unbind les buffers");

    return true;
}

void MD5Model::Update( float fDeltaTime )
{
    if ( m_bHasAnimation )
    {
        m_Animation.Update(fDeltaTime);

        const MatrixList& animatedSkeleton = m_Animation.GetSkeletonMatrixList();
        // Multiply the animated skeleton joints by the inverse of the bind pose.
        for ( int i = 0; i < m_iNumJoints; ++i )
        {
            m_AnimatedBones[i] = animatedSkeleton[i] * m_InverseBindPose[i];
        }
    }
    else
    {
        // No animation.. Just use identity matrix for each bone.
        m_AnimatedBones.assign(m_iNumJoints, glm::mat4x4(1.0) );
    }

    for ( unsigned int i = 0; i < m_Meshes.size(); ++i )
    {
        // NOTE: This only needs to be done for CPU skinning, but if I want to render the
        // animated normals, I have to update the mesh on the CPU.
        PrepareMesh( m_Meshes[i], m_AnimatedBones );
    }
}

void MD5Model::Render()
{
    glPushMatrix();
    glMultMatrixf( glm::value_ptr(m_LocalToWorldMatrix) );
    catchError("glMultMat par le LocalWorld");

    // Render the meshes
    for ( unsigned int i = 0; i < m_Meshes.size(); ++i )
    {
        RenderMesh( m_Meshes[i] );
    }

    /*m_Animation.Render();

    for ( unsigned int i = 0; i < m_Meshes.size(); ++i )
    {
        RenderNormals( m_Meshes[i] );
    }*/

    glPopMatrix();
}

void MD5Model::RenderCPU( const Mesh& mesh )
{
    glColor3f( 1.0f, 1.0f, 1.0f );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, mesh.m_TexID );

    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr( mesh.m_Material.Ambient ) );
    glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, glm::value_ptr( mesh.m_Material.Emissive ) );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr( mesh.m_Material.Diffuse) );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr( mesh.m_Material.Specular) );
    glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, mesh.m_Material.SpecularPower );

    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, &(mesh.m_PositionBuffer[0]) );

    glEnableClientState( GL_NORMAL_ARRAY );
    glNormalPointer( GL_FLOAT, 0, &(mesh.m_NormalBuffer[0]) );

    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer( 2, GL_FLOAT, 0, &(mesh.m_Tex2DBuffer[0]) );

    glDrawElements( GL_TRIANGLES, mesh.m_IndexBuffer.size(), GL_UNSIGNED_INT, &(mesh.m_IndexBuffer[0]) );

    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
}

void MD5Model::RenderGPU( const Mesh& mesh )
{
    catchError("RenderGPU");
    //glBindTexture(GL_TEXTURE_2D,  mesh.m_TexID);
    /*EffectManager& mgr = EffectManager::Get();
    mgr.SetWorldMatrix( m_LocalToWorldMatrix );
    mgr.SetMaterial( mesh.m_Material );

    EffectParameter& baseSampler = m_Effect.GetParameterByName("baseSampler");
    baseSampler.Set( mesh.m_TexID );

    EffectParameter& boneMatrix = m_Effect.GetParameterByName("gBoneMatrix");
    boneMatrix.Set( m_AnimatedBones );

    Technique& technique = m_Effect.GetFirstValidTechnique();
    Pass& pass0 = technique.GetPassByName("p0");

    mgr.UpdateSharedParameters();
    m_Effect.UpdateParameters();*/
    glUseProgram(m_shader.getProgramID());
    glUniformMatrix4fv(m_uniformIDs[i_uID_gBoneMatrix], m_AnimatedBones.size(), GL_FALSE, &(m_AnimatedBones[0][0][0]));

    // Position data
    glEnableClientState( GL_VERTEX_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLPositionBuffer );
    glVertexPointer( 3, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // Normal data
    glEnableClientState( GL_NORMAL_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLNormalBuffer );
    glNormalPointer( GL_FLOAT, 0, BUFFER_OFFSET(0) );


    // TEX0
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D,  mesh.m_TexID );
    //glUniform1i(m_unifomIDs[0] , /*GL_TEXTURE*/0);

    // TEXCOORD0 (Base texture coordinates)
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLTexCoord );
    glTexCoordPointer( 2, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // TEXCOORD1 (Blend weights)
    glClientActiveTexture( GL_TEXTURE1 );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLBoneWeights );
    glTexCoordPointer( 4, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // TEXCOORD2 (Bone indices)
    glClientActiveTexture( GL_TEXTURE2 );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, mesh.m_GLBoneIndex );
    glTexCoordPointer( 4, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    catchError("initialisation de l\'affichage");

    // Draw mesh from index buffer
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh.m_GLIndexBuffer );
    glDrawElements( GL_TRIANGLES, mesh.m_IndexBuffer.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0) );

    glClientActiveTexture( GL_TEXTURE2 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glClientActiveTexture( GL_TEXTURE1 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glClientActiveTexture( GL_TEXTURE0 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glActiveTexture( GL_TEXTURE0 );
    glDisable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    catchError("affichage");
    glUseProgram(0);
}

void MD5Model::RenderMesh( const Mesh& mesh )
{
    switch ( m_VertexSkinning )
    {
    case VS_CPU:
        {
            RenderCPU( mesh );
        }
        break;
    case VS_GPU:
        {
            RenderGPU( mesh );
        }
        break;
    }
}

void MD5Model::RenderNormals(  const Mesh& mesh )
{

    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_LIGHTING );

    glColor3f( 1.0f, 1.0f, 0.0f );// Yellow

    glBegin( GL_LINES );
    {
        for ( unsigned int i = 0; i < mesh.m_PositionBuffer.size(); ++i )
        {
            glm::vec3 p0 = mesh.m_PositionBuffer[i];
            glm::vec3 p1 = ( mesh.m_PositionBuffer[i] + mesh.m_NormalBuffer[i] );

            glVertex3fv( glm::value_ptr(p0) );
            glVertex3fv( glm::value_ptr(p1) );
        }
    }
    glEnd();

    glPopAttrib();
}

void MD5Model::RenderSkeleton( const JointList& joints )
{
    glPointSize( 5.0f );
    glColor3f( 1.0f, 0.0f, 0.0f );

    glPushAttrib( GL_ENABLE_BIT );

    glDisable(GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    // Draw the joint positions
    glBegin( GL_POINTS );
    {
        for ( unsigned int i = 0; i < joints.size(); ++i )
        {
            glVertex3fv( glm::value_ptr(joints[i].m_Pos) );
        }
    }
    glEnd();

    // Draw the bones
    glColor3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_LINES );
    {
        for ( unsigned int i = 0; i < joints.size(); ++i )
        {
            const Joint& j0 = joints[i];
            if ( j0.m_ParentID != -1 )
            {
                const Joint& j1 = joints[j0.m_ParentID];
                glVertex3fv( glm::value_ptr(j0.m_Pos) );
                glVertex3fv( glm::value_ptr(j1.m_Pos) );
            }
        }
    }
    glEnd();

    glPopAttrib();

}