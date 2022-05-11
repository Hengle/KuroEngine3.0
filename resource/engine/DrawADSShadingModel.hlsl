#include"ModelInfo.hlsli"
#include"Camera.hlsli"
#include"LightInfo.hlsli"

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    LightInfo ligNum; //�A�N�e�B�u���̃��C�g�̐��̏��
}

StructuredBuffer<DirectionLight> dirLight : register(t0);
StructuredBuffer<PointLight> pointLight : register(t1);
StructuredBuffer<SpotLight> spotLight : register(t2);
StructuredBuffer<HemiSphereLight> hemiSphereLight : register(t3);


cbuffer cbuff2 : register(b2)
{
    matrix world;
}

Texture2D<float4> tex : register(t4);
SamplerState smp : register(s0);

cbuffer cbuff3 : register(b3)
{
    Material material;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 worldpos : POSITION;
    float3 wnormal : NORMAL0; // �@��
    float3 vnormal : NORMAL1; //�r���[�ϊ���̖@���x�N�g��
    float2 uv : TEXCOORD;
    float depthInView : CAM_Z; //�J������Ԃł�Z
};

VSOutput VSmain(Vertex input)
{
    float4 resultPos = input.pos;
	
	////�{�[���s��
	////BDEF4(�{�[��4����3�ƁA���ꂼ��̃E�F�C�g�l�B�E�F�C�g���v��1.0�ł���ۏ�͂��Ȃ�)
 //   if (input.boneNo[2] != NO_BONE)
 //   {
 //       int num;
        
 //       if (input.boneNo[3] != NO_BONE)    //�{�[���S��
 //       {
 //           num = 4;
 //       }
 //       else //�{�[���R��
 //       {
 //           num = 3;
 //       }
        
 //       matrix mat = bones[input.boneNo[0]] * input.weight[0];
 //       for (int i = 1; i < num; ++i)
 //       {
 //           mat += bones[input.boneNo[i]] * input.weight[i];
 //       }
 //       resultPos = mul(mat, input.pos);
 //   }
	////BDEF2(�{�[��2�ƁA�{�[��1�̃E�F�C�g�l(PMD����))
 //   else if (input.boneNo[1] != NONE)
 //   {
 //       matrix mat = bones[input.boneNo[0]] * input.weight[0];
 //       mat += bones[input.boneNo[1]] * (1 - input.weight[0]);
        
 //       resultPos = mul(mat, input.pos);
 //   }
	////BDEF1(�{�[���̂�)
 //   else if (input.boneNo[0] != NONE)
 //   {
 //       resultPos = mul(bones[input.boneNo[0]], input.pos);
 //   }
	
    VSOutput output;
    float4 wpos = mul(world, resultPos); //���[���h�ϊ�
    output.svpos = mul(cam.view, wpos); //�r���[�ϊ�
    output.depthInView = output.svpos.z; //�J�������猩��Z
    output.svpos = mul(cam.proj, output.svpos); //�v���W�F�N�V�����ϊ�
    output.worldpos = wpos;
    
    // �@���Ƀ��[���h�s��ɂ��X�P�[�����O�E��]��K�p
    float4 wnormal = mul(world, float4(input.normal, 0));
    output.wnormal = normalize(wnormal.xyz);
    output.vnormal = normalize(mul(cam.view, wnormal).xyz);
    output.uv = input.uv;
    return output;
}

struct PSOutput
{
    float4 color : SV_Target0;
    //float4 emissive : SV_Target1;
};


