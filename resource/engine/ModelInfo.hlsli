static const int NO_BONE = -1;

//モデル用頂点
struct Vertex
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    min16int4 boneNo : BONE_NO;
    float4 weight : WEIGHT;
    uint instanceID : SV_InstanceID;
};

struct Material
{
    //Lambert
    float3 ambient;
    float ambientFactor;
    float3 diffuse;
    float diffuseFactor;
    float3 emissive;
    float emissiveFactor;
    float transparent;
	
	//Phong
    float3 specular;
    float specularFactor;
    float shininess;
    float reflection;
    
    //PBR
    float3 baseColor;
    float metalness;
    float specular_pbr;
    float roughness;
};