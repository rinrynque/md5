#pragma once

#include "MD5Animation.h"
#include "Material.h"
#include "shaders.h"
#include "programCommunication.h"

enum VertexSkinning
{
    VS_CPU,
    VS_GPU
};

class MD5Model
{
public:
    MD5Model();
    virtual ~MD5Model();

    void SetVertexSkinning( VertexSkinning skinning );
    VertexSkinning GetVertexSkinning() const;

    bool LoadModel( const std::string& filename );
    bool LoadAnim( const std::string& filename );
    void setShaders(const std::string& filePath );

    void SetWorldTransform( const glm::mat4x4 world );
    glm::mat4x4 GetWorldTransform() const;
    glm::mat4x4 GetInverseWorldTransform() const;

    void Update( float fDeltaTime );
    void Render();

protected:
    typedef std::vector<glm::vec3> PositionBuffer;
    typedef std::vector<glm::vec3> NormalBuffer;
    typedef std::vector<glm::vec2> Tex2DBuffer;
    typedef std::vector<glm::vec4> WeightBuffer;
    typedef std::vector<glm::vec4> BoneIndexBuffer;

    typedef std::vector<GLuint> IndexBuffer;


    struct Vertex
    {
        glm::vec3   m_Pos;
        glm::vec3   m_Normal;
        glm::vec2   m_Tex0;
        glm::vec4   m_BoneWeights;
        glm::vec4   m_BoneIndices;

        // DEPRICATED
        int         m_StartWeight;
        int         m_WeightCount;
    };
    typedef std::vector<Vertex> VertexList;

    struct Triangle
    {
        int             m_Indices[3];
    };
    typedef std::vector<Triangle> TriangleList;

    struct Weight
    {
        int             m_JointID;
        float           m_Bias;
        glm::vec3       m_Pos;
    };
    typedef std::vector<Weight> WeightList;

    struct Joint
    {
        std::string     m_Name;
        int             m_ParentID;
        glm::vec3       m_Pos;
        glm::quat       m_Orient;
    };
    typedef std::vector<Joint> JointList;

    struct Mesh
    {
        Mesh()
            : m_GLPositionBuffer(0)
            , m_GLNormalBuffer(0)
            , m_GLBoneWeights(0)
            , m_GLBoneIndex(0)
            , m_GLTexCoord(0)
            , m_GLIndexBuffer(0)
        {}

        std::string     m_ShaderName;
        Material        m_Material;
        // This vertex list stores the vertices's in the bind pose.
        VertexList      m_Verts;
        TriangleList    m_Tris;
        WeightList      m_Weights;

        // A texture ID for the material
        GLuint          m_TexID;

        // These buffers are used for rendering the animated mesh
        PositionBuffer  m_PositionBuffer;   // Vertex position stream
        NormalBuffer    m_NormalBuffer;     // Vertex normals stream
        WeightBuffer    m_BoneWeights;      // Bone weights buffer
        BoneIndexBuffer m_BoneIndex;       // Bone index buffer

        Tex2DBuffer     m_Tex2DBuffer;      // Texture coordinate set
        IndexBuffer     m_IndexBuffer;      // Vertex index buffer

        // Vertex buffer Object IDs for the vertex streams
        GLuint          m_GLPositionBuffer;
        GLuint          m_GLNormalBuffer;
        GLuint          m_GLBoneWeights;
        GLuint          m_GLBoneIndex;
        GLuint          m_GLTexCoord;
        GLuint          m_GLIndexBuffer;
    };
    typedef std::vector<Mesh> MeshList;

    // Build the bind-pose and the inverse bind-pose matrix array for the model.
    void BuildBindPose( const JointList& joints );

    // Prepare the mesh for rendering
    // Compute vertex positions and normals
    bool PrepareMesh( Mesh& mesh );
    bool PrepareMesh( Mesh& mesh, const std::vector<glm::mat4x4>& skel );
    bool PrepareNormals( Mesh& mesh );

    void DestroyMesh( Mesh& mesh );

    // Create the VBOs that are used to render the mesh with shaders
    bool CreateVertexBuffers( Mesh& mesh );

    void RenderCPU( const Mesh& mesh );
    void RenderGPU( const Mesh& mesh );

    // Render a single mesh of the model
    void RenderMesh( const Mesh& mesh );
    void RenderNormals( const Mesh& mesh );

    // Draw the skeleton of the mesh for debugging purposes.
    void RenderSkeleton( const JointList& joints );

    bool CheckAnimation( const MD5Animation& animation ) const;
private:
    typedef std::vector<glm::mat4x4> MatrixList;

    int                 m_iMD5Version;
    int                 m_iNumJoints;
    int                 m_iNumMeshes;

    bool                m_bHasAnimation;

    JointList           m_Joints;
    MeshList            m_Meshes;

    MD5Animation        m_Animation;

    MatrixList          m_BindPose;
    MatrixList          m_InverseBindPose;

    // Animated bone matrix from the animation with the inverse bind pose applied.
    MatrixList          m_AnimatedBones;

    glm::mat4x4         m_LocalToWorldMatrix;
    glm::mat4x4         m_WorldToLoacalMatrix;

    // The Cg shader effect that is used to render this model.
//    Effect&             m_Effect;
    // Define a default material to meshes.
    Material            m_DefaultMaterial;

    //Shader used to render the model
    opShader        m_shader;
    std::vector<GLuint> m_uniformIDs;

    // Perform vertex skinning on the CPU or the GPU
    VertexSkinning      m_VertexSkinning;
};