PSOutput PSmain(VSOutput input) : SV_TARGET
{
     //���C�g�̉e��
    float3 ligEffect = { 0.0f, 0.0f, 0.0f };
    
    //�f�B���N�V�������C�g
    for (int i = 0; i < ligNum.dirLigNum; ++i)
    {
        if (!dirLight[i].active)continue;
        
        float3 dir = dirLight[i].direction;
        float3 ligCol = dirLight[i].color.xyz * dirLight[i].color.w;
        ligEffect += CalcLambertDiffuse(dir, ligCol, input.wnormal) * (material.diffuse * material.diffuseFactor);
        ligEffect += CalcPhongSpecular(dir, ligCol, input.wnormal, input.worldpos, cam.eyePos) * (material.specular * material.specularFactor);
        ligEffect += CalcLimLight(dir, ligCol, input.wnormal, input.vnormal);
    }
    //�|�C���g���C�g
    for (int i = 0; i < ligNum.ptLigNum; ++i)
    {
        if (!pointLight[i].active)continue;
        
        float3 dir = input.worldpos - pointLight[i].pos;
        dir = normalize(dir);
        float3 ligCol = pointLight[i].color.xyz * pointLight[i].color.w;
        
        //�����Ȃ����
        float3 diffPoint = CalcLambertDiffuse(dir, ligCol, input.wnormal);
        float3 specPoint = CalcPhongSpecular(dir, ligCol, input.wnormal, input.worldpos, cam.eyePos);
        
        //�����ɂ�錸��
        float3 distance = length(input.worldpos - pointLight[i].pos);
		//�e�����͋����ɔ�Ⴕ�ď������Ȃ��Ă���
        float affect = 1.0f - 1.0f / pointLight[i].influenceRange * distance;
		//�e���͂��}�C�i�X�ɂȂ�Ȃ��悤�ɕ␳��������
        if (affect < 0.0f)
            affect = 0.0f;
		//�e�����w���֐��I�ɂ���
        affect = pow(affect, 3.0f);
        diffPoint *= affect;
        specPoint *= affect;
        
        ligEffect += diffPoint * (material.diffuse * material.diffuseFactor);
        ligEffect += specPoint * (material.specular * material.specularFactor);
        ligEffect += CalcLimLight(dir, ligCol, input.wnormal, input.vnormal);
    }
    //�X�|�b�g���C�g
    for (int i = 0; i < ligNum.spotLigNum; ++i)
    {
        if (!spotLight[i].active)continue;
        
        float3 ligDir = input.worldpos - spotLight[i].pos;
        ligDir = normalize(ligDir);
        float3 ligCol = spotLight[i].color.xyz * spotLight[i].color.w;
        
        //�����Ȃ����
        float3 diffSpotLight = CalcLambertDiffuse(ligDir, ligCol, input.wnormal);
        float3 specSpotLight = CalcPhongSpecular(ligDir, ligCol, input.wnormal, input.worldpos, cam.eyePos);
        
        //�X�|�b�g���C�g�Ƃ̋������v�Z
        float3 distance = length(input.worldpos - spotLight[i].pos);
       	//�e�����͋����ɔ�Ⴕ�ď������Ȃ��Ă���
        float affect = 1.0f - 1.0f / spotLight[i].influenceRange * distance;
        //�e���͂��}�C�i�X�ɂȂ�Ȃ��悤�ɕ␳��������
        if (affect < 0.0f)
            affect = 0.0f;
    //�e�����w���֐��I�ɂ���
        affect = pow(affect, 3.0f);
        diffSpotLight *= affect;
        specSpotLight *= affect;
    
        float3 spotlim = CalcLimLight(ligDir, ligCol, input.wnormal, input.vnormal) * affect;
        
        float3 dir = normalize(spotLight[i].target - spotLight[i].pos);
        float angle = dot(ligDir, dir);
        angle = abs(acos(angle));
        affect = 1.0f - 1.0f / spotLight[i].angle * angle;
        if (affect < 0.0f)
            affect = 0.0f;
        affect = pow(affect, 0.5f);
        
        ligEffect += diffSpotLight * affect * (material.diffuse * material.diffuseFactor);
        ligEffect += specSpotLight * affect * (material.specular * material.specularFactor);
        ligEffect += spotlim * affect;
    }
    //�V��
    for (int i = 0; i < ligNum.hemiSphereNum; ++i)
    {
        if (!hemiSphereLight[i].active)continue;
        
        float t = dot(input.wnormal.xyz, hemiSphereLight[i].groundNormal);
        t = (t + 1.0f) / 2.0f;
        float3 hemiLight = lerp(hemiSphereLight[i].groundColor, hemiSphereLight[i].skyColor, t);
        ligEffect += hemiLight;
    }
    
    float4 result = tex.Sample(smp, input.uv);
    result.xyz = ((material.ambient * material.ambientFactor) + ligEffect) * result.xyz;
    result.w *= (1.0f - material.transparent);
    
    PSOutput output;
    output.color = result;
    
    //output.emissive = emissiveMap.Sample(smp, input.uv);
    
    ////���邳�v�Z
    //float bright = dot(result.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    //if (1.0f < bright)
    //    output.emissive += result;
    //output.emissive.w = result.w;
    
    return output;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}