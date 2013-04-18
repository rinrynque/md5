#pragma once

struct Material
{
    Material( const glm::float4& ambient = glm::float4( 0.1f, 0.1f, 0.1f, 1.0f ),
              const glm::float4& emissive = glm::float4( 0.0f, 0.0f, 0.0f, 1.0f ),
              const glm::float4& diffuse = glm::float4( 1.0f, 1.0f, 1.0f, 1.0f ),
              const glm::float4& specular = glm::float4( 1.0f, 1.0f, 1.0f, 1.0f ),
              const float        specularPower = 1.0f )
    : Ambient( ambient )
    , Emissive( emissive )
    , Diffuse( diffuse )
    , Specular( specular )
    , SpecularPower( specularPower )
    {}

    glm::float4 Ambient;
    glm::float4 Emissive;
    glm::float4 Diffuse;
    glm::float4 Specular;
    float       SpecularPower;
};
