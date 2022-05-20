#include"ModelInfo.hlsli"
#include"Camera.hlsli"

cbuffer cbuff0 : register(b0)
{
    Camera cam;
}

cbuffer cbuff1 : register(b1)
{
    matrix world;
}

cbuffer cbuff2 : register(b2)
{
    Camera lightCam;
}

Texture2D<float4> tex : register(t0);
Texture2D<float4> shadowMap : register(t1);
SamplerState smp : register(s0);
SamplerComparisonState shadowMapSmp : register(s1);

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 posInLVP : TEXCOORD1;    //���C�g�r���[�X�N���[����Ԃł̃s�N�Z���̍��W
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
    float4 worldPos = mul(world, resultPos);
    
    //�ʏ�̍��W�ϊ�
    output.svpos = mul(cam.proj, mul(cam.view, worldPos));
    output.uv = input.uv;
    
    //���C�g�r���[�X�N���[����Ԃ̍��W
    output.posInLVP = mul(lightCam.proj, mul(lightCam.view, worldPos));
    
    return output;
}


float4 PSmain(VSOutput input) : SV_TARGET
{
    //���C�g�r���[�X�N���[����Ԃ���UV��Ԃɍ��W�ϊ�
    float2 shadowMapUV = input.posInLVP.xy / input.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;
    
    //���C�g�r���[�X�N���[����Ԃł�Z�l���v�Z����
    float zInLVP = input.posInLVP.z / input.posInLVP.w;

    float4 color = tex.Sample(smp, input.uv);
    
    if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        // �v�Z����UV���W���g���āA�V���h�E�}�b�v����[�x�l���T���v�����O
        float zInShadowMap = shadowMap.Sample(smp, shadowMapUV).r;
        if (zInLVP > zInShadowMap)
        {
          //�Օ����̌v�Z�i��r����Z�l���傫�����1.0,���������0.0�B������S�e�N�Z�����s�����ς�Ԃ��j
            float shadow = shadowMap.SampleCmpLevelZero(
            shadowMapSmp, //�g�p����T���v���[
            shadowMapUV, //�V���h�E�}�b�vUV
            zInLVP); //��r����Z�l
        
          //�V���h�E�J���[���v�Z
            float3 shadowColor = color.xyz * 0.5f;
        
        //�Օ������g���Đ��`���
            color.xyz = lerp(color.xyz, shadowColor, shadow);
        }
    }
    
    return color;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}