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

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 posInLVP : TEXCOORD1;    //ライトビュースクリーン空間でのピクセルの座標
};

VSOutput VSmain(Vertex input)
{
    float4 resultPos = input.pos;
	
	////ボーン行列
	////BDEF4(ボーン4つ又は3つと、それぞれのウェイト値。ウェイト合計が1.0である保障はしない)
 //   if (input.boneNo[2] != NO_BONE)
 //   {
 //       int num;
        
 //       if (input.boneNo[3] != NO_BONE)    //ボーン４つ
 //       {
 //           num = 4;
 //       }
 //       else //ボーン３つ
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
	////BDEF2(ボーン2つと、ボーン1のウェイト値(PMD方式))
 //   else if (input.boneNo[1] != NONE)
 //   {
 //       matrix mat = bones[input.boneNo[0]] * input.weight[0];
 //       mat += bones[input.boneNo[1]] * (1 - input.weight[0]);
        
 //       resultPos = mul(mat, input.pos);
 //   }
	////BDEF1(ボーンのみ)
 //   else if (input.boneNo[0] != NONE)
 //   {
 //       resultPos = mul(bones[input.boneNo[0]], input.pos);
 //   }
	
    VSOutput output;
    float4 worldPos = mul(world, resultPos);
    
    //通常の座標変換
    output.svpos = mul(cam.proj, mul(cam.view, worldPos));
    output.uv = input.uv;
    
    //ライトビュースクリーン空間の座標
    output.posInLVP = mul(lightCam.proj, mul(lightCam.view, worldPos));
    
    return output;
}


float4 PSmain(VSOutput input) : SV_TARGET
{
    //ライトビュースクリーン空間からUV空間に座標変換
    float2 shadowMapUV = input.posInLVP.xy / input.posInLVP.w;
    shadowMapUV *= float2(0.5f, -0.5f);
    shadowMapUV += 0.5f;

    //UV座標を使ってシャドウマップから影情報をサンプリング
    float3 shadowMapColor = 1.0f;
    if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
        && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
    {
        shadowMapColor = shadowMap.Sample(smp, shadowMapUV);
    }
    
    float4 color = tex.Sample(smp, input.uv);
    color.xyz *= shadowMapColor;
    return color;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}